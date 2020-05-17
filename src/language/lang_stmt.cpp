#include "src/language/language.hpp"
#include "src/x86gen/instruction.hpp"

#include <utility>

using namespace std;

namespace language {

Context& ctx = Context::getContext();


  // If::If(ExprPtr&& boolE, unique_ptr<Block>&& ifE, unique_ptr<Block>&& elseE)
  //     : boolE_(move(boolE)), ifE_(move(ifE)), elseE_(move(elseE)) {}

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
  unique_ptr<im::MakeLabel> mkIfLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkElseLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());

  boolE_->asBool(imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr());
  imStmts.emplace_back(move(mkElseLabel));
  elseE_->toImStmts(imStmts);
  imStmts.emplace_back(new im::Jump(mkDoneLabel->genInstr()));
  imStmts.emplace_back(move(mkIfLabel));
  ifE_->toImStmts(imStmts);
  imStmts.emplace_back(move(mkDoneLabel));
}

void While::toImStmts(vector<im::StmtPtr>& imStmts) {
  unique_ptr<im::MakeLabel> mkBodyLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());
  Label* bodyLabel = mkBodyLabel->genInstr();
  Label* doneLabel = mkDoneLabel->genInstr();
  // TODO: Not sure if possible (easy) b/c of uniqptrs, but don't compute this
  // twice
  boolE_->asBool(imStmts, bodyLabel, doneLabel);
  imStmts.emplace_back(move(mkBodyLabel));
  body_->toImStmts(imStmts);
  boolE_->asBool(imStmts, bodyLabel, doneLabel);
  imStmts.emplace_back(move(mkDoneLabel));
}

void CallStmt::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  imStmts.emplace_back(
      new im::CallStmt(make_unique<im::LabelAddr>(name_), argsToImExprs(name_, params_).first));
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
    // Make sure the function return type matches and translate the returned
    // expression
    imStmts.emplace_back(
        new im::ReturnStmt((*retValue_)->toImExprAssert(retType)));
  }
}

void Assign::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  if (lhs_->getType() != ExprType::VAR) {
    typeError("Only variables can be assigned");
  }

  const Context::VarInfo& varInfo =
      ctx.lookupVar(static_cast<Var*>(lhs_.get())->getName());
  imStmts.emplace_back(new im::Assign(
      make_unique<im::Temp>(varInfo.temp),
      rhs_->toImExprAssert(varInfo.type)));
}

void VarDecl::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  // Make sure the right side has the correct type and translate it
  im::ExprPtr rhs = e_->toImExprAssert(type_);
  // Insert the variable into the context
  int temp = ctx.insertVar(name_, move(type_));
  imStmts.emplace_back(
      new im::Assign(make_unique<im::Temp>(temp), move(rhs)));
}


}
