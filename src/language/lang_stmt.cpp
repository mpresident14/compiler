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
  unique_ptr<im::MakeLabel> mkIfLabel = make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkElseLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());

  boolE_->asBool(
      imStmts, mkIfLabel->genInstr(), mkElseLabel->genInstr(), true, ctx);
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

While::While(ExprPtr&& boolE, unique_ptr<Block> body, size_t line)
    : Stmt(line), boolE_(move(boolE)), body_(move(body)) {}

const StmtPtr* While::lastStmt() const {
  const StmtPtr* last = body_->lastStmt();
  return last ? last : (StmtPtr*)&body_;
}

void While::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  unique_ptr<im::MakeLabel> mkBodyLabel =
      make_unique<im::MakeLabel>(newLabel());
  unique_ptr<im::MakeLabel> mkDoneLabel =
      make_unique<im::MakeLabel>(newLabel());
  assem::Label* bodyLabel = mkBodyLabel->genInstr();
  assem::Label* doneLabel = mkDoneLabel->genInstr();

  boolE_->clone()->asBool(imStmts, bodyLabel, doneLabel, true, ctx);
  imStmts.emplace_back(move(mkBodyLabel));
  body_->toImStmts(imStmts, ctx);
  boolE_->asBool(imStmts, bodyLabel, doneLabel, false, ctx);
  imStmts.emplace_back(move(mkDoneLabel));
}


/************
 * ExprStmt *
 ************/
ExprStmt::ExprStmt(ExprPtr expr, size_t line) : Stmt(line), expr_(move(expr)) {}

void ExprStmt::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  ExprType exprType = expr_->getType();
  if (!(exprType == ExprType::CALL_EXPR || exprType == ExprType::INC_DEC)) {
    ctx.getLogger().logWarning(line_, "Unused expression.");
    return;
  }

  imStmts.emplace_back(new im::ExprStmt(expr_->toImExpr(ctx).imExpr));
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
    imStmts.push_back(make_unique<im::ReturnStmt>(nullptr));
  } else {
    // Make sure the function return type matches and translate the returned
    // expression
    imStmts.push_back(make_unique<im::ReturnStmt>(
        (*retValue_)->toImExprAssert(retType, ctx)));
  }
}


/**********
 * Assign *
 **********/

Assign::Assign(ExprPtr&& lhs, ExprPtr&& rhs)
    : Stmt(lhs->line_), lhs_(move(lhs)), rhs_(move(rhs)) {}

void Assign::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  if (lhs_->getType() == ExprType::MEMBER_ACCESS &&
      static_cast<MemberAccess*>(lhs_.get())
              ->objExpr_->toImExpr(ctx)
              .type->typeName == TypeName::ARRAY) {
    ctx.getLogger().logError(
        line_, "Cannot assign to length field of an array.");
    return;
  }

  ExprInfo lhsInfo = lhs_->toImExpr(ctx);
  imStmts.emplace_back(new im::Assign(
      move(lhsInfo.imExpr), rhs_->toImExprAssert(*lhsInfo.type, ctx)));
}

/**********
 * Update *
 **********/

std::unique_ptr<im::MemDeref> Update::derefTemp(int temp, u_char numBytes) {
  return make_unique<im::MemDeref>(
      0, make_unique<im::Temp>(temp), nullptr, numBytes);
}

std::unique_ptr<im::Assign> Update::assignAddr(int temp, im::MemDeref* memDeref) {
  return make_unique<im::Assign>(
      make_unique<im::Temp>(temp),
      make_unique<im::Leaq>(
          memDeref->offset_,
          move(memDeref->baseAddr_),
          move(memDeref->mult_),
          memDeref->numBytes_));
}

Update::Update(ExprPtr&& lhs, BOp bOp, ExprPtr&& rhs)
    : Stmt(lhs->line_), lhs_(move(lhs)), rhs_(move(rhs)), bOp_(bOp) {}

void Update::toImStmts(vector<im::StmtPtr>& imStmts, Ctx& ctx) {
  if (lhs_->getType() == ExprType::VAR) {
    ExprPtr lhsClone = lhs_->clone();
    Assign(move(lhsClone), make_unique<BinaryOp>(move(lhs_), move(rhs_), bOp_))
        .toImStmts(imStmts, ctx);
  } else {
    // For memory dereferences, we have to ensure that we don't calculate them
    // twice because then any side effects will be duplicated. Therefore, we
    // calculate the address and save that to a temp before performing the
    // update.
    size_t lhsLine = lhs_->line_;
    ExprInfo eInfo = lhs_->toImExpr(ctx);
    im::MemDeref* memDeref = static_cast<im::MemDeref*>(eInfo.imExpr.get());

    int t = newTemp();
    imStmts.push_back(assignAddr(t, memDeref));

    ExprPtr memWrapper1 = make_unique<ImWrapper>(
        derefTemp(t, memDeref->numBytes_), eInfo.type, lhsLine);
    ExprPtr memWrapper2 = make_unique<ImWrapper>(
        derefTemp(t, memDeref->numBytes_), eInfo.type, lhsLine);

    Assign(
        move(memWrapper1),
        make_unique<BinaryOp>(move(memWrapper2), move(rhs_), bOp_))
        .toImStmts(imStmts, ctx);
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
  vector<ExprPtr> toPrint;
  toPrint.push_back(move(expr_));
  vector<string> toStringPath = Program::importPathParts;
  toStringPath.push_back("to_string");
  im::ExprPtr callToString =
      CallExpr(move(toStringPath), "toString", move(toPrint), line_)
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
  printArgs.push_back(make_unique<im::MemDeref>(
      0, make_unique<im::Temp>(tCharsAddr), nullptr, 8));

  im::StmtPtr callPrint = make_unique<im::ExprStmt>(make_unique<im::CallExpr>(
      make_unique<im::LabelAddr>("__println"), move(printArgs), false));

  imStmts.push_back(move(getArr));
  imStmts.push_back(move(callPrint));
}


/********
 * Stmt *
 ********/

const StmtPtr* Stmt::lastStmt() const { return nullptr; }


}  // namespace language
