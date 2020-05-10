#include "instruction.hpp"

#include <stdexcept>
#include <prez/print_stuff.hpp>

using namespace std;

// TODO: Turn at()s into []s when all testing is complete.


std::ostream& operator<<(std::ostream& out, const Instruction& instr) {
  instr.toStream(out);
  return out;
}

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
    optional<vector<string>>&& jumps)
  : asmCode_(asmCode), srcs_(move(srcs)), dsts_(move(dsts)), jumps_(move(jumps)) {}


InstrType Label::getType() const noexcept { return InstrType::LABEL; }
InstrType Move::getType() const noexcept { return InstrType::MOVE; }
InstrType Operation::getType() const noexcept { return InstrType::OPER; }

Function::Function(std::string&& name, std::vector<std::unique_ptr<Instruction>>&& instrs)
 : name_(move(name)), instrs_(move(instrs)) {}

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

// Spill registers are R14 and R15
constexpr MachineRegs SPILL_REGS[] {R10, R11};

constexpr int digitToInt(char c) noexcept {
  return c - '0';
}


/* Replace variable destinations with their offset on the stack */
string Operation::replaceDsts(const unordered_map<int, size_t>& varToStackOffset) const {
  size_t len = asmCode_.size();
  size_t i = 0;
  string newAssem;
  newAssem.reserve(len);
  while (i < len) {
    char c = asmCode_.at(i);
    if (asmCode_.at(i) == 'D') {
      int temp = dsts_[digitToInt(asmCode_.at(i+1))];
      if (!isRegister(temp)) {
        newAssem.append(to_string(varToStackOffset.at(temp)));
        newAssem.append("(%rsp)");
      }
      i += 2;
    } else {
      newAssem.push_back(c);
      ++i;
    }
  }
  return newAssem;
}


/**************
 * spillTemps *
 **************/

void Label::spillTemps(
    const unordered_map<int, size_t>&,
    vector<unique_ptr<Instruction>>& newInstrs) {
  newInstrs.push_back(make_unique<Label>(*this));
}

void Move::spillTemps(
    const unordered_map<int, size_t>& varToStackOffset,
    vector<unique_ptr<Instruction>>& newInstrs) {

  // Src variable from stack into register
  if (!isRegister(src_)) {
    newInstrs.emplace_back(new Operation(
      "movq " + to_string(varToStackOffset.at(src_)) + "(%rsp), D0",
      vector<int>{},
      vector<int>{isRegister(dst_) > 0 ? dst_ : SPILL_REGS[0]},
      optional<vector<string>>{}));
  }

  // Dst from register to stack
  if (!isRegister(dst_)) {
    newInstrs.emplace_back(new Operation(
        "movq S0, " + to_string(varToStackOffset.at(dst_)) + "(%rsp)",
        vector<int>{isRegister(src_) > 0 ? src_ : SPILL_REGS[0]},
        vector<int>{},
        optional<vector<string>>{}));
  }
  // TODO: Really ugly band-aid for bad design
  else if (isRegister(src_) && isRegister(dst_)) {
    newInstrs.push_back(make_unique<Move>(*this));
  }
}

void Operation::spillTemps(
    const unordered_map<int, size_t>& varToStackOffset,
    vector<unique_ptr<Instruction>>& newInstrs) {

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
      newInstrs.emplace_back(new Operation(
          "movq " + to_string(varToStackOffset.at(src)) + "(%rsp), D0",
          vector<int>{},
          vector<int>{spillReg},
          optional<vector<string>>{}));
    }
  }

  // Remove the variables from the destination list since all variables
  // are now on the stack)
  vector<int> newDsts;
  for (int dst : dsts_) {
    if (isRegister(dst)) {
      newDsts.push_back(dst);
    }
  }

  // Replace the variable destinations with their stack offsets
  string newAssem = replaceDsts(varToStackOffset);
  // Insert the updated instruction
  newInstrs.emplace_back(new Operation(
      newAssem,
      move(srcs_),
      move(newDsts),
      move(jumps_)));
}


void Function::allocate() {
  // TODO: For now, assume all variables are spilled;
  /* Begin temporary code */
  vector<int> spilled;
  for (const unique_ptr<Instruction>& instr : instrs_) {
    instr->getVars(spilled);
  }

  unordered_map<int, size_t> varToStackOffset;
  for (int tempId : spilled) {
    if (!varToStackOffset.contains(tempId)) {
      varToStackOffset.emplace(tempId, 8 * (varToStackOffset.size() + 1));
    }
  }
  /* End temporary code */

  size_t stackSpace = (1 + varToStackOffset.size()) * 8;

  vector<unique_ptr<Instruction>> newInstrs;
  newInstrs.emplace_back(new Operation(
      "subq $" + to_string(stackSpace) + ", %rsp",
      vector<int>(),
      vector<int>(),
      {}));


  for (const unique_ptr<Instruction>& instr : instrs_) {
    instr->spillTemps(varToStackOffset, newInstrs);
  }

  newInstrs.emplace_back(new Operation(
      "addq $" + to_string(stackSpace) + ", %rsp",
      vector<int>(),
      vector<int>(),
      {}));
  newInstrs.emplace_back(new Operation(
      "retq",
      vector<int>(),
      vector<int>(),
      {}));

  instrs_ = move(newInstrs);
}



void Label::toStream(std::ostream& out) const {
  out << name_ << ":\n";
}

void Move::toStream(std::ostream& out) const {
  out << '\t';
  for (char c : asmCode_) {
    if (c == 'S') {
      out << regToString(src_);
    } else if (c == 'D') {
      out << regToString(dst_);
    } else {
      out << c;
    }
  }
  out << '\n';
}

void Operation::toStream(std::ostream& out) const {
  out << '\t';
  size_t len = asmCode_.size();
  size_t i = 0;
  while (i < len) {
    char c = asmCode_.at(i);
    if (c == 'S') {
      out << regToString(srcs_.at(digitToInt(asmCode_.at(i+1))));
      i += 2;
    } else if (c == 'D') {
      out << regToString(dsts_.at(digitToInt(asmCode_.at(i+1))));
      i += 2;
    } else {
      out << c;
      ++i;
    }
  }
  out << '\n';
}

std::ostream& operator<<(std::ostream& out, const Function& fn) {
  out << fn.name_ << ":\n";
  for (const unique_ptr<Instruction>& instr : fn.instrs_) {
    out << *instr;
  }
  return out;
}

