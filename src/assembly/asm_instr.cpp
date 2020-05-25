#include "src/assembly/assembly.hpp"

#include <stdexcept>

#include <prez/print_stuff.hpp>

using namespace std;

namespace assem {

// TODO: Turn at()s into []s when all testing is complete.

/****************
 * constructors *
 ****************/
Label::Label(const string& name) : name_(name) {}

Move::Move(int src, int dst) : src_(src), dst_(dst) {}

Operation::Operation(
    const string& asmCode,
    vector<int>&& srcs,
    vector<int>&& dsts)
    : asmCode_(asmCode), srcs_(move(srcs)), dsts_(move(dsts)) {}

JumpOp::JumpOp(
    const string& asmCode,
    vector<int>&& srcs,
    vector<int>&& dsts,
    vector<Instruction*>&& jumps)
    : Operation(asmCode, move(srcs), move(dsts)), jumps_(move(jumps)) {}


namespace {

  constexpr MachineReg SPILL_REGS[]{ R10, R11 };
  constexpr int digitToInt(char c) noexcept { return c - '0'; }

}  // namespace

/*********
 * spill *
 *********/

bool Label::spillTemps(vector<InstrPtr>&) {
  // No variables here, so we can insert this instruction
  return true;
}

bool Move::spillTemps(vector<InstrPtr>& newInstrs) {
  // No variables to spill, so insert the updated instruction
  if (isRegister(src_) && isRegister(dst_)) {
    return true;
  }

  // Src variable from stack into register
  if (!isRegister(src_)) {
    newInstrs.push_back(
        make_unique<Move>(src_, isRegister(dst_) > 0 ? dst_ : SPILL_REGS[0]));
  }

  if (!isRegister(dst_)) {
    newInstrs.push_back(
        make_unique<Move>(isRegister(src_) > 0 ? src_ : SPILL_REGS[0], dst_));
  }

  return false;
}

bool Operation::spillTemps(vector<InstrPtr>& newInstrs) {
  // For each temporary src, if it is a var, then update it to a spill register
  // and add an instruction to move it from the stack to the spill register
  size_t numSpilled = 0;
  size_t numSrcs = srcs_.size();
  for (size_t i = 0; i < numSrcs; ++i) {
    int src = srcs_.at(i);
    if (!isRegister(src)) {
      if (numSpilled == 2) {
        throw runtime_error("More than two variables in this instruction.");
      }
      int spillReg = SPILL_REGS[numSpilled++];
      srcs_.at(i) = spillReg;
      newInstrs.push_back(make_unique<Move>(src, spillReg));
    }
  }

  // Insert the updated instruction
  return true;
}

bool Return::spillTemps(vector<InstrPtr>&) {
  // No variables here, so we can insert this instruction
  return true;
}

/**************
 * assignRegs *
 **************/

void assignReg(int& temp, const unordered_map<int, MachineReg>& coloring) {
  auto iter = coloring.find(temp);
  if (iter != coloring.end()) {
    temp = iter->second;
  }
}

void Label::assignRegs(const unordered_map<int, MachineReg>&) { return; }

void Move::assignRegs(const unordered_map<int, MachineReg>& coloring) {
  assignReg(src_, coloring);
  assignReg(dst_, coloring);
}

void Operation::assignRegs(const unordered_map<int, MachineReg>& coloring) {
  for (int& src : srcs_) {
    assignReg(src, coloring);
  }
  for (int& dst : dsts_) {
    assignReg(dst, coloring);
  }
}

void Return::assignRegs(const unordered_map<int, MachineReg>&) { return; }

/**********
 * toCode *
 **********/

void tempToCode(
    ostream& out,
    int temp,
    const unordered_map<int, size_t>& varToStackOffset) {
  if (isRegister(temp)) {
    out << static_cast<MachineReg>(temp);
  } else {
    out << varToStackOffset.at(temp) << "(%rsp)";
  }
}

void Label::toCode(ostream& out, const unordered_map<int, size_t>&) const {
  out << name_ << ":\n";
}

void Move::toCode(
    ostream& out,
    const unordered_map<int, size_t>& varToStackOffset) const {
  if (src_ == dst_) {
    return;
  }

  out << "\tmovq ";
  tempToCode(out, src_, varToStackOffset);
  out << ", ";
  tempToCode(out, dst_, varToStackOffset);
  out << '\n';
}

void Operation::toCode(
    ostream& out,
    const unordered_map<int, size_t>& varToStackOffset) const {
  out << '\t';
  size_t len = asmCode_.size();
  size_t i = 0;
  while (i < len) {
    char c = asmCode_.at(i);
    if (c == '`') {
      c = asmCode_.at(i + 1);
      if (c == 'S') {
        tempToCode(
            out, srcs_.at(digitToInt(asmCode_.at(i + 2))), varToStackOffset);
      } else if (c == 'D') {
        tempToCode(
            out, dsts_.at(digitToInt(asmCode_.at(i + 2))), varToStackOffset);
      } else {
        // TODO: Remove this case when done
        throw runtime_error("Operation::toCode: I screwed up");
      }
      i += 3;
    } else {
      out << c;
      ++i;
    }
  }
  out << '\n';
}

// NOTE: Function::regAlloc handles the stack deallocation
void Return::toCode(ostream& out, const unordered_map<int, size_t>&) const {
  out << "\tretq" << endl;
}
/************
 * toStream *
 ************/

ostream& operator<<(ostream& out, const Instruction& instr) {
  instr.toStream(out);
  return out;
}

void Label::toStream(ostream& out) const { out << name_ << ':'; }

void Move::toStream(ostream& out) const {
  out << "MOVE"
      << " [" << src_ << "] [" << dst_ << ']';
}

void Operation::toStream(ostream& out) const {
  out << asmCode_ << ' ';
  // Operators should be defined in the same namespace as the class so they
  // can be found via ADL. Unfortunately, you can't add stuff to the std
  // namespace, so we get this nonsense
  detail::printContainerHelper(out, srcs_);
  out << ' ';
  detail::printContainerHelper(out, dsts_);
}

void JumpOp::toStream(ostream& out) const {
  Operation::toStream(out);
  out << " [";
  for (const Instruction* instr : jumps_) {
    out << *instr << ", ";
  }
  out << ']';
}

void Return::toStream(ostream& out) const {
  out << "RETURN (" << (hasValue_ ? "" : "no ") << "value)" << ':';
}


}
