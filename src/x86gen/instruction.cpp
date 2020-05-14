#include "src/x86gen/instruction.hpp"

#include <stdexcept>
#include <prez/print_stuff.hpp>

using namespace std;

// TODO: Turn at()s into []s when all testing is complete.

/****************
 * constructors *
 ****************/
Label::Label(string&& name) : name_(move(name)) {}

Move::Move(const string& asmCode, int src, int dst)
    : asmCode_(asmCode), src_(move(src)), dst_(move(dst)) {}

Operation::Operation(
    const string& asmCode,
    vector<int>&& srcs,
    vector<int>&& dsts,
    optional<vector<Instruction*>>&& jumps)
    : asmCode_(asmCode),
      srcs_(move(srcs)),
      dsts_(move(dsts)),
      jumps_(move(jumps)) {}


InstrType Label::getType() const noexcept { return InstrType::LABEL; }
InstrType Move::getType() const noexcept { return InstrType::MOVE; }
InstrType Operation::getType() const noexcept { return InstrType::OPER; }

/***********
 * getVars *
 ***********/

void Label::getVars(vector<int>&) const noexcept {}

void Move::getVars(vector<int>& vars) const noexcept {
  if (!isRegister(src_)) {
    vars.push_back(src_);
  }
  if (!isRegister(dst_)) {
    vars.push_back(dst_);
  }
}
void Operation::getVars(vector<int>& vars) const noexcept {
  for (int temp : srcs_) {
    if (!isRegister(temp)) {
      vars.push_back(temp);
    }
  }

  for (int temp : dsts_) {
    if (!isRegister(temp)) {
      vars.push_back(temp);
    }
  }
}

constexpr MachineReg SPILL_REGS[]{ R10, R11 };

constexpr int digitToInt(char c) noexcept { return c - '0'; }

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
    newInstrs.push_back(make_unique<Move>(
        asmCode_, src_, isRegister(dst_) > 0 ? dst_ : SPILL_REGS[0]));
  }

  if (!isRegister(dst_)) {
    newInstrs.push_back(make_unique<Move>(
        asmCode_, isRegister(src_) > 0 ? src_ : SPILL_REGS[0], dst_));
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
      newInstrs.push_back(make_unique<Move>("movq >, <", src, spillReg));
    }
  }

  // Insert the updated instruction
  return true;
}

/**************
 * assignRegs *
 **************/

void assignReg(int& temp, const std::unordered_map<int, MachineReg>& coloring) {
  auto iter = coloring.find(temp);
  if (iter != coloring.end()) {
    temp = iter->second;
  }
}


void Label::assignRegs(const std::unordered_map<int, MachineReg>&) {
  return;
}

void Move::assignRegs(const std::unordered_map<int, MachineReg>& coloring) {
  assignReg(src_, coloring);
  assignReg(dst_, coloring);
}

void Operation::assignRegs(const std::unordered_map<int, MachineReg>& coloring) {
  for (int& src : srcs_) {
    assignReg(src, coloring);
  }
  for (int& dst : dsts_) {
    assignReg(dst, coloring);
  }
}

/**********
 * toCode *
 **********/

/* > for srcs, < for dsts */

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

void Label::toCode(std::ostream& out, const unordered_map<int, size_t>&) const {
  out << name_ << ":\n";
}

void Move::toCode(
    std::ostream& out,
    const unordered_map<int, size_t>& varToStackOffset) const {
  if (src_ == dst_) {
    return;
  }

  out << '\t';
  for (char c : asmCode_) {
    if (c == '>') {
      tempToCode(out, src_, varToStackOffset);
    } else if (c == '<') {
      tempToCode(out, dst_, varToStackOffset);
    } else {
      out << c;
    }
  }
  out << '\n';
}

void Operation::toCode(
    std::ostream& out,
    const unordered_map<int, size_t>& varToStackOffset) const {
  out << '\t';
  size_t len = asmCode_.size();
  size_t i = 0;
  while (i < len) {
    char c = asmCode_.at(i);
    if (c == '>') {
      tempToCode(
          out, srcs_.at(digitToInt(asmCode_.at(i + 1))), varToStackOffset);
      i += 2;
    } else if (c == '<') {
      tempToCode(
          out, dsts_.at(digitToInt(asmCode_.at(i + 1))), varToStackOffset);
      i += 2;
    } else {
      out << c;
      ++i;
    }
  }
  out << '\n';
}


/************
 * toStream *
 ************/

std::ostream& operator<<(std::ostream& out, const Instruction& instr) {
  instr.toStream(out);
  return out;
}

void Label::toStream(std::ostream& out) const {
  out << name_ << ':';
}
void Move::toStream(std::ostream& out) const {
  out << asmCode_ << " [" << src_ << "] [" << dst_ << ']';
}
void Operation::toStream(std::ostream& out) const {
  out << asmCode_ << ' ' << srcs_ << ' ' << dsts_ << ' ';
  if (jumps_.has_value()) {
    out << '[';
    for (const Instruction* instr : *jumps_) {
      out << *instr << ", ";
    }
    out << ']';
  }
}


/***********
 * getters *
 ***********/

const std::string& Label::getName() const noexcept { return name_; }

const std::string& Move::getAsm() const noexcept { return asmCode_; }
int Move::getSrc() const noexcept { return src_; }
int Move::getDst() const noexcept { return dst_; }

const std::string& Operation::getAsm() const noexcept { return asmCode_; }
const std::vector<int>& Operation::getSrcs() const noexcept { return srcs_; }
const std::vector<int>& Operation::getDsts() const noexcept { return dsts_; }
const std::optional<std::vector<Instruction*>>& Operation::getJumps() const noexcept {
  return jumps_;
}
