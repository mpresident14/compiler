#include "src/intermediate/intermediate.hpp"

#include <stdexcept>


using namespace std;

namespace intermediate {


/*********
 * Const *
 *********/

void Const::toInstrs(int temp, std::vector<InstrPtr> &instrs) const {
  instrs.emplace_back(new Operation(
      string("movq $").append(to_string(n_)).append(", <0"), {}, {temp}, {}));
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
  instrs.emplace_back(new Operation(asmCode.append(" >0, <0"), {t1, t2}, {t2}, {}));
  Temp(t2).toInstrs(temp, instrs);
}


}
