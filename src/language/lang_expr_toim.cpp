#include "src/language/language.hpp"

#include <climits>

using namespace std;


namespace {

/* Allows us to proceed through compile errors so that we don't have to report
 * one at a time */
language::ExprInfo dummyInfo() { return { make_unique<im::Temp>(newTemp()), anyType }; }

}  // namespace


namespace language {

/************
 * ConstInt *
 ************/

ExprInfo ConstInt::toImExpr(Ctx&) {
  // Constant integers are the smallest type possible so that we allow implicit
  // conversion to larger types if required
  return { make_unique<im::Const>(n_), smallestIntegral(n_) };
}

/*************
 * ConstChar *
 *************/
ExprInfo ConstChar::toImExpr(Ctx&) { return { make_unique<im::Const>(c_), charType }; }

/*************
 * ConstBool *
 *************/
ExprInfo ConstBool::toImExpr(Ctx&) { return { make_unique<im::Const>(b_), boolType }; }

/*******
 * Var *
 *******/
bool Var::isLValue() const noexcept { return true; }

ExprInfo Var::toImExpr(Ctx& ctx) {
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

ExprInfo UnaryOp::toImExpr(Ctx& ctx) {
  switch (uOp_) {
    case UOp::NOT:
      // !b = b ^ 1
      return { make_unique<im::BinOp>(
                   e_->toImExprAssert(*boolType, ctx), make_unique<im::Const>(1), im::BOp::XOR),
               boolType };
    case UOp::NEG: {
      ExprInfo eInfo = e_->toImExprAssert(isIntegral, "Unary minus requires an integer.", ctx);
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

ExprInfo BinaryOp::toImExpr(Ctx& ctx) {
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
    case BOp::XOR:
      // XOR is valid for both ints and bools
      // This translates e1_ twice, but oh well
      if (isIntegral(*e1_->toImExpr(ctx).type)) {
        return toImExprArith(im::BOp::XOR, ctx);
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
    if (!isIntegral(type)) {
      ostringstream err;
      err << "Operator " << op << " expected integral types, got " << type;
      ctx.getLogger().logError(line, err.str());
      return false;
    }
    return true;
  }
}  // namespace

ExprInfo BinaryOp::toImExprArith(im::BOp op, Ctx& ctx) {
  ExprInfo eInfo1 = e1_->toImExpr(ctx);
  ExprInfo eInfo2 = e2_->toImExpr(ctx);
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

ExprInfo TernaryOp::toImExpr(Ctx& ctx) {
  size_t e1Line = e1_->line_;
  size_t e2Line = e2_->line_;
  ExprInfo e1Info = e1_->toImExpr(ctx);
  ExprInfo e2Info = e2_->toImExpr(ctx);
  if (!isConvertible(*e1Info.type, *e2Info.type, nullptr)) {
    ctx.getLogger().logError();
  }
  const TypePtr& widerType =
      e1Info.type->numBytes > e2Info.type->numBytes ? e1Info.type : e2Info.type;

  int t = newTemp();
  ExprPtr tempWrapper1 = make_unique<ImWrapper>(make_unique<im::Temp>(t), e1Info.type, true, e1Line);
  ExprPtr tempWrapper2 = make_unique<ImWrapper>(make_unique<im::Temp>(t), e2Info.type, true, e2Line);
  ExprPtr e1Wrapper = make_unique<ImWrapper>(move(e1Info.imExpr), e1Info.type, e1_->isLValue(), e1Line);
  ExprPtr e2Wrapper = make_unique<ImWrapper>(move(e2Info.imExpr), e2Info.type, e2_->isLValue(), e2Line);

  unique_ptr<Block> ifBlock = make_unique<Block>(vector<StmtPtr>{}, e1Line);
  ifBlock->stmts_.push_back(make_unique<Assign>(move(tempWrapper1), move(e1Wrapper)));

  unique_ptr<Block> elseBlock = make_unique<Block>(vector<StmtPtr>{}, e2Line);
  elseBlock->stmts_.push_back(make_unique<Assign>(move(tempWrapper2), move(e2Wrapper)));

  vector<im::StmtPtr> imStmts;
  If(move(boolE_), move(ifBlock), move(elseBlock), line_).toImStmts(imStmts, ctx);

  return { make_unique<im::DoThenEval>(move(imStmts), make_unique<im::Temp>(t)), move(widerType) };
}


/************
 * CallExpr *
 ************/

ExprInfo CallExpr::toImExpr(Ctx& ctx) {
  vector<im::ExprPtr> paramImExprs;
  vector<TypePtr> paramTypes;
  size_t numParams = params_.size();
  // Get types and convert params
  for (size_t i = 0; i < numParams; ++i) {
    ExprInfo exprInfo = params_[i]->toImExpr(ctx);
    paramImExprs.push_back(move(exprInfo.imExpr));
    paramTypes.push_back(move(exprInfo.type));
  }

  const Ctx::FnInfo* fnInfo = ctx.lookupFnRec(qualifiers_, name_, paramTypes, line_);
  if (!fnInfo) {
    // Undefined function
    return dummyInfo();
  }

  return { make_unique<im::CallExpr>(
               make_unique<im::LabelAddr>(
                   ctx.mangleFn(name_, fnInfo->declFile, fnInfo->paramTypes)),
               move(paramImExprs),
               fnInfo->returnType != voidType),
           move(fnInfo->returnType) };
}


/********
 * Cast *
 ********/

bool Cast::isLValue() const noexcept { return expr_->isLValue(); }

ExprInfo Cast::toImExpr(Ctx& ctx) {
  ExprInfo eInfo = expr_->toImExpr(ctx);
  if (!isConvertible(*eInfo.type, *toType_, nullptr)) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "No valid cast from " << *eInfo.type << " to " << *toType_;
  }
  return { make_unique<im::Cast>(move(eInfo.imExpr), toType_->numBytes), move(toType_) };
}


/************
 * NewArray *
 ************/

ExprInfo NewArray::toImExpr(Ctx& ctx) {
  if (numElems_) {
    return toImExprLen(ctx);
  }
  return toImExprElems(ctx);
}


ExprInfo NewArray::toImExprLen(Ctx& ctx) {
  auto p = makeArrayStmts(*type_, move(numElems_), ctx);

  return { make_unique<im::DoThenEval>(move(p.first), make_unique<im::Temp>(p.second)),
           make_unique<Array>(type_) };
}


ExprInfo NewArray::toImExprElems(Ctx& ctx) {
  size_t len = elems_.size();
  long maxLong = minMaxValue(*longType).second;
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

pair<vector<im::StmtPtr>, int>
NewArray::makeArrayStmts(const Type& type, ExprPtr&& numElems, Ctx& ctx) {
  int tLen = newTemp();

  // Store the length of the array in a temporary
  im::StmtPtr storeLen = make_unique<im::Assign>(
      make_unique<im::Temp>(tLen),
      numElems->toImExprAssert(isIntegral, "Array size requires an integral type", ctx).imExpr);

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
      make_unique<im::CallExpr>(make_unique<im::LabelAddr>("__malloc"), move(mallocBytes), true));

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

bool ArrayAccess::isLValue() const noexcept { return true; }

// TODO: Throw if out of range

ExprInfo ArrayAccess::toImExpr(Ctx& ctx) {
  ExprInfo exprInfo = arrExpr_->toImExpr(ctx);
  const Type& type = *exprInfo.type;
  if (type.typeName != TypeName::ARRAY) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Operator[] can only be used on an arrays, not type " << type;
    return dummyInfo();
  }

  const Type& arrType = *static_cast<const Array&>(type).arrType;

  im::ExprPtr imIndex =
      index_->toImExprAssert(isIntegral, "Operator[] requires an integral type", ctx).imExpr;

  // Add 8 bytes to skip the size field
  return { make_unique<im::MemDeref>(8, move(exprInfo.imExpr), move(imIndex), arrType.numBytes),
           static_cast<const Array*>(&type)->arrType };
}

/****************
 * MemberAccess *
 ****************/

bool MemberAccess::isLValue() const noexcept { return true; }

ExprInfo MemberAccess::toImExpr(Ctx& ctx) {
  ExprInfo eInfo = objExpr_->toImExpr(ctx);

  // The only member of an array is length
  if (eInfo.type->typeName == TypeName::ARRAY) {
    if (member_ == "length") {
      return { make_unique<im::MemDeref>(0, move(eInfo.imExpr), nullptr, 8), longType };
    }
    ctx.getLogger().logError(line_, "Array has no member " + member_);
    return dummyInfo();
  } else if (eInfo.type->typeName == TypeName::CLASS) {
    const Class* classTy = static_cast<const Class*>(eInfo.type.get());
    const Ctx::ClassInfo* classInfo = ctx.lookupClassRec(classTy->fullQuals, classTy->className);
    if (!classInfo) {
      throw runtime_error("MemberAccess::toImExpr");
    }
    auto iter = classInfo->fields.find(member_);
    if (iter == classInfo->fields.end()) {
      ostringstream& err = ctx.getLogger().logError(line_);
      err << "Field " << member_ << " is not defined in class " << classTy->className;
      return dummyInfo();
    }
    const Ctx::FieldInfo& fieldInfo = iter->second;
    return { make_unique<im::MemDeref>(
                 fieldInfo.offset, move(eInfo.imExpr), nullptr, fieldInfo.type->numBytes),
             fieldInfo.type };
  } else {
    return dummyInfo();
  }
}


/********************
 * MethodInvocation *
 ********************/

ExprInfo MethodInvocation::toImExpr(Ctx& ctx) {
  ExprInfo eInfo = objExpr_->toImExpr(ctx);
  if (eInfo.type->typeName != TypeName::CLASS) {
    ostringstream& err = ctx.getLogger().logError(line_);
    err << "Cannot invoke method on expression of type " << *eInfo.type;
    return dummyInfo();
  }

  const Class* classTy = static_cast<const Class*>(eInfo.type.get());
  // TODO: This code is duplicated from CallExpr::toImExpr
  vector<im::ExprPtr> paramImExprs;
  vector<TypePtr> paramTypes;
  size_t numParams = params_.size();
  // Get types and convert params
  for (size_t i = 0; i < numParams; ++i) {
    ExprInfo exprInfo = params_[i]->toImExpr(ctx);
    paramImExprs.push_back(move(exprInfo.imExpr));
    paramTypes.push_back(move(exprInfo.type));
  }
  // Push back "this" as last argument
  paramImExprs.push_back(move(eInfo.imExpr));
  paramTypes.push_back(eInfo.type);

  const Ctx::FnInfo* fnInfo =
      ctx.lookupMethod(classTy->fullQuals, methodName_, classTy->className, paramTypes, line_);
  if (!fnInfo) {
    // Undefined function
    return dummyInfo();
  }

  return { make_unique<im::CallExpr>(
               make_unique<im::LabelAddr>(ctx.mangleFn(
                   ClassDecl::mangleMethod(classTy->className, methodName_),
                   fnInfo->declFile,
                   fnInfo->paramTypes)),
               move(paramImExprs),
               fnInfo->returnType != voidType),
           move(fnInfo->returnType) };
}

/**********
 * IncDec *
 **********/

// TODO(BUG): Will probably fail with ((int) var)++;"
ExprInfo IncDec::toImExpr(Ctx& ctx) {
  BOp bOp = inc_ ? BOp::PLUS : BOp::MINUS;
  vector<im::StmtPtr> imStmts;

  if (lValue_->getType() == ExprType::VAR) {
    ExprInfo eInfo = lValue_->clone()->toImExpr(ctx);
    int tVar = static_cast<im::Temp*>(eInfo.imExpr.get())->t_;
    int tPostRes;

    if (!pre_) {
      // movq var, tPostRes (post)
      tPostRes = newTemp();
      imStmts.push_back(
          make_unique<im::Assign>(make_unique<im::Temp>(tPostRes), make_unique<im::Temp>(tVar)));
    }

    // var +/-= 1
    Update(move(lValue_), bOp, make_unique<ConstInt>(1, line_)).toImStmts(imStmts, ctx);

    // return var (pre) or tPostRes (post)
    return { make_unique<im::DoThenEval>(
                 move(imStmts), make_unique<im::Temp>(pre_ ? tVar : tPostRes)),
             move(eInfo.type) };

  } else {
    // As with Update, we have to make sure we don't duplicated any side effects
    // by calculating the memory address twice
    size_t eLine = lValue_->line_;
    ExprInfo eInfo = lValue_->toImExpr(ctx);
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
        make_unique<ImWrapper>(Update::derefTemp(tAddr, memDeref->numBytes_), eInfo.type, true, eLine),
        bOp,
        make_unique<ConstInt>(1, line_))
        .toImStmts(imStmts, ctx);

    // return *tAddr (pre) or *tPostRes (post)
    return { make_unique<im::DoThenEval>(
                 move(imStmts),
                 pre_ ? Update::derefTemp(tAddr, memDeref->numBytes_)
                      : (im::ExprPtr)make_unique<im::Temp>(tPostRes)),
             move(eInfo.type) };
  }
}


/*************
 * ImWrapper *
 *************/

bool ImWrapper::isLValue() const noexcept { return isLValue_; }

ExprInfo ImWrapper::toImExpr(Ctx&) { return { move(imExpr_), type_ }; }


/********
 * Expr *
 ********/

bool Expr::isLValue() const noexcept { return false; }

template <typename F>
ExprInfo Expr::toImExprAssert(F&& condFn, string_view errMsg, Ctx& ctx) {
  ExprInfo exprInfo = toImExpr(ctx);
  if (!condFn(*exprInfo.type)) {
    ctx.getLogger().logError(line_, errMsg);
  }
  return exprInfo;
}


im::ExprPtr Expr::toImExprAssert(const Type& type, Ctx& ctx) {
  ExprInfo exprInfo = toImExpr(ctx);
  const Type& eType = *exprInfo.type;
  bool isNarrowing;
  if (isConvertible(eType, type, &isNarrowing)) {
    if (isNarrowing) {
      ostream& warning = ctx.getLogger().logWarning(line_);
      warning << "Narrowing conversion from " << eType << " to " << type;
    }
  } else {
    ctx.typeError(type, eType, line_);
  }

  return move(exprInfo.imExpr);
}


}  // namespace language
