#include "src/language/language.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/typecheck/type.hpp"
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


im::StmtPtr If::toImStmts(vector<im::StmtPtr>& imStmts) {
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

im::StmtPtr While::toImStmts(vector<im::StmtPtr>& imStmts) {
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

im::StmtPtr CallStmt::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  // Ensure function was declared
  const Context::FnInfo& fnInfo = ctx.lookupFn(name_);
  // Ensure parameter types match and translate them to intermediate exprs
  const vector<TypePtr>& paramTypes = fnInfo.paramTypes;
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

// im::StmtPtr Return::toImStmts() const {
//   const TypePtr& retType = ctx.getReturnTy();
//   if (retValue_.has_value()) {
//     (*retValue_)->assertType(retType);
//   } else if (retType->getType() != TypeName::VOID) {
//       typeError();
//   }
// }

// im::StmtPtr Assign::toImStmts() const {
//   if (lhs_->getType() != ExprType::VAR) {
//     typeError();
//   }

//   const Context::VarInfo& varInfo = ctx.lookupVar(static_cast<Var*>(lhs_.get())->getName());
//   rhs_->assertType(varInfo.type);
// }


im::ExprPtr ConstInt::toImExpr() const {
  return make_unique<im::Const>(n_);
}

im::ExprPtr ConstBool::toImExpr() const {
  return make_unique<im::Const>(b_);
}

im::ExprPtr Var::toImExpr() const {
  return make_unique<im::Temp>(ctx.lookupVar(name_).temp);
}


void Expr::asBool(vector<im::StmtPtr>& imStmts, Label* ifTrue, Label* ifFalse)const {
  imStmts.emplace_back(new im::CondJump(
      toImExprAssert(make_unique<BoolTy>()),
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
  im::ExprPtr imE1 = e1_->toImExprAssert(make_unique<IntTy>());
  im::ExprPtr imE2 = e2_->toImExprAssert(make_unique<IntTy>());
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
