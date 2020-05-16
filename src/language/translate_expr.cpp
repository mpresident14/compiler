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

void typeError();



If::If(ExprPtr&& boolE, unique_ptr<Block>&& ifE, unique_ptr<Block>&& elseE)
    : boolE_(move(boolE)), ifE_(move(ifE)), elseE_(move(elseE)) {}


// im::StmtPtr If::toImStmts(vector<im::Stmt>& imStmts) const {
//   im::ExprPtr imBoolE = boolE_->toImExprAssert(make_unique<BoolTy>());
//   imStmts.push_back(new im::CondJump)
//   ifE_->toImStmts();
//   elseE_->toImStmts();
// }

// im::StmtPtr While::toImStmts() const {
//   boolE_->assertType(make_unique<BoolTy>());
//   body_->toImStmts();
// }

// im::StmtPtr CallStmt::toImStmts() const {
//   ctx.lookupFn(name_);
// }

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


void Expr::asBool(vector<im::Stmt>& imStmts, Label* ifTrue, Label* ifFalse)const {
  imStmts.emplace_back(new im::CondJump(
      toImExprAssert(make_unique<BoolTy>()),
      make_unique<im::Const>(0),
      im::ROp::EQ,
      ifTrue,
      ifFalse));
}

void BinaryOp::asBool(vector<im::Stmt>& imStmts, Label* ifTrue, Label* ifFalse)const {
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

void BinaryOp::asBoolComp(std::vector<im::Stmt>& imStmts, Label* ifTrue, Label* ifFalse, im::ROp rOp) const {
  // Make sure we are comparing two ints
  im::ExprPtr imE1 = e1_->toImExprAssert(make_unique<IntTy>());
  im::ExprPtr imE2 = e2_->toImExprAssert(make_unique<IntTy>());
  imStmts.emplace_back(new im::CondJump(move(imE1), move(imE2), rOp, ifTrue, ifFalse));
}

void BinaryOp::asBoolAnd(std::vector<im::Stmt>& imStmts, Label* ifTrue, Label* ifFalse) const {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, midLabel, ifFalse);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse);
}

void BinaryOp::asBoolOr(std::vector<im::Stmt>& imStmts, Label* ifTrue, Label* ifFalse) const {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, ifTrue, midLabel);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse);
}


}
