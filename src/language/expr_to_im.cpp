#include "src/language/expr.hpp"
#include "src/language/stmt.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/utils.hpp"

using namespace std;


namespace language {

namespace {

  /* Allows us to proceed through compile errors so that we don't have to report
   * one at a time */
  Expr::Info dummyInfo() { return { make_unique<im::Temp>(newTemp()), Type::ANY_TYPE }; }

}  // namespace


/************
 * ConstInt *
 ************/

Expr::Info ConstInt::toImExpr(Ctx&) {
  // Constant integers are the smallest type possible so that we allow implicit
  // conversion to larger types if required
  return { make_unique<im::Const>(n_), Type::smallestIntegral(n_) };
}

/*************
 * ConstChar *
 *************/
Expr::Info ConstChar::toImExpr(Ctx&) { return { make_unique<im::Const>(c_), Type::CHAR_TYPE }; }

/*************
 * ConstBool *
 *************/
Expr::Info ConstBool::toImExpr(Ctx&) { return { make_unique<im::Const>(b_), Type::BOOL_TYPE }; }

/**********
 * StrLit *
 **********/

Expr::Info StrLit::toImExpr(Ctx& ctx) {
  vector<ExprPtr> chars;
  chars.reserve(str_.size());
  for (char c : str_) {
    chars.push_back(make_unique<ConstChar>(c, line_));
  }

  vector<ExprPtr> params;
  params.push_back(make_unique<NewArray>(TypePtr(Type::CHAR_TYPE), move(chars), line_));
  params.push_back(make_unique<ConstBool>(true, line_));
  return Call(vector<string>(), "String", move(params), line_).toImExpr(ctx);
}


/*******
 * Var *
 *******/

Expr::Info Var::toImExpr(Ctx& ctx) {
  const Ctx::VarInfo* varInfo = ctx.lookupVar(name_, line_);
  if (!varInfo) {
    // Undefined variable
    return dummyInfo();
  }
  return { make_unique<im::Temp>(varInfo->temp), varInfo->type };
}

/***********
 * UnaryOp *
 ***********/

Expr::Info UnaryOp::toImExpr(Ctx& ctx) {
  switch (uOp_) {
    case UOp::NOT:
      // !b = b ^ 1
      return {
        make_unique<im::BinOp>(
            e_->toImExprAssert(*Type::BOOL_TYPE, ctx), make_unique<im::Const>(1), im::BOp::XOR),
        Type::BOOL_TYPE
      };
    case UOp::NEG: {
      Info eInfo = e_->toImExprAssert(
          mem_fun_ref(&Type::isIntegral), "Unary minus requires an integer.", ctx);
      return { make_unique<im::BinOp>(make_unique<im::Const>(-1), move(eInfo.imExpr), im::BOp::MUL),
               move(eInfo.type) };
    }
    default:
      throw invalid_argument("Unrecognized Uop");
  }
}


/************
 * BinaryOp *
 ************/

Expr::Info BinaryOp::toImExpr(Ctx& ctx) {
  switch (bOp_) {
    case BOp::PLUS:
      return toImExprArith(im::BOp::PLUS, ctx);
    case BOp::MINUS:
      return toImExprArith(im::BOp::MINUS, ctx);
    case BOp::MUL:
      return toImExprArith(im::BOp::MUL, ctx);
    case BOp::DIV:
      return toImExprArith(im::BOp::DIV, ctx);
    case BOp::MOD:
      return toImExprArith(im::BOp::MOD, ctx);
    case BOp::LSHIFT:
      return toImExprArith(im::BOp::LSHIFT, ctx);
    case BOp::ARSHIFT:
      return toImExprArith(im::BOp::ARSHIFT, ctx);
    case BOp::BIT_AND:
      return toImExprArith(im::BOp::AND, ctx);
    case BOp::BIT_OR:
      return toImExprArith(im::BOp::OR, ctx);
    case BOp::XOR: {
      // XOR is valid for both integers and booleans
      Info lhsInfo = e1_->toImExpr(ctx);
      e1_ = make_unique<ImWrapper>(move(lhsInfo.imExpr), lhsInfo.type, line_);
      if (lhsInfo.type->isIntegral()) {
        return toImExprArith(im::BOp::XOR, ctx);
      }
    }
      // Fall thru
    default:
      // boolean operator
      return TernaryOp(
                 make_unique<BinaryOp>(move(*this)),
                 make_unique<ConstBool>(true, line_),
                 make_unique<ConstBool>(false, line_))
          .toImExpr(ctx);
  }
}

namespace {
  bool checkIntegral(const Type& type, im::BOp op, size_t line, Ctx& ctx) {
    if (!type.isIntegral()) {
      ostringstream err;
      err << "Operator '" << op << "' expected integral types, got '" << type << '\'';
      ctx.getLogger().logError(line, err.str());
      return false;
    }
    return true;
  }
}  // namespace


Expr::Info BinaryOp::toImExprArith(im::BOp op, Ctx& ctx) {
  Info eInfo1 = e1_->toImExpr(ctx);
  Info eInfo2 = e2_->toImExpr(ctx);
  TypePtr& type1 = eInfo1.type;
  TypePtr& type2 = eInfo2.type;
  // We are not using toImExprAssert(isIntegral) b/c need to access the types to
  // figure out which type the result should be
  bool isOk = checkIntegral(*type1, op, e1_->line_, ctx);
  isOk &= checkIntegral(*type2, op, e2_->line_, ctx);
  if (!isOk) {
    return dummyInfo();
  }
  return { make_unique<im::BinOp>(move(eInfo1.imExpr), move(eInfo2.imExpr), op),
           type1->numBytes > type2->numBytes ? move(type1) : move(type2) };
}

/*************
 * TernaryOp *
 *************/

Expr::Info TernaryOp::toImExpr(Ctx& ctx) {
  size_t e1Line = e1_->line_;
  size_t e2Line = e2_->line_;
  Info e1Info = e1_->toImExpr(ctx);
  Info e2Info = e2_->toImExpr(ctx);

  bool e1ToE2 = e1Info.type->isConvertibleTo(*e2Info.type, nullptr, ctx);
  if (!(e1ToE2 || e2Info.type->isConvertibleTo(*e1Info.type, nullptr, ctx))) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Neither type '" << *e1Info.type << "' nor type '" << *e2Info.type
        << "' is convertible to the other in the ternary expression";
    return dummyInfo();
  }

  // Wider type for classes is the base class
  const TypePtr* widerType;
  if (e1Info.type->typeName == Type::Category::CLASS) {
    widerType = e1ToE2 ? &e2Info.type : &e1Info.type;
  } else {
    widerType = e1Info.type->numBytes > e2Info.type->numBytes ? &e1Info.type : &e2Info.type;
  }

  int t = newTemp();
  ExprPtr tempWrapper1 = make_unique<ImWrapper>(make_unique<im::Temp>(t), e1Info.type, e1Line);
  ExprPtr tempWrapper2 = make_unique<ImWrapper>(make_unique<im::Temp>(t), e2Info.type, e2Line);
  ExprPtr e1Wrapper = make_unique<ImWrapper>(move(e1Info.imExpr), e1Info.type, e1Line);
  ExprPtr e2Wrapper = make_unique<ImWrapper>(move(e2Info.imExpr), e2Info.type, e2Line);

  unique_ptr<Block> ifBlock = make_unique<Block>(vector<StmtPtr>{}, e1Line);
  ifBlock->stmts_.push_back(make_unique<Assign>(move(tempWrapper1), move(e1Wrapper)));

  unique_ptr<Block> elseBlock = make_unique<Block>(vector<StmtPtr>{}, e2Line);
  elseBlock->stmts_.push_back(make_unique<Assign>(move(tempWrapper2), move(e2Wrapper)));

  vector<im::StmtPtr> imStmts;
  If(move(boolE_), move(ifBlock), move(elseBlock), line_).toImStmts(imStmts, ctx);

  return { make_unique<im::DoThenEval>(move(imStmts), make_unique<im::Temp>(t)), move(*widerType) };
}


/********
 * Call *
 ********/

namespace {

  void checkAccess(
      string_view methodName,
      const Ctx::FnInfo& fnInfo,
      const Class& declTy,
      size_t line,
      Ctx& ctx) {
    const ClassDecl* enclosingClass = ctx.insideClass();
    bool isPriv = Func::isPrivate(fnInfo.modifiers);

    // Private methods accessible only within the same class
    // Protected methods accessible only within the subclasses
    if ((isPriv && !(enclosingClass && enclosingClass->id_ == declTy.id))
        || (Func::isProtected(fnInfo.modifiers)
            && !(enclosingClass && ctx.isSubClassOf(enclosingClass->id_, declTy.id)))) {
      ostream& err = ctx.getLogger().logError(line);
      err << "Method '" << declTy.className << "::" << methodName;
      Type::streamParamTypes(fnInfo.paramTypes, err);
      err << "' is not accessible (declared " << (isPriv ? "'private'" : "'protected'") << " in "
          << fnInfo.declFile << " on line " << fnInfo.line << ')';
    }
  }

}  // namespace

Expr::Info Call::toImExpr(Ctx& ctx) {
  vector<im::ExprPtr> paramImExprs;
  vector<TypePtr> paramTypes;
  size_t numParams = params_.size();
  // Get types and convert params
  for (size_t i = 0; i < numParams; ++i) {
    Info exprInfo = params_[i]->toImExpr(ctx);
    paramImExprs.push_back(move(exprInfo.imExpr));
    paramTypes.push_back(move(exprInfo.type));
  }

  const Ctx::FnInfo* fnInfo = ctx.lookupFnRec(qualifiers_, name_, paramTypes, line_);
  if (!fnInfo) {
    // Undefined function
    return dummyInfo();
  }

  if (Func::isCtor(fnInfo->modifiers)) {
    const Class* classTy = static_cast<const Class*>(fnInfo->returnType.get());
    int classId = classTy->id;
    const Ctx::ClassInfo* classInfo = ctx.lookupClass(classId);
    if (!classInfo) {
      throw runtime_error("CallExpr::toImExpr");
    }

    checkAccess(name_, *fnInfo, *classTy, line_, ctx);

    // Allocate the object
    if (ctorAlloc_) {
      vector<im::ExprPtr> mallocBytes;
      mallocBytes.push_back(make_unique<im::Const>(classInfo->numBytes));
      paramImExprs.push_back(
          make_unique<im::Call>(make_unique<im::LabelAddr>("__malloc"), move(mallocBytes), true));
    } else {
      paramImExprs.push_back(Var(lang_utils::THIS, line_).toImExpr(ctx).imExpr);
    }
    paramTypes.push_back(fnInfo->returnType);
  }

  return { make_unique<im::Call>(
               make_unique<im::LabelAddr>(Ctx::mangleFn(name_, fnInfo->id)),
               move(paramImExprs),
               *fnInfo->returnType != *Type::VOID_TYPE),
           move(fnInfo->returnType) };
}


/********
 * Cast *
 ********/

Expr::Info Cast::toImExpr(Ctx& ctx) {
  ctx.checkType(*toType_, line_);
  Info eInfo = expr_->toImExpr(ctx);

  if (!eInfo.type->isConvertibleTo(*toType_, nullptr, ctx)) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "No valid cast from type '" << *eInfo.type << "' to '" << *toType_ << '\'';
  }

  // Integer casts need to be truncated or expanded to the correct number of bytes; im::IntCast
  // handles this
  return eInfo.type->typeName == Type::Category::CLASS
             ? Expr::Info{ move(eInfo.imExpr), move(toType_) }
             : Expr::Info{ make_unique<im::IntCast>(move(eInfo.imExpr), toType_->numBytes),
                           move(toType_) };
}


/************
 * NewArray *
 ************/

Expr::Info NewArray::toImExpr(Ctx& ctx) {
  if (numElems_) {
    return toImExprLen(ctx);
  }
  return toImExprElems(ctx);
}


Expr::Info NewArray::toImExprLen(Ctx& ctx) {
  auto p = makeArrayStmts(*type_, move(numElems_), ctx);

  return { make_unique<im::DoThenEval>(move(p.first), make_unique<im::Temp>(p.second)),
           make_unique<Array>(type_) };
}


Expr::Info NewArray::toImExprElems(Ctx& ctx) {
  size_t len = elems_.size();
  long maxLong = Type::LONG_TYPE->minMaxValue().second;
  if (len > (size_t)maxLong) {
    ostringstream& err = ctx.getLogger().logError(line_);
    err << len << " is greater than the maximum array size " << maxLong;
  }

  auto p = makeArrayStmts(*type_, make_unique<ConstInt>(len, line_), ctx);
  vector<im::StmtPtr>& stmts = p.first;
  int tArrAddr = p.second;
  u_char elemSize = type_->numBytes;

  for (size_t i = 0; i < len; ++i) {
    const ExprPtr& elem = elems_[i];
    // Assign the element
    stmts.push_back(make_unique<im::Assign>(
        make_unique<im::MemDeref>(
            8, make_unique<im::Temp>(tArrAddr), make_unique<im::Const>(i), elemSize),
        elem->toImExprAssert(*type_, ctx)));
  }

  return { make_unique<im::DoThenEval>(move(stmts), make_unique<im::Temp>(tArrAddr)),
           make_unique<Array>(type_) };
}

pair<vector<im::StmtPtr>, int> NewArray::makeArrayStmts(Type& type, ExprPtr&& numElems, Ctx& ctx) {
  ctx.checkType(type, line_);
  int tLen = newTemp();

  // Store the length of the array in a temporary
  im::StmtPtr storeLen = make_unique<im::Assign>(
      make_unique<im::Temp>(tLen),
      numElems
          ->toImExprAssert(
              mem_fun_ref(&Type::isIntegral), "Array size requires an integral type", ctx)
          .imExpr);

  // Compute the size of the array in bytes
  im::ExprPtr mul = make_unique<im::BinOp>(
      make_unique<im::Temp>(tLen), make_unique<im::Const>(type.numBytes), im::BOp::MUL);
  im::ExprPtr arrBytes =
      make_unique<im::BinOp>(move(mul), make_unique<im::Const>(8), im::BOp::PLUS);

  // Allocate the correct number of bytes
  int tArrAddr = newTemp();
  vector<im::ExprPtr> mallocBytes;
  mallocBytes.emplace_back(move(arrBytes));
  im::StmtPtr callMalloc = make_unique<im::Assign>(
      make_unique<im::Temp>(tArrAddr),
      make_unique<im::Call>(make_unique<im::LabelAddr>("__malloc"), move(mallocBytes), true));

  // Arrays will start with the number of elements they contain
  im::StmtPtr setSize = make_unique<im::Assign>(
      make_unique<im::MemDeref>(0, make_unique<im::Temp>(tArrAddr), nullptr, 8),
      make_unique<im::Temp>(tLen));

  // TODO: Throw if less than 0

  vector<im::StmtPtr> stmts;
  stmts.push_back(move(storeLen));
  stmts.push_back(move(callMalloc));
  stmts.push_back(move(setSize));
  return { move(stmts), tArrAddr };
}


/***************
 * ArrayAccess *
 ***************/


// TODO: Throw if out of range

Expr::Info ArrayAccess::toImExpr(Ctx& ctx) {
  Info exprInfo = arrExpr_->toImExpr(ctx);
  const Type& type = *exprInfo.type;
  if (type.typeName != Type::Category::ARRAY) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Operator '[]' can only be used on an arrays, not type '" << type << '\'';
    return dummyInfo();
  }

  const Type& arrType = *static_cast<const Array&>(type).arrType;

  im::ExprPtr imIndex =
      index_
          ->toImExprAssert(
              mem_fun_ref(&Type::isIntegral), "Operator '[]' requires an integral type", ctx)
          .imExpr;

  // Add 8 bytes to skip the size field
  return { make_unique<im::MemDeref>(8, move(exprInfo.imExpr), move(imIndex), arrType.numBytes),
           static_cast<const Array*>(&type)->arrType };
}

/****************
 * MemberAccess *
 ****************/


Expr::Info MemberAccess::toImExpr(Ctx& ctx) {
  Info eInfo = objExpr_->toImExpr(ctx);

  // The only member of an array is length
  if (eInfo.type->typeName == Type::Category::ARRAY) {
    if (member_ == "length") {
      return { make_unique<im::MemDeref>(0, move(eInfo.imExpr), nullptr, 8), Type::LONG_TYPE_FIN };
    }
    ostream& err = ctx.getLogger().logError(line_);
    err << "Type '" << *eInfo.type << "' has no member '" << member_ << '\'';
    return dummyInfo();
  } else if (eInfo.type->typeName == Type::Category::CLASS) {
    const Class* classTy = static_cast<const Class*>(eInfo.type.get());
    const Ctx::ClassInfo* classInfo = ctx.lookupClass(classTy->id);
    if (!classInfo) {
      // If we hit this case, we already logged an undefined class error somewhere else, so don't
      // duplicate it
      return dummyInfo();
    }

    auto iter = classInfo->fields.find(member_);
    // Undefined field
    if (iter == classInfo->fields.end()) {
      ostream& err = ctx.getLogger().logError(line_);
      err << "Field '" << member_ << "' is not defined in class '" << classTy->className << '\'';
      return dummyInfo();
    }

    const Ctx::FieldInfo& fieldInfo = iter->second;

    // Private fields accessible only within the same class
    // Protected fields accessible only within the subclasses
    const ClassDecl* enclosingClass = ctx.insideClass();
    bool isPriv = Func::isPrivate(fieldInfo.accessMod);
    if ((isPriv && !(enclosingClass && enclosingClass->id_ == fieldInfo.declClassId))
        || (Func::isProtected(fieldInfo.accessMod)
            && !(enclosingClass && ctx.isSubClassOf(enclosingClass->id_, fieldInfo.declClassId)))) {
      ostream& err = ctx.getLogger().logError(line_);
      // TODO: Have Ctx::lookupClass return an iter so that we can access the class name in which
      // the field was declared (here and for methods too)
      err << "Field '" << classTy->className << "::" << member_ << "' is not accessible (declared "
          << (isPriv ? "'private'" : "'protected'") << " in " << fieldInfo.declFile << " on line "
          << fieldInfo.line << ')';
    }

    // Check constness
    TypePtr fieldTy;
    if (eInfo.type->isConst) {
      fieldTy = Type::makeConst(fieldInfo.type);
      fieldTy->isFinal = true;
    } else {
      fieldTy = fieldInfo.type;
    }

    return { make_unique<im::MemDeref>(
                 fieldInfo.offset, move(eInfo.imExpr), nullptr, fieldInfo.type->numBytes),
             move(fieldTy) };
  } else {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Type '" << *eInfo.type << "' has no fields";
    return dummyInfo();
  }
}


/********************
 * MethodInvocation *
 ********************/

Expr::Info MethodInvocation::toImExpr(Ctx& ctx) {
  // The method below returns true as the second element if it actually returned the
  // QualifiedInvocation
  pair<Info, bool> p = resolveObjExpr(ctx);
  if (p.second) {
    return move(p.first);
  }

  Info& eInfo = p.first;
  if (eInfo.type->typeName != Type::Category::CLASS) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Cannot invoke method on expression of type '" << *eInfo.type << '\'';
    return dummyInfo();
  }

  // Get types and convert params
  vector<im::ExprPtr> paramImExprs;
  vector<TypePtr> paramTypes;
  size_t numParams = params_.size();
  for (size_t i = 0; i < numParams; ++i) {
    Info exprInfo = params_[i]->toImExpr(ctx);
    paramImExprs.push_back(move(exprInfo.imExpr));
    paramTypes.push_back(move(exprInfo.type));
  }

  const Class* classTy = static_cast<const Class*>(eInfo.type.get());
  auto classFnInfo =
      ctx.lookupMethod(classTy->id, classTy->className, methodName_, paramTypes, line_);
  const Ctx::FnInfo* fnInfo = classFnInfo.second;
  if (!fnInfo) {
    // Undefined function
    return dummyInfo();
  }

  checkAccess(methodName_, *fnInfo, *classTy, line_, ctx);

  if (eInfo.type->isConst && !Func::isConst(fnInfo->modifiers)) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Object of type '" << *eInfo.type << "' cannot invoke non-const method '"
        << classTy->className << "::" << methodName_;
    Type::streamParamTypes(fnInfo->paramTypes, err);
    err << '\'';
  }

  if (Func::isVirtual(fnInfo->modifiers)) {
    vector<im::StmtPtr> assignThis;
    int tThis = newTemp();
    assignThis.push_back(make_unique<im::Assign>(make_unique<im::Temp>(tThis), move(eInfo.imExpr)));
    paramImExprs.push_back(make_unique<im::Temp>(tThis));
    size_t vTableOffset = classFnInfo.first->vTableOffsets.at(fnInfo->id);
    return { make_unique<im::DoThenEval>(
                 move(assignThis),
                 make_unique<im::Call>(
                     // method = *(vTableAddr + offset)
                     make_unique<im::MemDeref>(
                         vTableOffset * 8,
                         // vTableAddr = *this
                         make_unique<im::MemDeref>(0, make_unique<im::Temp>(tThis), nullptr, 8),
                         nullptr,
                         8),
                     move(paramImExprs),
                     fnInfo->returnType != Type::VOID_TYPE)),
             move(fnInfo->returnType) };
  } else {
    // Push back "this" as last argument (AFTER the context lookup)
    paramImExprs.push_back(move(eInfo.imExpr));
    return { make_unique<im::Call>(
                 make_unique<im::LabelAddr>(Ctx::mangleFn(methodName_, fnInfo->id)),
                 move(paramImExprs),
                 fnInfo->returnType != Type::VOID_TYPE),
             move(fnInfo->returnType) };
  }
}

pair<Expr::Info, bool> MethodInvocation::resolveObjExpr(Ctx& ctx) {
  // Parser cannot distinguish var.method() and ClassName.method(), so need to check if var
  // exists, and if not, assume it is a className
  if (!objExpr_) {
    const Ctx::VarInfo* varInfo = ctx.lookupVarNoError(ident_);
    if (!varInfo) {
      return { QualifiedInvocation({}, ident_, methodName_, move(params_), line_).toImExpr(ctx),
               true };
    }
    return { Expr::Info{ make_unique<im::Temp>(varInfo->temp), varInfo->type }, false };
  }
  return { objExpr_->toImExpr(ctx), false };
}

/***********************
 * QualifiedInvocation *
 ***********************/

Expr::Info QualifiedInvocation::toImExpr(Ctx& ctx) {
  ctx.checkType(classTy_, line_);

  // Get types and convert params
  vector<im::ExprPtr> paramImExprs;
  vector<TypePtr> paramTypes;
  size_t numParams = params_.size();
  for (size_t i = 0; i < numParams; ++i) {
    Info exprInfo = params_[i]->toImExpr(ctx);
    paramImExprs.push_back(move(exprInfo.imExpr));
    paramTypes.push_back(move(exprInfo.type));
  }

  auto classFnInfo =
      ctx.lookupMethod(classTy_.id, classTy_.className, methodName_, paramTypes, line_);

  const Ctx::FnInfo* fnInfo = classFnInfo.second;
  if (!fnInfo) {
    // Undefined function
    return dummyInfo();
  }

  checkAccess(methodName_, *fnInfo, classTy_, line_, ctx);

  const ClassDecl* enclosingClass = ctx.insideClass();
  if (!Func::isStatic(fnInfo->modifiers)) {
    if (!(enclosingClass && ctx.isSubClassOf(enclosingClass->id_, classTy_))) {
      ostream& err = ctx.getLogger().logError(line_);
      err << "Can only invoke qualified non-static method '"
          << lang_utils::qualifiedName(classTy_.qualifiers, classTy_.className)
          << "::" << methodName_;
      Type::streamParamTypes(paramTypes, err);
      err << "' from within a subclass of '" << enclosingClass->name_ << '\'';
      return dummyInfo();
    }

    paramImExprs.push_back(Var(lang_utils::THIS, line_).toImExpr(ctx).imExpr);
  }

  return { make_unique<im::Call>(
               make_unique<im::LabelAddr>(Ctx::mangleFn(methodName_, fnInfo->id)),
               move(paramImExprs),
               fnInfo->returnType != Type::VOID_TYPE),
           move(fnInfo->returnType) };
}

/**********
 * IncDec *
 **********/

Expr::Info IncDec::toImExpr(Ctx& ctx) {
  BOp bOp = inc_ ? BOp::PLUS : BOp::MINUS;
  vector<im::StmtPtr> imStmts;
  size_t eLine = lValue_->line_;
  Info eInfo = lValue_->toImExpr(ctx);
  im::Expr::Category category = eInfo.imExpr->getCategory();

  if (category == im::Expr::Category::TEMP) {
    int tVar = static_cast<im::Temp*>(eInfo.imExpr.get())->t_;
    int tPostRes;

    if (!pre_) {
      // movq var, tPostRes (post)
      tPostRes = newTemp();
      imStmts.push_back(
          make_unique<im::Assign>(make_unique<im::Temp>(tPostRes), make_unique<im::Temp>(tVar)));
    }

    // var +/-= 1
    Update(
        make_unique<ImWrapper>(move(eInfo.imExpr), eInfo.type, eLine),
        bOp,
        make_unique<ConstInt>(1, line_))
        .toImStmts(imStmts, ctx);

    // return var (pre) or tPostRes (post)
    return { make_unique<im::DoThenEval>(
                 move(imStmts), make_unique<im::Temp>(pre_ ? tVar : tPostRes)),
             move(eInfo.type) };

  } else if (category == im::Expr::Category::MEM_DEREF) {
    // As with Update, we have to make sure we don't duplicated any side effects
    // by calculating the memory address twice

    im::MemDeref* memDeref = static_cast<im::MemDeref*>(eInfo.imExpr.get());

    // movq &expr, tAddr
    int tAddr = newTemp();
    imStmts.push_back(Update::assignAddr(tAddr, memDeref));

    int tPostRes;
    if (!pre_) {
      // movq tAddr, tPosRes (post)
      tPostRes = newTemp();
      imStmts.push_back(make_unique<im::Assign>(
          make_unique<im::Temp>(tPostRes), Update::derefTemp(tAddr, memDeref->numBytes_)));
    }

    // *tAddr +/-= 1
    Update(
        make_unique<ImWrapper>(Update::derefTemp(tAddr, memDeref->numBytes_), eInfo.type, eLine),
        bOp,
        make_unique<ConstInt>(1, line_))
        .toImStmts(imStmts, ctx);

    // return *tAddr (pre) or *tPostRes (post)
    return { make_unique<im::DoThenEval>(
                 move(imStmts),
                 pre_ ? Update::derefTemp(tAddr, memDeref->numBytes_)
                      : (im::ExprPtr)make_unique<im::Temp>(tPostRes)),
             move(eInfo.type) };
  } else {
    // All lvalues translate into a Temp or a MemDeref
    ostream& err = ctx.getLogger().logError(line_);
    err << "Operator '" << (inc_ ? "++" : "--") << "' requires an lvalue.";
    return dummyInfo();
  }
}


/*************
 * ImWrapper *
 *************/

Expr::Info ImWrapper::toImExpr(Ctx&) { return { move(imExpr_), type_ }; }


/********
 * Expr *
 ********/


template <typename F>
Expr::Info Expr::toImExprAssert(F&& condFn, string_view errMsg, Ctx& ctx) {
  Info exprInfo = toImExpr(ctx);
  if (!condFn(*exprInfo.type)) {
    ctx.getLogger().logError(line_, errMsg);
  }
  return exprInfo;
}


im::ExprPtr Expr::toImExprAssert(const Type& type, Ctx& ctx) {
  Info exprInfo = toImExpr(ctx);
  const Type& eType = *exprInfo.type;
  bool isNarrowing;
  if (eType.isConvertibleTo(type, &isNarrowing, ctx)) {
    if (isNarrowing) {
      ostream& warning = ctx.getLogger().logWarning(line_);
      warning << "Narrowing conversion from '" << eType << "' to '" << type << '\'';
    }
  } else {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Expected '" << type << "', but got '" << eType << '\'';
  }

  return move(exprInfo.imExpr);
}


}  // namespace language
