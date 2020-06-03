#include "src/assembly/assembly.hpp"
#include "src/language/language.hpp"

#include <sstream>
#include <utility>

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
ExprInfo ConstInt::toImExpr(Ctx&) {
  return { make_unique<im::Const>(n_), intType };
}

/*************
 * ConstBool *
 *************/
ExprInfo ConstBool::toImExpr(Ctx&) {
  return { make_unique<im::Const>(b_), boolType };
}

/*******
 * Var *
 *******/

Var::Var(string_view name, size_t line) : Expr(line), name_(name) {}

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

UnaryOp::UnaryOp(ExprPtr&& e, UOp uOp, size_t line)
    : Expr(line), e_(move(e)), uOp_(uOp) {}

ExprInfo UnaryOp::toImExpr(Ctx& ctx) {
  switch (uOp_) {
    case UOp::NOT:
      // !b = b ^ 1
      return { make_unique<im::BinOp>(
                   e_->toImExprAssert(*boolType, ctx),
                   make_unique<im::Const>(1),
                   im::BOp::XOR),
               boolType };
    case UOp::NEG:
      return { make_unique<im::BinOp>(
                   make_unique<im::Const>(0),
                   e_->toImExprAssert(*intType, ctx),
                   im::BOp::MINUS),
               intType };
    default:
      throw invalid_argument("Unrecognized Uop");
  }
}

void UnaryOp::asBool(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    Ctx& ctx) {
  // Only valid for NOT
  if (uOp_ == UOp::NOT) {
    return e_->asBool(imStmts, ifFalse, ifTrue, ctx);
  }
  ctx.getLogger().logError(line_, "Cannot interpret unary minus at bool");
}


/************
 * BinaryOp *
 ************/
BinaryOp::BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp)
    : Expr(e1->getLine()), e1_(move(e1)), e2_(move(e2)), bOp_(bOp) {}

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
    case BOp::RSHIFT:
      return toImExprArith(im::BOp::ARSHIFT, ctx);
    case BOp::BIT_AND:
      return toImExprArith(im::BOp::AND, ctx);
    case BOp::BIT_OR:
      return toImExprArith(im::BOp::OR, ctx);
    case BOp::XOR:
      // XOR is valid for both ints and bools
      // This translates e1_ twice, but oh well
      if (e1_->toImExpr(ctx).type == intType) {
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

ExprInfo BinaryOp::toImExprArith(im::BOp op, Ctx& ctx) {
  return { make_unique<im::BinOp>(
               e1_->toImExprAssert(*intType, ctx),
               e2_->toImExprAssert(*intType, ctx),
               op),
           intType };
}

void BinaryOp::asBool(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    Ctx& ctx) {
  switch (bOp_) {
    case BOp::EQ:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::EQ, ctx);
    case BOp::NEQ:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::NEQ, ctx);
    case BOp::LT:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::LT, ctx);
    case BOp::LTE:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::LTE, ctx);
    case BOp::GT:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::GT, ctx);
    case BOp::GTE:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::GTE, ctx);
    case BOp::AND:
      return asBoolAnd(imStmts, ifTrue, ifFalse, ctx);
    case BOp::OR:
      return asBoolOr(imStmts, ifTrue, ifFalse, ctx);
    case BOp::XOR:
      return asBoolXor(imStmts, ifTrue, ifFalse, ctx);
    default:
      // Type error
      return Expr::asBool(imStmts, ifTrue, ifFalse, ctx);
  }
}

void BinaryOp::asBoolComp(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    im::ROp rOp,
    Ctx& ctx) {
  // Make sure we are comparing two ints
  im::ExprPtr imE1 = e1_->toImExprAssert(*intType, ctx);
  im::ExprPtr imE2 = e2_->toImExprAssert(*intType, ctx);
  imStmts.emplace_back(
      new im::CondJump(move(imE1), move(imE2), rOp, ifTrue, ifFalse));
}

void BinaryOp::asBoolAnd(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    Ctx& ctx) {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  assem::Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, midLabel, ifFalse, ctx);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse, ctx);
}

void BinaryOp::asBoolOr(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    Ctx& ctx) {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  assem::Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, ifTrue, midLabel, ctx);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse, ctx);
}

/*
 * XOR doesn't short-circuit, so we can simply XOR the booleans and
 * compare to 0
 */
void BinaryOp::asBoolXor(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    Ctx& ctx) {
  im::ExprPtr imXor = make_unique<im::BinOp>(
      e1_->toImExprAssert(*boolType, ctx),
      e2_->toImExprAssert(*boolType, ctx),
      im::BOp::XOR);
  imStmts.emplace_back(new im::CondJump(
      move(imXor), make_unique<im::Const>(0), im::ROp::EQ, ifFalse, ifTrue));
}


/*************
 * TernaryOp *
 *************/
TernaryOp::TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2)
    : Expr(boolE->getLine()),
      boolE_(move(boolE)),
      e1_(move(e1)),
      e2_(move(e2)) {}

/* This is really similar to If, but using If directy would force use to
 * use a Temp and also do typechecking twice for one of the expressions */
ExprInfo TernaryOp::toImExpr(Ctx& ctx) {
  // Make sure expressions are the same type
  ExprInfo exprInfo = e1_->toImExpr(ctx);
  im::ExprPtr imExpr2 = e2_->toImExprAssert(*exprInfo.type, ctx);

  unique_ptr<im::MakeLabel> mkIfLabel = make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkElseLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());

  int temp = newTemp();
  vector<im::StmtPtr> imStmts;

  boolE_->asBool(imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr(), ctx);
  imStmts.emplace_back(move(mkElseLabel));
  imStmts.emplace_back(
      new im::Assign(make_unique<im::Temp>(temp), move(imExpr2)));
  imStmts.emplace_back(new im::Jump(mkDoneLabel->genInstr()));
  imStmts.emplace_back(move(mkIfLabel));
  imStmts.emplace_back(
      new im::Assign(make_unique<im::Temp>(temp), move(exprInfo.imExpr)));
  imStmts.emplace_back(move(mkDoneLabel));
  return { make_unique<im::DoThenEval>(
               move(imStmts), make_unique<im::Temp>(temp)),
           move(exprInfo.type) };
}


/************
 * CallExpr *
 ************/

CallExpr::CallExpr(
    vector<string>&& qualifiers,
    string_view name,
    vector<ExprPtr>&& params,
    size_t line)
    : Expr(line),
      qualifiers_(move(qualifiers)),
      name_(name),
      params_(move(params)) {}

ExprInfo CallExpr::toImExpr(Ctx& ctx) {
  auto infoTupleOpt = argsToImExprs(qualifiers_, name_, params_, line_, ctx);
  if (!infoTupleOpt) {
    // Undefined function
    return dummyInfo();
  }
  auto& infoTuple = *infoTupleOpt;
  return { make_unique<im::CallExpr>(
               make_unique<im::LabelAddr>(get<0>(infoTuple)),
               move(get<1>(infoTuple)),
               get<2>(infoTuple) != voidType),
           move(get<2>(infoTuple)) };
}


/************
 * NewArray *
 ************/

NewArray::NewArray(TypePtr&& type, size_t numElems, size_t line)
    : Expr(line), type_(move(type)), numElems_(numElems) {}

ExprInfo NewArray::toImExpr(Ctx&) {
  int t = newTemp();
  vector<im::ExprPtr> mallocParam;
  // Arrays will start with the number of elements they contain
  size_t arrSize = 8 + numElems_ * type_->numBytes;
  mallocParam.emplace_back(new im::Const(arrSize));

  vector<im::StmtPtr> stmts;
  im::StmtPtr callMalloc = make_unique<im::Assign>(
      make_unique<im::Temp>(t),
      make_unique<im::CallExpr>(
          make_unique<im::LabelAddr>("__malloc"), move(mallocParam), true));

  im::StmtPtr setSize = make_unique<im::Assign>(
      make_unique<im::MemDeref>(make_unique<im::Temp>(t), type_->numBytes),
      make_unique<im::Const>(numElems_));

  // TODO: Zero/null initialize array
  // TODO: Throw if less than 0

  stmts.push_back(move(callMalloc));
  stmts.push_back(move(setSize));

  return { make_unique<im::DoThenEval>(move(stmts), make_unique<im::Temp>(t)),
           make_unique<Array>(type_) };
}

/***************
 * ArrayAccess *
 ***************/

ArrayAccess::ArrayAccess(ExprPtr&& expr, size_t index, size_t line)
    : Expr(line), expr_(move(expr)), index_(index) {}

// TODO: Throw if out of range

ExprInfo ArrayAccess::toImExpr(Ctx& ctx) {
  ExprInfo exprInfo = expr_->toImExpr(ctx);
  TypePtr& type = exprInfo.type;
  if (type->typeName != TypeName::ARRAY) {
    ostream& err = ctx.getLogger().logError(line_);
    err << "Operator[] can only be used on an arrays, not type " << *type;
    return dummyInfo();
  }

  // Add 8 bytes to skip the size field
  size_t offset = 8 + index_ * type->numBytes;
  im::ExprPtr offsetAddr = make_unique<im::BinOp>(
      move(exprInfo.imExpr),
      make_unique<im::Const>(offset),
      im::BOp::PLUS);

  return { make_unique<im::MemDeref>(move(offsetAddr), type->numBytes),
           static_cast<Array*>(type.get())->arrType };
}

/********
 * Expr *
 ********/

im::ExprPtr Expr::toImExprAssert(const Type& type, Ctx& ctx) {
  ExprInfo exprInfo = toImExpr(ctx);
  // TODO: Use isConvertible here later
  if (*exprInfo.type != type) {
    ctx.typeError(type, *exprInfo.type, line_);
  }
  return move(exprInfo.imExpr);
}


void Expr::asBool(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    Ctx& ctx) {
  imStmts.emplace_back(new im::CondJump(
      toImExprAssert(*boolType, ctx),
      make_unique<im::Const>(0),
      im::ROp::EQ,
      ifFalse,
      ifTrue));
}


}  // namespace language
