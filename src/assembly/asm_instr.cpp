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
    vector<int>&& dsts,
    bool hasMemRefs)
    : asmCode_(asmCode),
      srcs_(move(srcs)),
      dsts_(move(dsts)),
      hasMemRefs_(hasMemRefs) {}

JumpOp::JumpOp(
    string_view asmCode,
    vector<int>&& srcs,
    vector<int>&& dsts,
    Label* jump)
    : Operation(asmCode, move(srcs), move(dsts)), jump_(jump) {}


namespace {
  constexpr int digitToInt(char c) noexcept { return c - '0'; }
}  // namespace


/*****************
 * updateLiveout *
 *****************/

namespace {

  /* Returns true if a new element was inserted */
  template <typename T, template <typename...> class Container>
  bool setUnion(unordered_set<T>& to, const Container<T>& from) {
    bool newInsertion = false;
    for (const T& n : from) {
      newInsertion |= to.insert(n).second;
    }
    return newInsertion;
  }

}  // namespace


bool Instruction::updateLiveOut(
    std::unordered_set<int>& liveOut,
    const Instruction* prevInstr,
    const std::unordered_map<const Instruction*, Liveness>& nodes) const {
  // Instruction falls through by default
  return setUnion(liveOut, nodes.at(prevInstr).liveIn);
}


bool JumpOp::updateLiveOut(
    std::unordered_set<int>& liveOut,
    const Instruction*,
    const std::unordered_map<const Instruction*, Liveness>& nodes) const {
  // Instruction jumps unconditionally
  return setUnion(liveOut, nodes.at(jump_).liveIn);
}

bool CondJumpOp::updateLiveOut(
    std::unordered_set<int>& liveOut,
    const Instruction* prevInstr,
    const std::unordered_map<const Instruction*, Liveness>& nodes) const {
  // Instruction may jump or fall through
  return Instruction::updateLiveOut(liveOut, prevInstr, nodes) ||
         JumpOp::updateLiveOut(liveOut, prevInstr, nodes);
}

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
        make_unique<Move>(src_, isRegister(dst_) ? dst_ : SPILL_REGS[0]));
  }

  if (!isRegister(dst_)) {
    newInstrs.push_back(
        make_unique<Move>(isRegister(src_) ? src_ : SPILL_REGS[0], dst_));
  }

  return false;
}

bool Operation::spillTemps(vector<InstrPtr>& newInstrs) {
  // For each temporary src (and maybe dst, see below), if it is a non-machine
  // register, then update it to a spill register and add an instruction to move
  // it from the stack to the spill register
  size_t numSpilled = 0;
  vector<InstrPtr> postOpInstrs;

  // - movq srcs, spilledSrcs
  size_t numSrcs = srcs_.size();
  for (size_t i = 0; i < numSrcs; ++i) {
    int src = srcs_[i];
    if (!isRegister(src)) {
      int spillReg = SPILL_REGS[numSpilled++];
      srcs_[i] = spillReg;
      newInstrs.push_back(make_unique<Move>(src, spillReg));
      if (hasMemRefs_) {
        // See comment below
        postOpInstrs.push_back(make_unique<Move>(spillReg, src));
      }
    }
  }

  if (hasMemRefs_) {
    // The instruction contains memory references, so we cannot access the stack
    // for the srcs or dsts. For example: "movq (%t1), %t2" cannot be "movq
    // (8(%rsp)), %t2" or "movq (%t1), (16(%rsp))"
    // - movq dsts, spillDsts
    // - op spillSrcs, spillDsts
    // - movq spilledSrcs, srcs
    // - movq spillDsts, dsts
    size_t numDsts = dsts_.size();
    vector<int> newDsts;
    for (size_t i = 0; i < numDsts; ++i) {
      int dst = dsts_[i];
      if (!isRegister(dst)) {
        int spillReg = SPILL_REGS[numSpilled++];
        dsts_[i] = spillReg;
        newInstrs.push_back(make_unique<Move>(dst, spillReg));
        postOpInstrs.push_back(make_unique<Move>(spillReg, dst));
      }
    }
    newInstrs.emplace_back(new Operation(asmCode_, move(srcs_), move(dsts_)));
    for (InstrPtr& instr : postOpInstrs) {
      newInstrs.push_back(move(instr));
    }
    return false;
  } else {
    // No memory references, just insert the instruction with the updated srcs
    // - op spillSrcs, dsts
    return true;
  }
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
    u_char numBytes,
    const unordered_map<int, size_t>& varToStackOffset) {
  if (isRegister(temp)) {
    out << regToString(static_cast<MachineReg>(temp), numBytes);
  } else {
#ifdef DEBUG
    out << "%t" << -temp;
#else
    out << varToStackOffset.at(temp) << '(' << regToString(RSP, numBytes)
        << ')';
#endif
  }
}

void streamTemp(ostream& out, int temp) { tempToCode(out, temp, 8, {}); }

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
  tempToCode(out, src_, 8, varToStackOffset);
  out << ", ";
  tempToCode(out, dst_, 8, varToStackOffset);
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
      int bytesChar = digitToInt(asmCode_.at(i + 1));
      c = asmCode_.at(i + 2);
      if (c == 'S') {
        tempToCode(
            out,
            srcs_.at(digitToInt(asmCode_.at(i + 3))),
            bytesChar,
            varToStackOffset);
      } else if (c == 'D') {
        tempToCode(
            out,
            dsts_.at(digitToInt(asmCode_.at(i + 3))),
            bytesChar,
            varToStackOffset);
      } else {
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
      << " [";
  streamTemp(out, src_);
  out << "] [";
  streamTemp(out, dst_);
  out << ']';
}

void Operation::toStream(ostream& out) const {
  out << asmCode_ << ' ';
  out << '[';
  for (int src : srcs_) {
    streamTemp(out, src);
    out << ", ";
  }
  out << ']';
  out << ' ';
  out << '[';
  for (int dst : dsts_) {
    streamTemp(out, dst);
    out << ", ";
  }
  out << ']';
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
