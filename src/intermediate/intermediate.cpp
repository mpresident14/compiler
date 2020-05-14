#include "src/intermediate/intermediate.hpp"

using namespace std;

namespace intermediate {


BinOp::BinOp(ExprPtr&& expr1, ExprPtr&& expr2, Bop bop)
  : expr1_(move(expr1)), expr2_(move(expr2)), bop_(bop) {}

void BinOp::toInstrs(int temp, std::vector<InstrPtr>& instrs) const {
  switch (bop_) {
    case Bop::LSHIFT: return handleShifts("shlq", temp, instrs);
    case Bop::RSHIFT: return handleShifts("shrq", temp, instrs);
    case Bop::ARSHIFT: return handleShifts("sarq", temp, instrs);
    case Bop::DIV: return handleDiv(true, temp, instrs);
    case Bop::MOD: return handleDiv(false, temp, instrs);
  }
}


void BinOp::handleShifts(string asmCode, int temp, std::vector<InstrPtr> &instrs) const {
  expr1_->toInstrs(temp, instrs);

  // If shift number (expr2_) is not an immediate, its value must be in %cl
  if (expr2_->getType() == ExprType::CONST) {
    asmCode.append(" $");
    asmCode.append(to_string(static_cast<Const*>(expr2_.get())->getInt()));
    asmCode.append(", <0");
    instrs.emplace_back(new Operation(asmCode, {temp}, {temp}, {}));
  } else {
    expr2_->toInstrs(RCX, instrs);
    asmCode.append(" %cl, <0");
    instrs.emplace_back(new Operation(asmCode, {RCX, temp}, {temp}, {}));
  }
}

void BinOp::handleDiv(bool isDiv, int temp, std::vector<InstrPtr> &instrs) const {
  expr1_->toInstrs(RAX, instrs);
  int t2 = newTemp();
  expr2_->toInstrs(t2, instrs);
  // Sign-extend %rax into %rdx
  instrs.emplace_back(new Operation("cqto", {RAX}, {RDX}, {}));
  // Divide %rax%rdx by t2, quotient in %rax, remainder in %rdx
  instrs.emplace_back(new Operation("idivq >0", {t2, RAX, RDX}, {RAX, RDX}, {}));
  if (isDiv) {
    // If division, move %rax into temp
    Temp(RAX).toInstrs(t2, instrs);
  } else {
    // If mod, move %rdx into temp
    Temp(RDX).toInstrs(t2, instrs);
  }
}


}
