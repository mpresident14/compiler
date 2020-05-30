#include "src/assembly/assembly.hpp"
#include "src/language/language.hpp"

#include <sstream>
#include <utility>

using namespace std;


namespace language {


/*********
 * Block *
 *********/
Block::Block(vector<StmtPtr>&& stmts, size_t line)
    : Stmt(line), stmts_(move(stmts)) {}

void Block::toImStmts(vector<im::StmtPtr>& imStmts) {
  // Keep track of variables declared in this scope and their lines
  vector<pair<string, size_t>> newVars;
  for (StmtPtr& stmt : stmts_) {
    stmt->toImStmts(imStmts);
    // TODO: Maybe remove this and replace with StmtType
    if (VarDecl* varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
      newVars.emplace_back(varDecl->getName(), varDecl->getLine());
    }
  }

  // When we exit the block remove declared variables from this scope
  ctx::removeVars(newVars);
}


/******
 * If *
 ******/

If::If(
    ExprPtr&& boolE,
    unique_ptr<Block>&& ifE,
    StmtPtr&& elseE,
    size_t line)
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

While::While(ExprPtr&& boolE, unique_ptr<Block> body, size_t line)
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
CallStmt::CallStmt(
    string_view name,
    vector<ExprPtr>&& params,
    size_t line)
    : Stmt(line), name_(name), params_(move(params)) {}

void CallStmt::toImStmts(vector<im::StmtPtr>& imStmts) {
  imStmts.emplace_back(new im::CallStmt(
      make_unique<im::LabelAddr>(name_),
      argsToImExprs(name_, params_, line_).first));
}


/**********
 * Return *
 **********/

Return::Return(optional<ExprPtr>&& retValue, size_t line)
    : Stmt(line), retValue_(move(retValue)) {}

void Return::toImStmts(vector<im::StmtPtr>& imStmts) {
  const TypePtr& retType = ctx::lookupFn(ctx::getCurrentFn(), line_).returnType;
  if (!retValue_.has_value()) {
    // Make sure the function return type is void
    if (retType->typeName != TypeName::VOID) {
      ostringstream& err = ctx::getLogger().logError(line_);
      err << "Function has return type " << *retType << " but may return void.";
    }
    imStmts.emplace_back(new im::ReturnStmt(nullptr));
  } else {
    // Make sure the function return type matches and translate the returned
    // expression
    imStmts.emplace_back(
        new im::ReturnStmt((*retValue_)->toImExprAssert(*retType)));
  }
}


/**********
 * Assign *
 **********/

Assign::Assign(ExprPtr&& lhs, ExprPtr&& rhs)
    : Stmt(lhs->getLine()), lhs_(move(lhs)), rhs_(move(rhs)) {}

void Assign::toImStmts(vector<im::StmtPtr>& imStmts) {
  switch (lhs_->getType()) {
    case ExprType::VAR: {
      const ctx::VarInfo& varInfo =
          ctx::lookupVar(static_cast<Var*>(lhs_.get())->getName(), line_);
      imStmts.emplace_back(new im::Assign(
          make_unique<im::Temp>(varInfo.temp),
          rhs_->toImExprAssert(*varInfo.type)));
      break;
    }
    case ExprType::ARRAY_ACCESS: {
      ExprInfo lhsInfo = lhs_->toImExpr();
      imStmts.emplace_back(new im::Assign(
          move(lhsInfo.imExpr),
          rhs_->toImExprAssert(*lhsInfo.type)));
      break;
    }
    default:
      ctx::getLogger().logError(line_, "Only lvalues can be assigned");
  }
}


/***********
 * VarDecl *
 ***********/

VarDecl::VarDecl(
    TypePtr&& type,
    string_view name,
    ExprPtr&& e,
    size_t line)
    : Stmt(line), type_(move(type)), name_(name), e_(move(e)) {}

void VarDecl::toImStmts(vector<im::StmtPtr>& imStmts) {
  // Make sure the right side has the correct type and translate it
  im::ExprPtr rhs = e_->toImExprAssert(*type_);
  // Insert the variable into the context
  int temp = ctx::insertVar(name_, type_, line_);
  imStmts.emplace_back(new im::Assign(make_unique<im::Temp>(temp), move(rhs)));
}


}  // namespace language
