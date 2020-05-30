#include "src/assembly/assembly.hpp"
#include "src/language/language.hpp"

#include <sstream>
#include <utility>

using namespace std;


namespace language {

/************
 * ConstInt *
 ************/
ExprInfo ConstInt::toImExpr() {
  return { make_unique<im::Const>(n_), intType };
}

/*************
 * ConstBool *
 *************/
ExprInfo ConstBool::toImExpr() {
  return { make_unique<im::Const>(b_), boolType };
}

/*******
 * Var *
 *******/

Var::Var(string_view name, size_t line) : Expr(line), name_(name) {}

ExprInfo Var::toImExpr() {
  const ctx::VarInfo& varInfo = ctx::lookupVar(name_, line_);
  return { make_unique<im::Temp>(varInfo.temp), varInfo.type };
}

/***********
 * UnaryOp *
 ***********/

UnaryOp::UnaryOp(ExprPtr&& e, UOp uOp, size_t line)
    : Expr(line), e_(move(e)), uOp_(uOp) {}

ExprInfo UnaryOp::toImExpr() {
  switch (uOp_) {
    case UOp::NOT:
      // !b = b ^ 1
      return { make_unique<im::BinOp>(
                   e_->toImExprAssert(boolType),
                   make_unique<im::Const>(1),
                   im::BOp::XOR),
               boolType };
    case UOp::NEG:
      return { make_unique<im::BinOp>(
                   make_unique<im::Const>(0),
                   e_->toImExprAssert(intType),
                   im::BOp::MINUS),
               intType };
    default:
      throw invalid_argument("Unrecognized Uop");
  }
}

void UnaryOp::asBool(
    std::vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse) {
  // Only valid for NOT
  if (uOp_ == UOp::NOT) {
    return e_->asBool(imStmts, ifFalse, ifTrue);
  }
  ctx::getLogger().logError(line_, "Cannot interpret unary minus at bool");
}


/************
 * BinaryOp *
 ************/
BinaryOp::BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp)
    : Expr(e1->getLine()), e1_(move(e1)), e2_(move(e2)), bOp_(bOp) {}

ExprInfo BinaryOp::toImExpr() {
  switch (bOp_) {
    case BOp::PLUS:
      return toImExprArith(im::BOp::PLUS);
    case BOp::MINUS:
      return toImExprArith(im::BOp::MINUS);
    case BOp::MUL:
      return toImExprArith(im::BOp::MUL);
    case BOp::DIV:
      return toImExprArith(im::BOp::DIV);
    case BOp::MOD:
      return toImExprArith(im::BOp::MOD);
    case BOp::LSHIFT:
      return toImExprArith(im::BOp::LSHIFT);
    case BOp::RSHIFT:
      return toImExprArith(im::BOp::ARSHIFT);
    case BOp::BIT_AND:
      return toImExprArith(im::BOp::AND);
    case BOp::BIT_OR:
      return toImExprArith(im::BOp::OR);
    case BOp::XOR:
      // XOR is valid for both ints and bools
      // This translates e1_ twice, but oh well
      if (e1_->toImExpr().type == intType) {
        return toImExprArith(im::BOp::XOR);
      }
      // Fall thru
    default:
      // boolean operator
      return TernaryOp(
                 make_unique<BinaryOp>(move(*this)),
                 make_unique<ConstBool>(true, line_),
                 make_unique<ConstBool>(false, line_))
          .toImExpr();
  }
}

ExprInfo BinaryOp::toImExprArith(im::BOp op) {
  return { make_unique<im::BinOp>(
               e1_->toImExprAssert(intType), e2_->toImExprAssert(intType), op),
           intType };
}

void BinaryOp::asBool(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse) {
  switch (bOp_) {
    case BOp::EQ:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::EQ);
    case BOp::NEQ:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::NEQ);
    case BOp::LT:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::LT);
    case BOp::LTE:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::LTE);
    case BOp::GT:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::GT);
    case BOp::GTE:
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::GTE);
    case BOp::AND:
      return asBoolAnd(imStmts, ifTrue, ifFalse);
    case BOp::OR:
      return asBoolOr(imStmts, ifTrue, ifFalse);
    case BOp::XOR:
      return asBoolXor(imStmts, ifTrue, ifFalse);
    default:
      // Type error
      return Expr::asBool(imStmts, ifTrue, ifFalse);
  }
}

void BinaryOp::asBoolComp(
    std::vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    im::ROp rOp) {
  // Make sure we are comparing two ints
  im::ExprPtr imE1 = e1_->toImExprAssert(intType);
  im::ExprPtr imE2 = e2_->toImExprAssert(intType);
  imStmts.emplace_back(
      new im::CondJump(move(imE1), move(imE2), rOp, ifTrue, ifFalse));
}

void BinaryOp::asBoolAnd(
    std::vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse) {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  assem::Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, midLabel, ifFalse);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse);
}

void BinaryOp::asBoolOr(
    std::vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse) {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  assem::Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, ifTrue, midLabel);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse);
}

/*
 * XOR doesn't short-circuit, so we can simply XOR the booleans and
 * compare to 0
 */
void BinaryOp::asBoolXor(
    std::vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse) {
  im::ExprPtr imXor = make_unique<im::BinOp>(
      e1_->toImExprAssert(boolType),
      e2_->toImExprAssert(boolType),
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
ExprInfo TernaryOp::toImExpr() {
  // Make sure expressions are the same type
  ExprInfo exprInfo = e1_->toImExpr();
  im::ExprPtr imExpr2 = e2_->toImExprAssert(exprInfo.type);

  unique_ptr<im::MakeLabel> mkIfLabel = make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkElseLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());

  int temp = newTemp();
  vector<im::StmtPtr> imStmts;

  boolE_->asBool(imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr());
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
           exprInfo.type };
}


/************
 * CallExpr *
 ************/

CallExpr::CallExpr(string_view name, std::vector<ExprPtr>&& params, size_t line)
    : Expr(line), name_(name), params_(move(params)) {}

ExprInfo CallExpr::toImExpr() {
  pair<vector<im::ExprPtr>, Type> argCodes =
      argsToImExprs(name_, params_, line_);
  return { make_unique<im::CallExpr>(
               make_unique<im::LabelAddr>(name_),
               move(argCodes.first),
               argCodes.second != voidType),
           argCodes.second };
}


/************
 * NewArray *
 ************/

ExprInfo NewArray::toImExpr() {
  int t = newTemp();
  vector<ExprPtr> mallocParams;
  // Arrays will start with the number of elements they contain
  mallocParams.emplace_back(new im::Const((numElems_ + 1) * OBJ_SIZE));

  vector<im::StmtPtr> stmts;
  im::StmtPtr callMalloc = make_unique<im::Assign>(
      make_unique<im::Temp>(t),
      make_unique<im::CallExpr>(
          make_unique<im::LabelAddr>("__malloc"), move(mallocParams), true));

  im::StmtPtr setSize = make_unique<im::Assign>(make_unique<im::MemDeref>(
      make_unique<im::Temp>(t), make_unique<im::Const>(numElems_)));

  // TODO: Zero/null initialize array

  stmts.push_back(move(callMalloc));
  stmts.push_back(move(setSize));

  return { make_unique<im::DoThenEval>(move(stmts), make_unique<im::Temp>(t)),
           Array(type_) };
}

/***************
 * ArrayAccess *
 ***************/

ArrayAccess::ArrayAccess(ExprPtr&& expr, size_t index, size_t line)
    : Expr(line), expr_(move(expr)), index_(index) {}


ExprInfo ArrayAccess::toImExpr() {
  ExprInfo exprInfo = expr_->toImExpr();
  const Type& type = exprInfo.type;
  if (type.typeName != TypeName::ARRAY) {
    ostringstream err;
    err << "Expected an array type, got " << type;
    ctx::getLogger().logFatal(line_, err.str());
  }

  // Add 1 to skip the size field
  im::ExprPtr offsetAddr = make_unique<im::BinOp>(
      move(exprInfo.imExpr), make_unique<im::Const>((index_ + 1) * OBJ_SIZE));

  return { make_unique<im::MemDeref>(move(offsetAddr)),
           static_cast<const Array&>(type).arrType };
}

/********
 * Expr *
 ********/

im::ExprPtr Expr::toImExprAssert(const Type& type) {
  ExprInfo exprInfo = toImExpr();
  // TODO: Use isConvertible here later
  if (exprInfo.type != type) {
    typeError(type, exprInfo.type, line_);
  }
  return move(exprInfo.imExpr);
}


void Expr::asBool(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse) {
  imStmts.emplace_back(new im::CondJump(
      toImExprAssert(boolType),
      make_unique<im::Const>(0),
      im::ROp::EQ,
      ifFalse,
      ifTrue));
}


}  // namespace language
