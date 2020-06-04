#include "src/intermediate/intermediate.hpp"

#include <stdexcept>


using namespace std;

namespace im {


/*********
 * Const *
 *********/

void Const::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  instrs.emplace_back(new assem::Operation(
      string("movq $").append(to_string(n_)).append(", `8D0"), {}, { temp }));
}


/*********
 * Temp *
 *********/

void Temp::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  instrs.emplace_back(new assem::Move(t_, temp));
}

int Temp::toAssemInstrs(vector<assem::InstrPtr>&) const { return t_; }


/*********
 * BinOp *
 *********/

BinOp::BinOp(ExprPtr&& expr1, ExprPtr&& expr2, BOp bop)
    : expr1_(move(expr1)), expr2_(move(expr2)), bop_(bop) {}

void BinOp::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  switch (bop_) {
    case BOp::LSHIFT:
      return handleShifts("shlq", temp, instrs);
    case BOp::RSHIFT:
      return handleShifts("shrq", temp, instrs);
    case BOp::ARSHIFT:
      return handleShifts("sarq", temp, instrs);
    case BOp::DIV:
      return handleDiv(true, temp, instrs);
    case BOp::MOD:
      return handleDiv(false, temp, instrs);
    case BOp::PLUS:
      return handleOthers("addq", temp, instrs);
    case BOp::MINUS:
      return handleOthers("subq", temp, instrs);
    case BOp::MUL:
      return handleOthers("imulq", temp, instrs);
    case BOp::AND:
      return handleOthers("andq", temp, instrs);
    case BOp::OR:
      return handleOthers("orq", temp, instrs);
    case BOp::XOR:
      return handleOthers("xorq", temp, instrs);
    default:
      throw invalid_argument("Unrecognized binary operator.");
  }
}


void BinOp::handleShifts(
    string asmCode,
    int temp,
    vector<assem::InstrPtr>& instrs) const {
  expr1_->toAssemInstrs(temp, instrs);

  // If shift number (expr2_) is not an immediate, its value must be in %cl
  if (expr2_->getType() == ExprType::CONST) {
    asmCode.append(" $");
    asmCode.append(to_string(static_cast<Const*>(expr2_.get())->getInt()));
    asmCode.append(", `8D0");
    instrs.emplace_back(new assem::Operation(asmCode, { temp }, { temp }));
  } else {
    expr2_->toAssemInstrs(RCX, instrs);
    asmCode.append(" %cl, `8D0");
    instrs.emplace_back(new assem::Operation(asmCode, { RCX, temp }, { temp }));
  }
}

void BinOp::handleDiv(bool isDiv, int temp, vector<assem::InstrPtr>& instrs)
    const {
  expr1_->toAssemInstrs(RAX, instrs);
  int t2 = expr2_->toAssemInstrs(instrs);
  // Sign-extend %rax into %rdx
  instrs.emplace_back(new assem::Operation("cqto", { RAX }, { RDX }));
  // Divide %rax%rdx by t2, quotient in %rax, remainder in %rdx
  instrs.emplace_back(
      new assem::Operation("idivq `8S0", { t2, RAX, RDX }, { RAX, RDX }));
  if (isDiv) {
    // If division, move %rax into temp
    Temp(RAX).toAssemInstrs(temp, instrs);
  } else {
    // If mod, move %rdx into temp
    Temp(RDX).toAssemInstrs(temp, instrs);
  }
}


void BinOp::handleOthers(
    string asmCode,
    int temp,
    vector<assem::InstrPtr>& instrs) const {
  // TODO: Specialize if expr1_ is a Const
  // TODO: Leaq optimization
  // TODO: Inc/deq optimization
  int t1 = expr1_->toAssemInstrs(instrs);
  int t2 = expr2_->toAssemInstrs(instrs);
  // Need to create a new temp first in case t1 or t2 is the same as temp
  // TODO: Handle these cases separately so we don't waste instructions most of
  // the time
  int tRes = newTemp();
  instrs.emplace_back(new assem::Move(t1, tRes));
  instrs.emplace_back(new assem::Operation(
      asmCode.append(" `8S1, `8D0"), { tRes, t2 }, { tRes }));
  instrs.emplace_back(new assem::Move(tRes, temp));
}


/************
 * MemDeref *
 ************/

namespace {
  void addInstrLetter(char bytesChar, string& str) {
    switch (bytesChar) {
      case '8':
        return;
      case '4':
        str.append("sl");
        return;
      case '2':
        str.append("sw");
        return;
      case '1':
        str.append("sb");
        return;
      default:
        throw invalid_argument("addInstrLetter: " + to_string(bytesChar));
    }
  }
}  // namespace

MemDeref::MemDeref(ExprPtr&& addr, u_char numBytes)
    : addr_(move(addr)), numBytes_(numBytes) {}

void MemDeref::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  int t = addr_->toAssemInstrs(instrs);
  string asmOp = "mov";

  char bytesChar = numBytes_ + '0';
  addInstrLetter(bytesChar, asmOp);
  asmOp.append("q (`8S0), `8D0");
  instrs.emplace_back(new assem::Operation(move(asmOp), { t }, { temp }));
}

/**************
 * DoThenEval *
 **************/

DoThenEval::DoThenEval(vector<StmtPtr>&& stmts, ExprPtr expr)
    : stmts_(move(stmts)), expr_(move(expr)) {}

void DoThenEval::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs)
    const {
  for (const StmtPtr& stmt : stmts_) {
    stmt->toAssemInstrs(instrs);
  }
  expr_->toAssemInstrs(temp, instrs);
}

/*************
 * LabelAddr *
 *************/

LabelAddr::LabelAddr(const string& name) : name_(name) {}

void LabelAddr::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  // "leaq symbol(%rip), dst" looks like symbol + %rip, but actually means
  // symbol with respect to %rip. Essentially, it calculates the address of
  // symbol
  instrs.emplace_back(new assem::Operation(
      string("leaq ").append(name_).append("(%rip), `8D0"), {}, { temp }));
}


/************
 * CallExpr *
 ************/
CallExpr::CallExpr(
    ExprPtr&& addr,
    vector<ExprPtr>&& params,
    bool hasReturnValue)
    : addr_(move(addr)),
      params_(move(params)),
      hasReturnValue_(hasReturnValue) {}


void CallExpr::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  // Move params into argument registers
  // TODO: If more than six params, need to put onto stack
  vector<int> srcTemps;
  size_t numParams = params_.size();
  if (numParams > 6) {
    throw runtime_error("TODO: More than 6 args not implemented yet");
  }
  for (size_t i = 0; i < numParams; ++i) {
    int argReg = ARG_REGS[i];
    params_[i]->toAssemInstrs(argReg, instrs);
    srcTemps.push_back(argReg);
  }

  if (addr_->getType() == ExprType::LABEL_ADDR) {
    // If we are calling a function name, just call it directly
    instrs.emplace_back(new assem::Operation(
        "callq " + static_cast<LabelAddr*>(addr_.get())->getName(),
        move(srcTemps),
        regsAsInts(CALLER_SAVE_REGS)));
  } else {
    // If we are calling an address, we need to put it in a register
    int t = addr_->toAssemInstrs(instrs);
    srcTemps.push_back(t);
    instrs.emplace_back(new assem::Operation(
        "callq *`8S0", move(srcTemps), regsAsInts(CALLER_SAVE_REGS)));
  }

  // Move result from %rax to temp if needed
  if (hasReturnValue_) {
    instrs.emplace_back(new assem::Move(RAX, temp));
  }
}


/********
 * Expr *
 ********/
int Expr::toAssemInstrs(vector<assem::InstrPtr>& instrs) const {
  int temp = newTemp();
  toAssemInstrs(temp, instrs);
  return temp;
}

}  // namespace im
