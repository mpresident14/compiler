#include "src/language/language.hpp"

using namespace std;

namespace language {

/***********
 * UnaryOp *
 ***********/


void UnaryOp::asBool(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    bool flipEquiv,
    Ctx& ctx) const {
  // Only valid for NOT
  if (uOp_ == UOp::NOT) {
    return e_->asBool(imStmts, ifFalse, ifTrue, !flipEquiv, ctx);
  }
  ctx.getLogger().logError(line_, "Cannot interpret unary minus at bool");
}


/************
 * BinaryOp *
 ************/
void BinaryOp::asBool(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    bool flipEquiv,
    Ctx& ctx) const {
  switch (bOp_) {
    case BOp::EQ:
      if (flipEquiv) {
        return asBoolComp(imStmts, ifFalse, ifTrue, im::ROp::NEQ, ctx);
      }
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::EQ, ctx);
    case BOp::NEQ:
      if (flipEquiv) {
        return asBoolComp(imStmts, ifFalse, ifTrue, im::ROp::EQ, ctx);
      }
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::NEQ, ctx);
    case BOp::LT:
      if (flipEquiv) {
        return asBoolComp(imStmts, ifFalse, ifTrue, im::ROp::GTE, ctx);
      }
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::LT, ctx);
    case BOp::LTE:
      if (flipEquiv) {
        return asBoolComp(imStmts, ifFalse, ifTrue, im::ROp::GT, ctx);
      }
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::LTE, ctx);
    case BOp::GT:
      if (flipEquiv) {
        return asBoolComp(imStmts, ifFalse, ifTrue, im::ROp::LTE, ctx);
      }
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::GT, ctx);
    case BOp::GTE:
      if (flipEquiv) {
        return asBoolComp(imStmts, ifFalse, ifTrue, im::ROp::LT, ctx);
      }
      return asBoolComp(imStmts, ifTrue, ifFalse, im::ROp::GTE, ctx);
    case BOp::AND:
      return asBoolAnd(imStmts, ifTrue, ifFalse, flipEquiv, ctx);
    case BOp::OR:
      return asBoolOr(imStmts, ifTrue, ifFalse, flipEquiv, ctx);
    case BOp::XOR:
      return asBoolXor(imStmts, ifTrue, ifFalse, flipEquiv, ctx);
    default:
      // Type error, let this function handle it
      return Expr::asBool(imStmts, ifTrue, ifFalse, flipEquiv, ctx);
  }
}

void BinaryOp::asBoolComp(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    im::ROp rOp,
    Ctx& ctx) const {
  // Make sure we are comparing two integral types
  ExprInfo info1 = e1_->toImExpr(ctx);
  ExprInfo info2 = e2_->toImExpr(ctx);
  if (!(info1.type->isIntegral && info2.type->isIntegral)) {
    ostream& err = ctx.getLogger().logError(
        line_, "Comparison operator requires integral types. Got ");
    err << *info1.type << " and " << *info2.type;
  }
  imStmts.emplace_back(new im::CondJump(
      move(info1.imExpr), move(info2.imExpr), rOp, ifTrue, ifFalse));
}

void BinaryOp::asBoolAnd(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    bool flipEquiv,
    Ctx& ctx) const {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  assem::Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, midLabel, ifFalse, flipEquiv, ctx);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse, flipEquiv, ctx);
}

void BinaryOp::asBoolOr(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    bool flipEquiv,
    Ctx& ctx) const {
  unique_ptr<im::MakeLabel> mkMidLabel = make_unique<im::MakeLabel>(newLabel());
  assem::Label* midLabel = mkMidLabel->genInstr();
  e1_->asBool(imStmts, ifTrue, midLabel, !flipEquiv, ctx);
  imStmts.emplace_back(move(mkMidLabel));
  e2_->asBool(imStmts, ifTrue, ifFalse, flipEquiv, ctx);
}


/*
 * XOR doesn't short-circuit, so we can simply XOR the booleans and
 * compare to 0
 */
void BinaryOp::asBoolXor(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    bool flipEquiv,
    Ctx& ctx) const {
  im::ExprPtr imXor = make_unique<im::BinOp>(
      e1_->toImExprAssert(*boolType, ctx),
      e2_->toImExprAssert(*boolType, ctx),
      im::BOp::XOR);
  if (flipEquiv) {
    imStmts.emplace_back(new im::CondJump(
        move(imXor), make_unique<im::Const>(0), im::ROp::EQ, ifFalse, ifTrue));
  } else {
    imStmts.emplace_back(new im::CondJump(
        move(imXor), make_unique<im::Const>(0), im::ROp::NEQ, ifTrue, ifFalse));
  }
}


/********
 * Expr *
 ********/

void Expr::asBool(
    vector<im::StmtPtr>& imStmts,
    assem::Label* ifTrue,
    assem::Label* ifFalse,
    bool flipEquiv,
    Ctx& ctx) const {
  if (flipEquiv) {
    imStmts.emplace_back(new im::CondJump(
        toImExprAssert(*boolType, ctx),
        make_unique<im::Const>(0),
        im::ROp::EQ,
        ifFalse,
        ifTrue));
  } else {
    imStmts.emplace_back(new im::CondJump(
        toImExprAssert(*boolType, ctx),
        make_unique<im::Const>(0),
        im::ROp::NEQ,
        ifTrue,
        ifFalse));
  }
}

}  // namespace language
