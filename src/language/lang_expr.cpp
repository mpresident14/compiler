#include "src/language/language.hpp"
#include "src/assembly/assembly.hpp"

#include <utility>

using namespace std;

namespace {
  Context& ctx = Context::getContext();
}

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

  Var::Var(const std::string& name)
      : name_(name) {}

  ExprInfo Var::toImExpr() {
    const Context::VarInfo& varInfo = ctx.lookupVar(name_);
    return { make_unique<im::Temp>(varInfo.temp), varInfo.type };
  }

  /***********
   * UnaryOp *
   ***********/

  UnaryOp::UnaryOp(ExprPtr&& e, UOp uOp)
      : e_(move(e)), uOp_(uOp) {}

  ExprInfo UnaryOp::toImExpr() {
    switch (uOp_) {
      case UOp::NOT:
        // !b = b ^ 1
        return { make_unique<im::BinOp>(
                     e_->toImExprAssert(intType), make_unique<im::Const>(1), im::BOp::XOR),
                 intType };
      case UOp::NEG:
        return {
          make_unique<im::BinOp>(
              make_unique<im::Const>(0), e_->toImExprAssert(boolType), im::BOp::MINUS),
          boolType
        };
      default:
        throw invalid_argument("Unrecognized Uop");
    }
  }

  void UnaryOp::asBool(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse) {
    // Only valid for NOT
    if (uOp_ == UOp::NEG) {
      typeError("- cannot be interpreted as a boolean");
    }
    return e_->asBool(imStmts, ifFalse, ifTrue);
  }


  /************
   * BinaryOp *
   ************/
  BinaryOp::BinaryOp(ExprPtr&& e1, ExprPtr&& e2, BOp bOp)
      : e1_(move(e1)), e2_(move(e2)), bOp_(bOp) {}

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
      default:
        return TernaryOp(
                   // TODO: If there's a memory error, it could be here
                   make_unique<BinaryOp>(move(*this)),
                   make_unique<ConstBool>(true),
                   make_unique<ConstBool>(false))
            .toImExpr();
    }
  }

  ExprInfo BinaryOp::toImExprArith(im::BOp op) {
    return {
      make_unique<im::BinOp>(
          e1_->toImExprAssert(intType), e2_->toImExprAssert(intType), op),
      intType
    };
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
      default:
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
    unique_ptr<im::MakeLabel> mkMidLabel =
        make_unique<im::MakeLabel>(newLabel());
    assem::Label* midLabel = mkMidLabel->genInstr();
    e1_->asBool(imStmts, midLabel, ifFalse);
    imStmts.emplace_back(move(mkMidLabel));
    e2_->asBool(imStmts, ifTrue, ifFalse);
  }

  void BinaryOp::asBoolOr(
      std::vector<im::StmtPtr>& imStmts,
      assem::Label* ifTrue,
      assem::Label* ifFalse) {
    unique_ptr<im::MakeLabel> mkMidLabel =
        make_unique<im::MakeLabel>(newLabel());
    assem::Label* midLabel = mkMidLabel->genInstr();
    e1_->asBool(imStmts, ifTrue, midLabel);
    imStmts.emplace_back(move(mkMidLabel));
    e2_->asBool(imStmts, ifTrue, ifFalse);
  }


  /*************
   * TernaryOp *
   *************/
  TernaryOp::TernaryOp(ExprPtr&& boolE, ExprPtr&& e1, ExprPtr&& e2)
      : boolE_(move(boolE)), e1_(move(e1)), e2_(move(e2)) {}

  /* This is really similar to If, but using If directy would force use to
   * use a Temp and also do typechecking twice for one of the expressions */
  ExprInfo TernaryOp::toImExpr() {
    // Make sure expressions are the same type
    ExprInfo exprInfo = e1_->toImExpr();
    im::ExprPtr imExpr2 = e2_->toImExprAssert(exprInfo.type);

    unique_ptr<im::MakeLabel> mkIfLabel =
        make_unique<im::MakeLabel>(newLabel());
    unique_ptr<im::MakeLabel> mkElseLabel =
        make_unique<im::MakeLabel>(newLabel());
    unique_ptr<im::MakeLabel> mkDoneLabel =
        make_unique<im::MakeLabel>(newLabel());

    int temp = newTemp();
    vector<im::StmtPtr> imStmts;

    boolE_->asBool(imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr());
    imStmts.emplace_back(move(mkElseLabel));
    imStmts.emplace_back(
        new im::Assign(make_unique<im::Temp>(temp), move(exprInfo.imExpr)));
    imStmts.emplace_back(new im::Jump(mkDoneLabel->genInstr()));
    imStmts.emplace_back(move(mkIfLabel));
    imStmts.emplace_back(
        new im::Assign(make_unique<im::Temp>(temp), move(imExpr2)));
    imStmts.emplace_back(move(mkDoneLabel));
    return { make_unique<im::DoThenEval>(
                 move(imStmts), make_unique<im::Temp>(temp)),
             exprInfo.type };
  }


  /************
   * CallExpr *
   ************/

  CallExpr::CallExpr(const std::string& name, std::vector<ExprPtr>&& params)
      : name_(name), params_(move(params)) {}

  ExprInfo CallExpr::toImExpr() {
    pair<vector<im::ExprPtr>, Type> argCodes = argsToImExprs(name_, params_);
    return {
        make_unique<im::CallExpr>(
            make_unique<im::LabelAddr>(name_),
            move(argCodes.first),
            argCodes.second != voidType),
        argCodes.second};
  }


  /********
   * Expr *
   ********/

  im::ExprPtr Expr::toImExprAssert(const Type& type) {
    ExprInfo exprInfo = toImExpr();
    // TODO: Use isConvertible here later
    if (exprInfo.type != type) {
      typeError("TODO: Add error message");
    }
    return move(exprInfo.imExpr);
  }


  void
  Expr::asBool(vector<im::StmtPtr>& imStmts, assem::Label* ifTrue, assem::Label* ifFalse) {
    imStmts.emplace_back(new im::CondJump(
        toImExprAssert(boolType),
        make_unique<im::Const>(0),
        im::ROp::EQ,
        ifTrue,
        ifFalse));
  }


}  // namespace language
