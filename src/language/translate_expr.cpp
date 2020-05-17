#include "src/language/language.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/x86gen/instruction.hpp"

using namespace std;

namespace language {


string newLabel() {
  static int i = 0;
  return "L" + to_string(i++);
}

Context ctx;

void typeError(string errMsg);



If::If(ExprPtr&& boolE, unique_ptr<Block>&& ifE, unique_ptr<Block>&& elseE)
    : boolE_(move(boolE)), ifE_(move(ifE)), elseE_(move(elseE)) {}


/*************
 * toImStmts *
 *************/
void Block::toImStmts(vector<im::StmtPtr>& imStmts) {
  // Keep track of variables declared in this scope
  vector<string> newVars;
  for (StmtPtr& stmt : stmts_) {
    stmt->toImStmts(imStmts);
    // TODO: Remove this and replace with StmtType
    if (VarDecl* varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
      newVars.push_back(varDecl->getName());
    }
  }

  // When we exit the block remove declared variables from this scope
  for (const string& var : newVars) {
    ctx.removeVar(var);
  }
}

void If::toImStmts(vector<im::StmtPtr>& imStmts) {
  unique_ptr<im::MakeLabel> mkIfLabel = make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkElseLabel = make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel = make_unique<im::MakeLabel>(newLabel());

  boolE_->asBool(imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr());
  imStmts.emplace_back(move(mkElseLabel));
  elseE_->toImStmts(imStmts);
  imStmts.emplace_back(new im::Jump(mkDoneLabel->genInstr()));
  imStmts.emplace_back(move(mkIfLabel));
  ifE_->toImStmts(imStmts);
  imStmts.emplace_back(move(mkDoneLabel));
}

void While::toImStmts(vector<im::StmtPtr>& imStmts) {
  unique_ptr<im::MakeLabel> mkBodyLabel = make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel = make_unique<im::MakeLabel>(newLabel());
  Label* bodyLabel = mkBodyLabel->genInstr();
  Label* doneLabel = mkDoneLabel->genInstr();
  // TODO: Not sure if possible (easy) b/c of uniqptrs, but don't compute this twice
  boolE_->asBool(imStmts, bodyLabel, doneLabel);
  imStmts.emplace_back(move(mkBodyLabel));
  body_->toImStmts(imStmts);
  boolE_->asBool(imStmts, bodyLabel, doneLabel);
  imStmts.emplace_back(move(mkDoneLabel));
}

void CallStmt::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  // Ensure function was declared
  const Context::FnInfo& fnInfo = ctx.lookupFn(name_);
  // Ensure parameter types match and translate them to intermediate exprs
  const vector<Type>& paramTypes = fnInfo.paramTypes;
  size_t numParams = params_.size();
  if (numParams != paramTypes.size()) {
    typeError("Wrong number of arguments for function: " + name_);
  }
  std::vector<im::ExprPtr> argsCode(numParams);
  for (size_t i = 0; i < numParams; ++i) {
    argsCode.push_back(params_[i]->toImExprAssert(paramTypes[i]));
  }
  imStmts.emplace_back(new im::CallStmt(make_unique<im::LabelAddr>(name_), move(argsCode)));
}

void Return::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  const Type& retType = ctx.getReturnTy();
  if (!retValue_.has_value()) {
    // Make sure the function return type is void
    if (retType.type != TypeName::VOID) {
      typeError("Should return void");
    }
    imStmts.emplace_back(new im::ReturnStmt(nullptr));
  } else {
    // Make sure the function return type matches and translate the returned expression
    imStmts.emplace_back(new im::ReturnStmt((*retValue_)->toImExprAssert(retType)));
  }
}

void Assign::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  if (lhs_->getType() != ExprType::VAR) {
    typeError("Only variables can be assigned");
  }

  const Context::VarInfo& varInfo = ctx.lookupVar(static_cast<Var*>(lhs_.get())->getName());
  imStmts.emplace_back(new im::Assign(
      make_unique<im::Temp>(varInfo.temp),
      rhs_->toImExprAssert(varInfo.type)));
}

void VarDecl::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  // Make sure the right side has the correct type and translate it
  im::ExprPtr rhs = e_->toImExprAssert(type_);
  // Insert the variable into the context
  int temp = ctx.insertVar(name_, move(type_));
  imStmts.emplace_back(new im::Assign(make_unique<im::Temp>(temp), move(rhs)));
}


/************
 * toImExpr *
 ************/
ExprInfo ConstInt::toImExpr() {
  return { make_unique<im::Const>(n_), intType };
}

ExprInfo ConstBool::toImExpr() {
  return { make_unique<im::Const>(b_), boolType };
}

ExprInfo Var::toImExpr() {
  const Context::VarInfo& varInfo = ctx.lookupVar(name_);
  return { make_unique<im::Temp>(varInfo.temp), varInfo.type };
}

ExprInfo UnaryOp::toImExpr() {
  switch (uOp_) {
    case UOp::NOT:
      // !b = b ^ 1
      return { make_unique<im::BinOp>(e_->toImExprAssert(intType), im::Const(1), im::BOp::XOR), intType};
    case UOp::NEG:
      return { make_unique<im::BinOp>(im::Const(0), e_->toImExprAssert(boolType), im::BOp::MINUS), boolType};
    default: throw invalid_argument("Unrecognized Uop");
  }
}

ExprInfo BinaryOp::toImExpr() {
  switch (bOp_) {
    case BOp::PLUS: return toImExprArith(im::BOp::PLUS);
    case BOp::MINUS: return toImExprArith(im::BOp::MINUS);
    case BOp::MUL: return toImExprArith(im::BOp::MUL);
    case BOp::DIV: return toImExprArith(im::BOp::DIV);
    default:
      return TernaryOp(
          // TODO: If there's a memory error, could be here
          make_unique<BinaryOp>(move(*this)),
          make_unique<ConstBool>(true),
          make_unique<ConstBool>(false)).toImExpr();
  }
}

ExprInfo BinaryOp::toImExprArith(im::BOp op) {
  return { make_unique<im::BinOp>(
      e1_->toImExprAssert(intType),
      e2_->toImExprAssert(intType),
      op), intType };
}




im::ExprPtr Expr::toImExprAssert(const Type& type) {
  ExprInfo exprInfo = toImExpr();
  if (exprInfo.type != type) {
    typeError("TODO: Add error message");
  }
  return move(exprInfo.imExpr);
}




void Expr::asBool(vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse)const {
  imStmts.emplace_back(new im::CondJump(
      toImExprAssert(boolType),
      make_unique<im::Const>(0),
      im::ROp::EQ,
      ifTrue,
      ifFalse));
}

void UnaryOp::asBool(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse) const {
  // Only valid for NOT
  if (uOp_ == UOp::NEG) {
    typeError("- cannot be interpreted as a boolean");
  }
  return e_->asBool(imStmts, ifFalse, ifTrue);
}

void BinaryOp::asBool(vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse)const {
  switch (bOp_) {
    case BOp::EQ: return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::EQ);
    case BOp::NEQ: return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::NEQ);
    case BOp::LT: return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::LT);
    case BOp::LTE: return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::LTE);
    case BOp::GT: return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::GT);
    case BOp::GTE: return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::GTE);
    case BOp::AND: return asBoolAnd(imStmts, ifTrue, ifFalse);
    case BOp::OR: return asBoolOr(imStmts, ifTrue, ifFalse);
    default: return Expr::asBool(imStmts, ifTrue, ifFalse);
  }
}

void BinaryOp::asBoolComp(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse, im::ROp rOp) const {
  // Make sure we are comparing two ints
  im::ExprPtr imE1 = e1_->toImExprAssert(intType);
  im::ExprPtr imE2 = e2_->toImExprAssert(intType);
  imStmts.emplace_back(new im::CondJump(move(imE1), move(imE2), rOp, ifTrue, ifFalse));
}

void BinaryOp::asBoolAnd(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse) const {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, midLabel, ifFalse);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse);
}

void BinaryOp::asBoolOr(std::vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse) const {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, ifTrue, midLabel);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse);
}


}
