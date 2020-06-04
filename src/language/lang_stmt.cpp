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

void Block::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  // Keep track of variables declared in this scope and their lines
  vector<pair<string, size_t>> newVars;
  for (StmtPtr& stmt : stmts_) {
    stmt->toImStmts(imStmts, ctx);
    // TODO: Maybe remove this and replace with StmtType
    if (VarDecl* varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
      newVars.emplace_back(varDecl->getName(), varDecl->getLine());
    }
  }

  // When we exit the block remove declared variables from this scope
  ctx.removeVars(newVars);
}


/******
 * If *
 ******/

If::If(ExprPtr&& boolE, unique_ptr<Block>&& ifE, StmtPtr&& elseE, size_t line)
    : Stmt(line), boolE_(move(boolE)), ifE_(move(ifE)), elseE_(move(elseE)) {}

void If::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  unique_ptr<im::MakeLabel> mkIfLabel = make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkElseLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());

  boolE_->asBool(imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr(), ctx);
  imStmts.emplace_back(move(mkElseLabel));
  elseE_->toImStmts(imStmts, ctx);
  imStmts.emplace_back(new im::Jump(mkDoneLabel->genInstr()));
  imStmts.emplace_back(move(mkIfLabel));
  ifE_->toImStmts(imStmts, ctx);
  imStmts.emplace_back(move(mkDoneLabel));
}


/*********
 * While *
 *********/

While::While(ExprPtr&& boolE, unique_ptr<Block> body, size_t line)
    : Stmt(line), boolE_(move(boolE)), body_(move(body)) {}

void While::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  unique_ptr<im::MakeLabel> mkBodyLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());
  assem::Label* bodyLabel = mkBodyLabel->genInstr();
  assem::Label* doneLabel = mkDoneLabel->genInstr();
  // TODO: Not sure if possible (easy) b/c of uniqptrs, but don't compute this
  // twice
  boolE_->asBool(imStmts, bodyLabel, doneLabel, ctx);
  imStmts.emplace_back(move(mkBodyLabel));
  body_->toImStmts(imStmts, ctx);
  boolE_->asBool(imStmts, bodyLabel, doneLabel, ctx);
  imStmts.emplace_back(move(mkDoneLabel));
}


/************
 * CallStmt *
 ************/
CallStmt::CallStmt(
    vector<string>&& qualifiers,
    string_view name,
    vector<ExprPtr>&& params,
    size_t line)
    : Stmt(line),
      qualifiers_(move(qualifiers)),
      name_(name),
      params_(move(params)) {}

void CallStmt::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  auto infoTupleOpt = argsToImExprs(qualifiers_, name_, params_, line_, ctx);
  if (!infoTupleOpt) {
    // Undefined function
    return;
  }
  imStmts.emplace_back(new im::CallStmt(
      make_unique<im::LabelAddr>(get<0>(*infoTupleOpt)),
      move(get<1>(*infoTupleOpt))));
}


/**********
 * Return *
 **********/

Return::Return(optional<ExprPtr>&& retValue, size_t line)
    : Stmt(line), retValue_(move(retValue)) {}

void Return::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  const Type& retType = ctx.getCurrentRetType();
  if (!retValue_.has_value()) {
    // Make sure the function return type is void
    if (retType.typeName != TypeName::VOID) {
      ostringstream& err = ctx.getLogger().logError(line_);
      err << "Function has return type " << retType << " but may return void.";
    }
    imStmts.emplace_back(new im::ReturnStmt(nullptr));
  } else {
    // Make sure the function return type matches and translate the returned
    // expression
    imStmts.emplace_back(
        new im::ReturnStmt((*retValue_)->toImExprAssert(retType, ctx)));
  }
}


/**********
 * Assign *
 **********/

Assign::Assign(ExprPtr&& lhs, ExprPtr&& rhs)
    : Stmt(lhs->getLine()), lhs_(move(lhs)), rhs_(move(rhs)) {}

void Assign::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  switch (lhs_->getType()) {
    case ExprType::VAR: {
      const Ctx::VarInfo* varInfo =
          ctx.lookupVar(static_cast<Var*>(lhs_.get())->getName(), line_);
      if (!varInfo) {
        // Undefined variable
        return;
      }
      imStmts.emplace_back(new im::Assign(
          make_unique<im::Temp>(varInfo->temp),
          rhs_->toImExprAssert(*varInfo->type, ctx)));
      break;
    }
    case ExprType::ARRAY_ACCESS: {
      ExprInfo lhsInfo = lhs_->toImExpr(ctx);
      imStmts.emplace_back(new im::Assign(
          move(lhsInfo.imExpr), rhs_->toImExprAssert(*lhsInfo.type, ctx)));
      break;
    }
    default:
      throw invalid_argument(
          "Assign::toImStmts: Should be enforced by grammar");
  }
}


/***********
 * VarDecl *
 ***********/

VarDecl::VarDecl(TypePtr&& type, string_view name, ExprPtr&& e, size_t line)
    : Stmt(line), type_(move(type)), name_(name), e_(move(e)) {}

void VarDecl::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  // Make sure the right side has the correct type and translate it
  im::ExprPtr rhs = e_->toImExprAssert(*type_, ctx);
  // Insert the variable into the context
  int temp = ctx.insertVar(name_, type_, line_);
  imStmts.emplace_back(new im::Assign(make_unique<im::Temp>(temp), move(rhs)));
}


/*********
 * Print *
 *********/

Print::Print(ExprPtr&& expr, size_t line) : Stmt(line), expr_(move(expr)) {}

void Print::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  // TODO: Separate search path for function defined by language
  vector<ExprPtr> toPrint;
  toPrint.push_back(move(expr_));
  im::ExprPtr callToString =
      CallExpr(
          { "home", "mpresident", "cs", "compiler", "src", "imports", "to_string" },
          "toString",
          move(toPrint),
          line_)
          .toImExpr(ctx)
          .imExpr;

  // Put the char[] in a temporary
  int tCharsAddr = newTemp();
  im::StmtPtr getArr = make_unique<im::Assign>(
      make_unique<im::Temp>(tCharsAddr), move(callToString));

  vector<im::ExprPtr> printArgs;
  // Address of the char[]
  printArgs.push_back(make_unique<im::BinOp>(
      make_unique<im::Temp>(tCharsAddr),
      make_unique<im::Const>(8),
      im::BOp::PLUS));
  // Number of bytes
  printArgs.push_back(
      make_unique<im::MemDeref>(make_unique<im::Temp>(tCharsAddr), 8));

  im::StmtPtr callPrint = make_unique<im::CallStmt>(
      make_unique<im::LabelAddr>("__print"), move(printArgs));

  imStmts.push_back(move(getArr));
  imStmts.push_back(move(callPrint));
}


}  // namespace language
