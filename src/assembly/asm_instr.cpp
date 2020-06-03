#include "src/assembly/assembly.hpp"

#include <stdexcept>

#include <prez/print_stuff.hpp>

using namespace std;

namespace assem {

// TODO: Turn at()s into []s and make noexcept when all testing is complete.

/****************
 * constructors *
 ****************/
Label::Label(string_view name) : name_(name) {}

Move::Move(int src, int dst) : src_(src), dst_(dst) {}

Operation::Operation(
    string_view asmCode,
    vector<int>&& srcs,
    vector<int>&& dsts)
    : asmCode_(asmCode), srcs_(move(srcs)), dsts_(move(dsts)) {}

JumpOp::JumpOp(
    string_view asmCode,
    vector<int>&& srcs,
    vector<int>&& dsts,
    Label* jump)
    : Operation(asmCode, move(srcs), move(dsts)), jump_(jump) {}


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
/* Returns true if temp was assigned to a machine register, false otherwise
 * (will be spilled) */
bool assignReg(int& temp, const unordered_map<int, MachineReg>& coloring) {
  auto iter = coloring.find(temp);
  if (iter != coloring.end()) {
    temp = iter->second;
    return true;
  }
  return false;
}

void Label::assignRegs(
    const unordered_map<int, MachineReg>&,
    bitset<NUM_AVAIL_REGS>&) {
  return;
}

void Move::assignRegs(
    const unordered_map<int, MachineReg>& coloring,
    bitset<NUM_AVAIL_REGS>& writtenRegs) {
  assignReg(src_, coloring);
  if (assignReg(dst_, coloring)) {
    writtenRegs.set(dst_);
  }
}

void Operation::assignRegs(
    const unordered_map<int, MachineReg>& coloring,
    bitset<NUM_AVAIL_REGS>& writtenRegs) {
  for (int& src : srcs_) {
    assignReg(src, coloring);
  }
  for (int& dst : dsts_) {
    if (assignReg(dst, coloring)) {
      writtenRegs.set(dst);
    }
  }
}

void Return::assignRegs(
    const unordered_map<int, MachineReg>&,
    bitset<NUM_AVAIL_REGS>&) {
  return;
}

/**********
 * toCode *
 **********/

void tempToCode(
    ostream& out,
    int temp,
    char numBytes,
    const unordered_map<int, size_t>& varToStackOffset) {
  if (isRegister(temp)) {
    out << regToString(static_cast<MachineReg>(temp), numBytes);
  } else {
#ifdef DEBUG
    out << "%t" << -temp;
#else
    out << varToStackOffset.at(temp) << '(' << regToString(RSP, numBytes) << ')';
#endif
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
  tempToCode(out, src_, '8', varToStackOffset);
  out << ", ";
  tempToCode(out, dst_, '8', varToStackOffset);
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
      char numBytes = asmCode_.at(i + 1);
      c = asmCode_.at(i + 2);
      if (c == 'S') {
        tempToCode(
            out, srcs_.at(digitToInt(asmCode_.at(i + 3))), numBytes, varToStackOffset);
      } else if (c == 'D') {
        tempToCode(
            out, dsts_.at(digitToInt(asmCode_.at(i + 3))), numBytes, varToStackOffset);
      } else {
        // TODO: Remove this case when done
        throw runtime_error("Operation::toCode");
      }
      i += 4;
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
  out << *jump_;
  out << ']';
}

void Return::toStream(ostream& out) const {
  out << "RETURN (" << (hasValue_ ? "" : "no ") << "value)" << ':';
}


}  // namespace assem
