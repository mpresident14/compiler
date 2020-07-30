#include "src/language/stmt.hpp"

#include "src/assembly/assembly.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/utils.hpp"

#include <utility>

using namespace std;


namespace language {


/*********
 * Block *
 *********/
Block::Block(vector<StmtPtr>&& stmts, size_t line) : Stmt(line), stmts_(move(stmts)) {}

const StmtPtr* Block::lastStmt() const {
  if (stmts_.empty()) {
    return nullptr;
  }
  const StmtPtr* last = stmts_.back()->lastStmt();
  return last ? last : &stmts_.back();
}


void Block::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  // Keep track of variables declared in this scope and their lines
  vector<pair<string, size_t>> newVars;
  for (StmtPtr& stmt : stmts_) {
    stmt->toImStmts(imStmts, ctx);
    if (VarDecl* varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
      newVars.emplace_back(varDecl->name_, varDecl->line_);
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

const StmtPtr* If::lastStmt() const {
  const StmtPtr* last = elseE_->lastStmt();
  return last ? last : &elseE_;
}

void If::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  unique_ptr<im::MakeLabel> mkIfLabel = make_unique<im::MakeLabel>(lang_utils::newLabel());
  unique_ptr<im::MakeLabel> mkElseLabel = make_unique<im::MakeLabel>(lang_utils::newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel = make_unique<im::MakeLabel>(lang_utils::newLabel());

  boolE_->asBool(imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr(), true, ctx);
  imStmts.emplace_back(move(mkIfLabel));
  ifE_->toImStmts(imStmts, ctx);
  imStmts.emplace_back(new im::Jump(mkDoneLabel->genInstr()));
  imStmts.emplace_back(move(mkElseLabel));
  elseE_->toImStmts(imStmts, ctx);
  imStmts.emplace_back(move(mkDoneLabel));
}


/*********
 * While *
 *********/

While::While(ExprPtr&& boolE, unique_ptr<Block>&& body, size_t line)
    : Stmt(line), boolE_(move(boolE)), body_(move(body)) {}

const StmtPtr* While::lastStmt() const {
  const StmtPtr* last = body_->lastStmt();
  return last ? last : (StmtPtr*)&body_;
}

void While::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  unique_ptr<im::MakeLabel> mkBodyLabel = make_unique<im::MakeLabel>(lang_utils::newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel = make_unique<im::MakeLabel>(lang_utils::newLabel());
  assem::Label* bodyLabel = mkBodyLabel->genInstr();
  assem::Label* doneLabel = mkDoneLabel->genInstr();

  boolE_->clone()->asBool(imStmts, bodyLabel, doneLabel, true, ctx);
  imStmts.emplace_back(move(mkBodyLabel));
  body_->toImStmts(imStmts, ctx);
  boolE_->asBool(imStmts, bodyLabel, doneLabel, false, ctx);
  imStmts.emplace_back(move(mkDoneLabel));
}

/*******
 * For *
 *******/

For::For(
    unique_ptr<VarDecl>&& varDecl,
    ExprPtr&& boolE,
    StmtPtr&& update,
    unique_ptr<Block>&& body,
    size_t line)
    : Stmt(line),
      varDecl_(move(varDecl)),
      boolE_(move(boolE)),
      body_(move(body)),
      update_(move(update)) {}

const StmtPtr* For::lastStmt() const {
  const StmtPtr* last = body_->lastStmt();
  return last ? last : (StmtPtr*)&body_;
}

void For::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  varDecl_->toImStmts(imStmts, ctx);
  While whileStmt(move(boolE_), move(body_), line_);
  whileStmt.body_->stmts_.push_back(move(update_));
  whileStmt.toImStmts(imStmts, ctx);
  ctx.removeVar(varDecl_->name_, line_);
}

/************
 * ExprStmt *
 ************/
ExprStmt::ExprStmt(ExprPtr&& expr) : Stmt(expr->line_), expr_(move(expr)) {}

void ExprStmt::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  // The grammar determines what Exprs can also be Stmts (see StmtExpr in lang.pgen)
  imStmts.emplace_back(new im::ExprStmt(expr_->toImExpr(ctx).imExpr));
}


/**********
 * Return *
 **********/

Return::Return(optional<ExprPtr>&& retValue, size_t line) : Stmt(line), retValue_(move(retValue)) {}

void Return::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  const Type& retType = ctx.getCurrentRetType();
  if (!retValue_.has_value()) {
    // Make sure the function return type is void
    if (retType.typeName != Type::Category::VOID) {
      ostream& err = ctx.getLogger().logError(line_);
      err << "Function has return type " << retType << " but may return void.";
    }
    imStmts.push_back(make_unique<im::ReturnStmt>(nullptr));
  } else {
    // Make sure the function return type matches and translate the returned
    // expression
    imStmts.push_back(make_unique<im::ReturnStmt>((*retValue_)->toImExprAssert(retType, ctx)));
  }
}


/**********
 * Assign *
 **********/

Assign::Assign(ExprPtr&& lValue, ExprPtr&& rhs)
    : Stmt(lValue->line_), lValue_(move(lValue)), rhs_(move(rhs)) {}

void Assign::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  Expr::Info lValueInfo = lValue_->toImExpr(ctx);
  if (lValueInfo.type->isFinal) {
    ctx.getLogger().logError(line_, "Cannot assign to a final variable");
  }

  im::Expr::Category category = lValueInfo.imExpr->getCategory();
  if (category == im::Expr::Category::TEMP || category == im::Expr::Category::MEM_DEREF) {
    imStmts.emplace_back(
        new im::Assign(move(lValueInfo.imExpr), rhs_->toImExprAssert(*lValueInfo.type, ctx)));
  } else {
    ctx.getLogger().logError(line_, "Left side of assignment must be an lvalue.");
  }
}

/**********
 * Update *
 **********/

unique_ptr<im::MemDeref> Update::derefTemp(int temp, u_char numBytes) {
  return make_unique<im::MemDeref>(0, make_unique<im::Temp>(temp), nullptr, numBytes);
}

unique_ptr<im::Assign> Update::assignAddr(int temp, im::MemDeref* memDeref) {
  return make_unique<im::Assign>(
      make_unique<im::Temp>(temp),
      make_unique<im::Leaq>(
          memDeref->offset_,
          move(memDeref->baseAddr_),
          move(memDeref->mult_),
          memDeref->numBytes_));
}

Update::Update(ExprPtr&& lValue, Expr::BOp bOp, ExprPtr&& rhs)
    : Stmt(lValue->line_), lValue_(move(lValue)), rhs_(move(rhs)), bOp_(bOp) {}


void Update::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  Expr::Info eInfo = lValue_->toImExpr(ctx);
  size_t lValueLine = lValue_->line_;
  im::Expr::Category category = eInfo.imExpr->getCategory();

  if (category == im::Expr::Category::TEMP) {
    im::Temp* imTemp = static_cast<im::Temp*>(eInfo.imExpr.get());
    Assign(
        make_unique<ImWrapper>(make_unique<im::Temp>(imTemp->t_), eInfo.type, lValueLine),
        make_unique<BinaryOp>(
            make_unique<ImWrapper>(move(eInfo.imExpr), eInfo.type, lValueLine), move(rhs_), bOp_))
        .toImStmts(imStmts, ctx);
  } else if (category == im::Expr::Category::MEM_DEREF) {
    // For memory dereferences, we have to ensure that we don't calculate them
    // twice because then any side effects will be duplicated. Therefore, we
    // calculate the address and save that to a temp before performing the
    // update.
    im::MemDeref* memDeref = static_cast<im::MemDeref*>(eInfo.imExpr.get());
    int t = newTemp();
    imStmts.push_back(assignAddr(t, memDeref));

    ExprPtr memWrapper1 =
        make_unique<ImWrapper>(derefTemp(t, memDeref->numBytes_), eInfo.type, lValueLine);
    ExprPtr memWrapper2 =
        make_unique<ImWrapper>(derefTemp(t, memDeref->numBytes_), eInfo.type, lValueLine);
    Assign(move(memWrapper1), make_unique<BinaryOp>(move(memWrapper2), move(rhs_), bOp_))
        .toImStmts(imStmts, ctx);
  } else {
    // All lvalues translate into a Temp or a MemDeref
    ctx.getLogger().logError(line_, "Left side of an update operator must be an lvalue.");
    return;
  }
}

/***********
 * VarDecl *
 ***********/

VarDecl::VarDecl(TypePtr&& type, string_view name, ExprPtr&& e, size_t line)
    : Stmt(line), type_(move(type)), name_(name), e_(move(e)) {}

void VarDecl::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  ctx.checkType(*type_, line_);
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
  size_t eLine = expr_->line_;
  Expr::Info eInfo = expr_->toImExpr(ctx);
  auto imWrapExpr = make_unique<ImWrapper>(move(eInfo.imExpr), eInfo.type, eLine);

  // Put the String in a temporary
  static const string strVar = "_str";
  const char toString[] = "toString";

  ExprPtr callToString;
  if (eInfo.type->typeName == Type::Category::CLASS) {
    callToString =
        make_unique<MethodInvocation>(move(imWrapExpr), toString, vector<ExprPtr>(), line_);
  } else {
    vector<ExprPtr> toPrint;
    toPrint.push_back(move(imWrapExpr));
    callToString = make_unique<Call>(vector<string>(), toString, move(toPrint), line_);
  }

  VarDecl(TypePtr(Type::STRING_TYPE), strVar, move(callToString), line_).toImStmts(imStmts, ctx);

  vector<im::ExprPtr> printArgs;
  // Address of the char[] + 8 to skip length field
  printArgs.push_back(make_unique<im::BinOp>(
      MemberAccess(make_unique<Var>(strVar, line_), "arr").toImExpr(ctx).imExpr,
      make_unique<im::Const>(8),
      im::BOp::PLUS));

  // Number of bytes
  printArgs.push_back(MemberAccess(make_unique<Var>(strVar, line_), "len").toImExpr(ctx).imExpr);

  im::StmtPtr callPrint = make_unique<im::ExprStmt>(
      make_unique<im::Call>(make_unique<im::LabelAddr>("__println"), move(printArgs), false));

  imStmts.push_back(move(callPrint));

  ctx.removeVar(strVar, line_);
}


/********
 * Stmt *
 ********/

const StmtPtr* Stmt::lastStmt() const { return nullptr; }


}  // namespace language
