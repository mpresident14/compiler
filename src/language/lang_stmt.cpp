#include "src/language/language.hpp"
#include "src/assembly/assembly.hpp"

#include <utility>

using namespace std;

namespace {
  Context& ctx = Context::getContext();
}

namespace language {


/*********
 * Block *
 *********/
Block::Block(std::vector<StmtPtr>&& stmts)
    : stmts_(move(stmts)) {}

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


/******
 * If *
 ******/

If::If(ExprPtr&& boolE, std::vector<StmtPtr>&& ifE, std::vector<StmtPtr>&& elseE)
      : boolE_(move(boolE)), ifE_(make_unique<Block>(move(ifE))), elseE_(make_unique<Block>(move(elseE))) {}

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


/*********
 * While *
 *********/

While::While(ExprPtr&& boolE, std::unique_ptr<Block> body)
    : boolE_(move(boolE)), body_(move(body)) {}

void While::toImStmts(vector<im::StmtPtr>& imStmts) {
  unique_ptr<im::MakeLabel> mkBodyLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());
  assem::Label* bodyLabel = mkBodyLabel->genInstr();
  assem::Label* doneLabel = mkDoneLabel->genInstr();
  // TODO: Not sure if possible (easy) b/c of uniqptrs, but don't compute this
  // twice
  boolE_->asBool(imStmts, bodyLabel, doneLabel);
  imStmts.emplace_back(move(mkBodyLabel));
  body_->toImStmts(imStmts);
  boolE_->asBool(imStmts, bodyLabel, doneLabel);
  imStmts.emplace_back(move(mkDoneLabel));
}


/************
 * CallStmt *
 ************/
CallStmt::CallStmt(const std::string& name, std::vector<ExprPtr>&& params)
    : name_(name), params_(move(params)) {}

void CallStmt::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  imStmts.emplace_back(
      new im::CallStmt(make_unique<im::LabelAddr>(name_), argsToImExprs(name_, params_).first));
}


/**********
 * Return *
 **********/

Return::Return(std::optional<ExprPtr>&& retValue)
    : retValue_(move(retValue)) {}

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


/**********
 * Assign *
 **********/

Assign::Assign(ExprPtr&& lhs, ExprPtr&& rhs)
    : lhs_(move(lhs)), rhs_(move(rhs)) {}

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


/***********
 * VarDecl *
 ***********/

VarDecl::VarDecl(const Type& type, const std::string& name, ExprPtr&& e)
    : type_(type), name_(name), e_(move(e)) {}

void VarDecl::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  // Make sure the right side has the correct type and translate it
  im::ExprPtr rhs = e_->toImExprAssert(type_);
  // Insert the variable into the context
  int temp = ctx.insertVar(name_, move(type_));
  imStmts.emplace_back(
      new im::Assign(make_unique<im::Temp>(temp), move(rhs)));
}


}
