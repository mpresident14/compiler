#include "src/language/language.hpp"

using namespace std;


namespace {

/* Allows us to proceed through compile errors so that we don't have to report
 * one at a time */
language::ExprInfo dummyInfo() {
  return { make_unique<im::Temp>(newTemp()), anyType };
}

}  // namespace


namespace language {

/************
 * ConstInt *
 ************/
ExprInfo ConstInt::toImExpr(Ctx&) const {
  // NOTE: Constant integers are intType by default, but since we store them
  // as a long, it allows an implicit conversion to longType if required
  return { make_unique<im::Const>(n_), intType };
}

/*************
 * ConstChar *
 *************/
ExprInfo ConstChar::toImExpr(Ctx&) const {
  return { make_unique<im::Const>(c_), charType };
}

/*************
 * ConstBool *
 *************/
ExprInfo ConstBool::toImExpr(Ctx&) const {
  return { make_unique<im::Const>(b_), boolType };
}

/*******
 * Var *
 *******/

ExprInfo Var::toImExpr(Ctx& ctx) const {
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

ExprInfo UnaryOp::toImExpr(Ctx& ctx) const {
  switch (uOp_) {
    case UOp::NOT:
      // !b = b ^ 1
      return { make_unique<im::BinOp>(
                   e_->toImExprAssert(*boolType, ctx),
                   make_unique<im::Const>(1),
                   im::BOp::XOR),
               boolType };
    case UOp::NEG: {
      ExprInfo eInfo = e_->toImExprAssert(
          isIntegral, "Unary minus requires an integer.", ctx);
      return {
        make_unique<im::BinOp>(
            make_unique<im::Const>(-1), move(eInfo.imExpr), im::BOp::MUL),
        move(eInfo.type)
      };
    }
    default:
      throw invalid_argument("Unrecognized Uop");
  }
}


namespace {

  /* This is a hacky way for us to use the TernaryOp code to evaulate a
   * boolean BinaryOp without moving the BinaryOp (to maintain constness).
   * This is really similar to If, but using If directy would force use to
   * use a Temp and also do typechecking twice for one of the expressions */
  // TODO: The above statement is wrong I think. Do this.
  ExprInfo
  ternaryEval(const Expr& boolE, const Expr& e1, const Expr& e2, Ctx& ctx) {
    // Make sure expressions are the same type
    ExprInfo exprInfo = e1.toImExpr(ctx);
    im::ExprPtr imExpr2 = e2.toImExprAssert(*exprInfo.type, ctx);

    unique_ptr<im::MakeLabel> mkIfLabel =
        make_unique<im::MakeLabel>(newLabel());
    unique_ptr<im::MakeLabel> mkElseLabel =
        make_unique<im::MakeLabel>(newLabel());
    unique_ptr<im::MakeLabel> mkDoneLabel =
        make_unique<im::MakeLabel>(newLabel());

    int temp = newTemp();
    vector<im::StmtPtr> imStmts;

    boolE.asBool(
        imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr(), true, ctx);
    imStmts.emplace_back(move(mkIfLabel));
    imStmts.emplace_back(
        new im::Assign(make_unique<im::Temp>(temp), move(exprInfo.imExpr)));
    imStmts.emplace_back(new im::Jump(mkDoneLabel->genInstr()));
    imStmts.emplace_back(move(mkElseLabel));
    imStmts.emplace_back(
        new im::Assign(make_unique<im::Temp>(temp), move(imExpr2)));
    imStmts.emplace_back(move(mkDoneLabel));
    return { make_unique<im::DoThenEval>(
                 move(imStmts), make_unique<im::Temp>(temp)),
             move(exprInfo.type) };
  }

}  // namespace


/************
 * BinaryOp *
 ************/

ExprInfo BinaryOp::toImExpr(Ctx& ctx) const {
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
      return ternaryEval(
          *this, ConstBool(true, line_), ConstBool(false, line_), ctx);
  }
}


ExprInfo BinaryOp::toImExprArith(im::BOp op, Ctx& ctx) const {
  ExprInfo eInfo1 = e1_->toImExpr(ctx);
  ExprInfo eInfo2 = e2_->toImExpr(ctx);
  TypePtr& type1 = eInfo1.type;
  TypePtr& type2 = eInfo2.type;
  if (!isIntegral(*type1)) {
    // TODO: Specify which operator
    ctx.getLogger().logError(
        e1_->getLine(), "Binary operator expected integral types");
  }
  if (!isIntegral(*type2)) {
    ctx.getLogger().logError(
        e2_->getLine(), "Binary operator expected integral types");
  }
  return { make_unique<im::BinOp>(move(eInfo1.imExpr), move(eInfo2.imExpr), op),
           type1->numBytes > type2->numBytes ? move(type1) : move(type2) };
}

/*************
 * TernaryOp *
 *************/

ExprInfo TernaryOp::toImExpr(Ctx& ctx) const {
  return ternaryEval(*boolE_, *e1_, *e2_, ctx);
}


/************
 * CallExpr *
 ************/

ExprInfo CallExpr::toImExpr(Ctx& ctx) const {
  vector<im::ExprPtr> paramImExprs;
  vector<TypePtr> paramTypes;
  size_t numParams = params_.size();
  // Get types and convert params
  for (size_t i = 0; i < numParams; ++i) {
    ExprInfo exprInfo = params_[i]->toImExpr(ctx);
    paramImExprs.push_back(move(exprInfo.imExpr));
    paramTypes.push_back(move(exprInfo.type));
  }

  const Ctx::FnInfo* fnInfo =
      ctx.lookupFnRec(qualifiers_, name_, paramTypes, line_);
  if (!fnInfo) {
    // Undefined function
    return dummyInfo();
  }

  return { make_unique<im::CallExpr>(
               make_unique<im::LabelAddr>(
                   ctx.mangleFn(name_, fnInfo->declFile, paramTypes)),
               move(move(paramImExprs)),
               fnInfo->returnType != voidType),
           move(fnInfo->returnType) };
}


/********
 * Cast *
 ********/

// TODO: These casts will not truncate unless stored in an array
// We really need to pass the size of an Expr down to the intermediate
// level so that it can generate the proper assembly
ExprInfo Cast::toImExpr(Ctx& ctx) const {
  ExprInfo eInfo = expr_->toImExpr(ctx);
  if (!isConvertible(*eInfo.type, *toType_, nullptr)) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "No valid cast from " << *eInfo.type << " to " << *toType_;
  }
  return { move(eInfo.imExpr), move(toType_) };
}


/************
 * NewArray *
 ************/

ExprInfo NewArray::toImExpr(Ctx& ctx) const {
  if (numElems_) {
    return toImExprLen(ctx);
  }
  return toImExprElems(ctx);
}


ExprInfo NewArray::toImExprLen(Ctx& ctx) const {
  int tLen = newTemp();

  // Store the length of the array in a temporary
  im::StmtPtr storeLen = make_unique<im::Assign>(
      make_unique<im::Temp>(tLen),
      numElems_
          ->toImExprAssert(
              isIntegral, "Array size requires an integral type", ctx)
          .imExpr);

  // Compute the size of the array in bytes
  im::ExprPtr mul = make_unique<im::BinOp>(
      make_unique<im::Temp>(tLen),
      make_unique<im::Const>(type_->numBytes),
      im::BOp::MUL);
  im::ExprPtr arrBytes = make_unique<im::BinOp>(
      move(mul), make_unique<im::Const>(8), im::BOp::PLUS);

  // Allocate the correct number of bytes
  int tArrAddr = newTemp();
  vector<im::ExprPtr> mallocBytes;
  mallocBytes.emplace_back(move(arrBytes));
  im::StmtPtr callMalloc = make_unique<im::Assign>(
      make_unique<im::Temp>(tArrAddr),
      make_unique<im::CallExpr>(
          make_unique<im::LabelAddr>("__malloc"), move(mallocBytes), true));

  // Arrays will start with the number of elements they contain
  im::StmtPtr setSize = make_unique<im::Assign>(
      make_unique<im::MemDeref>(make_unique<im::Temp>(tArrAddr), 8),
      make_unique<im::Temp>(tLen));

  // TODO: Zero/null initialize array
  // TODO: Throw if less than 0

  vector<im::StmtPtr> stmts;
  stmts.push_back(move(storeLen));
  stmts.push_back(move(callMalloc));
  stmts.push_back(move(setSize));

  return { make_unique<im::DoThenEval>(
               move(stmts), make_unique<im::Temp>(tArrAddr)),
           make_unique<Array>(type_) };
}


ExprInfo NewArray::toImExprElems(Ctx& ctx) const {
  size_t len = elems_.size();
  size_t elemSize = type_->numBytes;

  // Compute the size of the array in bytes
  im::ExprPtr mul = make_unique<im::BinOp>(
      make_unique<im::Const>(len),
      make_unique<im::Const>(elemSize),
      im::BOp::MUL);
  im::ExprPtr arrBytes = make_unique<im::BinOp>(
      move(mul), make_unique<im::Const>(8), im::BOp::PLUS);

  vector<im::StmtPtr> stmts;

  // Allocate the correct number of bytes
  int tArrAddr = newTemp();
  vector<im::ExprPtr> mallocBytes;
  mallocBytes.emplace_back(move(arrBytes));
  stmts.push_back(make_unique<im::Assign>(
      make_unique<im::Temp>(tArrAddr),
      make_unique<im::CallExpr>(
          make_unique<im::LabelAddr>("__malloc"), move(mallocBytes), true)));

  // Set the size of the array in the first 8 bytes
  stmts.push_back(make_unique<im::Assign>(
      make_unique<im::MemDeref>(make_unique<im::Temp>(tArrAddr), 8),
      make_unique<im::Const>(len)));

  // Add 8 to tArrAddr (move past size)
  int tNextElem = newTemp();
  stmts.push_back(make_unique<im::Assign>(
      make_unique<im::Temp>(tNextElem),
      make_unique<im::BinOp>(
          make_unique<im::Temp>(tArrAddr),
          make_unique<im::Const>(8),
          im::BOp::PLUS)));

  for (const ExprPtr& elem : elems_) {
    // Assign the element
    stmts.push_back(make_unique<im::Assign>(
        make_unique<im::MemDeref>(make_unique<im::Temp>(tNextElem), elemSize),
        elem->toImExprAssert(*type_, ctx)));

    // Update the assignment address
    stmts.push_back(make_unique<im::Assign>(
        make_unique<im::Temp>(tNextElem),
        make_unique<im::BinOp>(
            make_unique<im::Temp>(tNextElem),
            make_unique<im::Const>(elemSize),
            im::BOp::PLUS)));
  }

  return { make_unique<im::DoThenEval>(
               move(stmts), make_unique<im::Temp>(tArrAddr)),
           make_unique<Array>(type_) };
}


/***************
 * ArrayAccess *
 ***************/

// TODO: Throw if out of range

ExprInfo ArrayAccess::toImExpr(Ctx& ctx) const {
  ExprInfo exprInfo = arrExpr_->toImExpr(ctx);
  const Type& type = *exprInfo.type;
  if (type.typeName != TypeName::ARRAY) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Operator[] can only be used on an arrays, not type " << type;
    return dummyInfo();
  }

  const Type& arrType = *static_cast<const Array&>(type).arrType;

  // Add 8 bytes to skip the size field
  im::ExprPtr imIndex =
      index_
          ->toImExprAssert(
              isIntegral, "Operator[] requires an integral type", ctx)
          .imExpr;
  im::ExprPtr mul = make_unique<im::BinOp>(
      move(imIndex), make_unique<im::Const>(arrType.numBytes), im::BOp::MUL);
  im::ExprPtr offset = make_unique<im::BinOp>(
      move(mul), make_unique<im::Const>(8), im::BOp::PLUS);
  im::ExprPtr offsetAddr = make_unique<im::BinOp>(
      move(exprInfo.imExpr), move(offset), im::BOp::PLUS);

  return { make_unique<im::MemDeref>(move(offsetAddr), arrType.numBytes),
           static_cast<const Array*>(&type)->arrType };
}

/****************
 * MemberAccess *
 ****************/
ExprInfo MemberAccess::toImExpr(Ctx& ctx) const {
  ExprInfo eInfo = objExpr_->toImExpr(ctx);

  // The only member of an array is length
  if (eInfo.type->typeName == TypeName::ARRAY) {
    if (member_ == "length") {
      return { make_unique<im::MemDeref>(move(eInfo.imExpr), 8), longType };
    }
    ctx.getLogger().logError(line_, "Array has no member " + member_);
    return dummyInfo();
  }

  return dummyInfo();
}


/********
 * Expr *
 ********/
template <typename F>
ExprInfo Expr::toImExprAssert(F&& condFn, std::string_view errMsg, Ctx& ctx)
    const {
  ExprInfo exprInfo = toImExpr(ctx);
  if (!condFn(*exprInfo.type)) {
    ctx.getLogger().logError(line_, errMsg);
  }
  return exprInfo;
}


im::ExprPtr Expr::toImExprAssert(const Type& type, Ctx& ctx) const {
  ExprInfo exprInfo = toImExpr(ctx);
  const Type& eType = *exprInfo.type;
  // TODO: Use isConvertible here later
  if (eType != type) {
    bool isNarrowing;
    if (isConvertible(eType, type, &isNarrowing)) {
      // TODO: We don't want to report narrowing for short n = 3;
      // Check if the expression is a const, and then if the number fits in the
      // specified number of bytes
      if (isNarrowing) {
        long n;
        if (getType() == ExprType::CONST_INT) {
          n = static_cast<const ConstInt&>(*this).getInt();
        } else if (getType() == ExprType::CONST_CHAR) {
          n = static_cast<const ConstChar&>(*this).getChar();
        } else {
          goto warnNarrow;
        }

        pair<long, long> limits = minMaxValue(type);
        if (n < limits.first || n > limits.second) {
          goto warnNarrow;
        }
      }
    } else {
      ctx.typeError(type, eType, line_);
    }
  }

  return move(exprInfo.imExpr);

warnNarrow:
  ostream& warning = ctx.getLogger().logWarning(line_);
  warning << "Narrowing conversion from " << eType << " to " << type;
  return move(exprInfo.imExpr);
}


/**************
 * InfoHolder *
 **************/

ExprInfo InfoHolder::toImExpr(Ctx&) const {
  if (used_) {
    throw runtime_error("InfoHolder::toImExpr called twice");
  }
  return move(exprInfo_);
}

}  // namespace language
