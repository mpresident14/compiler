#include "src/intermediate/intermediate.hpp"

#include <stdexcept>


using namespace std;

namespace intermediate {


/*********
 * Const *
 *********/

void Const::toInstrs(int temp, std::vector<InstrPtr> &instrs) const {
  instrs.emplace_back(new Operation(
      string("movq $").append(to_string(n_)).append(", `D0"), {}, {temp}));
}


/*********
 * Temp *
 *********/

void Temp::toInstrs(int temp, std::vector<InstrPtr> &instrs) const {
  instrs.emplace_back(new Move(t_, temp));
}


/*********
 * BinOp *
 *********/

BinOp::BinOp(ExprPtr&& expr1, ExprPtr&& expr2, Bop bop)
  : expr1_(move(expr1)), expr2_(move(expr2)), bop_(bop) {}

void BinOp::toInstrs(int temp, std::vector<InstrPtr>& instrs) const {
  switch (bop_) {
    case Bop::LSHIFT: return handleShifts("shlq", temp, instrs);
    case Bop::RSHIFT: return handleShifts("shrq", temp, instrs);
    case Bop::ARSHIFT: return handleShifts("sarq", temp, instrs);
    case Bop::DIV: return handleDiv(true, temp, instrs);
    case Bop::MOD: return handleDiv(false, temp, instrs);
    case Bop::PLUS: return handleOthers("addq", temp, instrs);
    case Bop::MINUS: return handleOthers("subq", temp, instrs);
    case Bop::MUL: return handleOthers("imulq", temp, instrs);
    case Bop::AND: return handleOthers("andq", temp, instrs);
    case Bop::OR: return handleOthers("orq", temp, instrs);
    case Bop::XOR: return handleOthers("xorq", temp, instrs);
    default: throw invalid_argument("Unrecognized binary operator.");
  }
}


void BinOp::handleShifts(string asmCode, int temp, std::vector<InstrPtr> &instrs) const {
  expr1_->toInstrs(temp, instrs);

  // If shift number (expr2_) is not an immediate, its value must be in %cl
  if (expr2_->getType() == ExprType::CONST) {
    asmCode.append(" $");
    asmCode.append(to_string(static_cast<Const*>(expr2_.get())->getInt()));
    asmCode.append(", `D0");
    instrs.emplace_back(new Operation(asmCode, {temp}, {temp}));
  } else {
    expr2_->toInstrs(RCX, instrs);
    asmCode.append(" %cl, `D0");
    instrs.emplace_back(new Operation(asmCode, {RCX, temp}, {temp}));
  }
}

void BinOp::handleDiv(bool isDiv, int temp, std::vector<InstrPtr> &instrs) const {
  expr1_->toInstrs(RAX, instrs);
  int t2 = newTemp();
  expr2_->toInstrs(t2, instrs);
  // Sign-extend %rax into %rdx
  instrs.emplace_back(new Operation("cqto", {RAX}, {RDX}));
  // Divide %rax%rdx by t2, quotient in %rax, remainder in %rdx
  instrs.emplace_back(new Operation("idivq `S0", {t2, RAX, RDX}, {RAX, RDX}));
  if (isDiv) {
    // If division, move %rax into temp
    Temp(RAX).toInstrs(temp, instrs);
  } else {
    // If mod, move %rdx into temp
    Temp(RDX).toInstrs(temp, instrs);
  }
}


void BinOp::handleOthers(std::string asmCode, int temp, std::vector<InstrPtr> &instrs) const {
  // TODO: Specialize if expr1_ is a Const
  // TODO: Leaq optimization
  // TODO: Inc/deq optimization
  int t1 = newTemp();
  expr1_->toInstrs(t1, instrs);
  int t2 = newTemp();
  expr2_->toInstrs(t2, instrs);
  instrs.emplace_back(new Operation(asmCode.append(" `S0, `D0"), {t1, t2}, {t2}));
  Temp(t2).toInstrs(temp, instrs);
}


/************
 * MemDeref *
 ************/

MemDeref::MemDeref(ExprPtr&& addr) : addr_(move(addr)) {}

void MemDeref::toInstrs(int temp, std::vector<InstrPtr> &instrs) const {
  int t = newTemp();
  addr_->toInstrs(t, instrs);
  instrs.emplace_back(new Operation("movq (`S0), `D0", {t}, {temp}));
}


/*************
 * LabelAddr *
 *************/

LabelAddr::LabelAddr(const string& name) : name_(name) {}

void LabelAddr::toInstrs(int temp, std::vector<InstrPtr> &instrs) const {
  // "leaq symbol(%rip), dst" looks like symbol + %rip, but actually means
  // symbol with respect to %rip. Essentially, it calculates the address of symbol
  instrs.emplace_back(new Operation(
      string("leaq ").append(name_).append("(%rip), `D0"), {}, {temp}));
}



/********
 * Call *
 ********/
Call::Call(ExprPtr&& addr, std::vector<ExprPtr>&& params, bool hasReturnValue)
    : addr_(move(addr)), params_(move(params)), hasReturnValue_(hasReturnValue) {}


void Call::toInstrs(int temp, std::vector<InstrPtr> &instrs) const {
  // Move params into argument registers
  // TODO: If more than six params, need to put onto stack
  vector<int> srcTemps;
  size_t numParams = params_.size();
  if (numParams > 6) {
    throw runtime_error("More than 6 args not implemented yet");
  }
  for (size_t i = 0; i < numParams; ++i) {
    int argReg = ARG_REGS[i];
    params_[i]->toInstrs(argReg, instrs);
    srcTemps.push_back(argReg);
  }

  if (addr_->getType() == ExprType::LABEL_ADDR) {
    // If we are calling a function name, just call it directly
    instrs.emplace_back(
        new JumpOp("callq " + static_cast<LabelAddr*>(addr_.get())->getName(), move(srcTemps), vector<int>(CALLEE_SAVE_REGS), {}));
  } else {
    // If we are calling an address, we need to put it in a register
    int t = newTemp();
    addr_->toInstrs(t, instrs);
    srcTemps.push_back(t);
    instrs.emplace_back(new JumpOp("callq *`S0", move(srcTemps), vector<int>(CALLEE_SAVE_REGS), {}));
  }

  // Move result from %rax to temp if needed
  if (hasReturnValue_) {
    instrs.emplace_back(new Move(RAX, temp));
  }
}

}


