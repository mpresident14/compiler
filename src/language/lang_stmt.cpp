#include "src/assembly/assembly.hpp"
#include "src/language/language.hpp"

#include <sstream>
#include <utility>

using namespace std;

namespace {
Context& ctx = Context::getContext();
}

namespace language {


/*********
 * Block *
 *********/
Block::Block(std::vector<StmtPtr>&& stmts, size_t line) : Stmt(line), stmts_(move(stmts)) {}

void Block::toImStmts(vector<im::StmtPtr>& imStmts) {
  // Keep track of variables declared in this scope
  vector<string> newVars;
  for (StmtPtr& stmt : stmts_) {
    stmt->toImStmts(imStmts);
    // TODO: Maybe remove this and replace with StmtType
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

If::If(ExprPtr&& boolE, std::unique_ptr<Block>&& ifE, StmtPtr&& elseE, size_t line)
    : Stmt(line), boolE_(move(boolE)), ifE_(move(ifE)), elseE_(move(elseE)) {}

void If::toImStmts(vector<im::StmtPtr>& imStmts) {
  unique_ptr<im::MakeLabel> mkIfLabel = make_unique<im::MakeLabel>(newLabel());
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

While::While(ExprPtr&& boolE, std::unique_ptr<Block> body, size_t line)
    : Stmt(line), boolE_(move(boolE)), body_(move(body)) {}

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
CallStmt::CallStmt(const std::string& name, std::vector<ExprPtr>&& params, size_t line)
    : Stmt(line), name_(name), params_(move(params)) {}

void CallStmt::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  imStmts.emplace_back(new im::CallStmt(
      make_unique<im::LabelAddr>(name_), argsToImExprs(name_, params_).first));
}


/**********
 * Return *
 **********/

Return::Return(std::optional<ExprPtr>&& retValue, size_t line) : Stmt(line), retValue_(move(retValue)) {}

void Return::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  const Type& retType = ctx.getReturnTy();
  if (!retValue_.has_value()) {
    // Make sure the function return type is void
    if (retType.type != TypeName::VOID) {
      stringstream err;
      err << "Function has return type " << retType << " but may return void.";
      typeError(err.str());
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
    : Stmt(lhs->getLine()), lhs_(move(lhs)), rhs_(move(rhs)) {}

void Assign::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  if (lhs_->getType() != ExprType::VAR) {
    typeError("Only variables can be assigned");
  }

  const Context::VarInfo& varInfo =
      ctx.lookupVar(static_cast<Var*>(lhs_.get())->getName());
  imStmts.emplace_back(new im::Assign(
      make_unique<im::Temp>(varInfo.temp), rhs_->toImExprAssert(varInfo.type)));
}


/***********
 * VarDecl *
 ***********/

VarDecl::VarDecl(const Type& type, const std::string& name, ExprPtr&& e, size_t line)
    : Stmt(line), type_(type), name_(name), e_(move(e)) {}

void VarDecl::toImStmts(std::vector<im::StmtPtr>& imStmts) {
  // Make sure the right side has the correct type and translate it
  im::ExprPtr rhs = e_->toImExprAssert(type_);
  // Insert the variable into the context
  int temp = ctx.insertVar(name_, move(type_));
  imStmts.emplace_back(new im::Assign(make_unique<im::Temp>(temp), move(rhs)));
}


}  // namespace language
