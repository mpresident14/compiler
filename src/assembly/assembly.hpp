#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

#include "src/assembly/temp.hpp"

#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <bitset>

namespace assem {


enum class InstrType { LABEL, MOVE, OPER, JUMP_OP, COND_JUMP_OP, RETURN };

class Instruction;
using InstrPtr = std::unique_ptr<Instruction>;


class Instruction {
public:
  virtual ~Instruction() {}
  virtual InstrType getType() const noexcept = 0;
  // TODO: This needs to be tested with code that spills both spill regs, e.g. a leaq instruction
  virtual bool spillTemps(std::vector<InstrPtr>& newInstrs) = 0;
  /* Assigns temps to machine registers and keeps track of which machine registers are written */
  virtual void assignRegs(
      const std::unordered_map<int, MachineReg>& coloring, std::bitset<NUM_AVAIL_REGS>& writtenRegs) = 0;
  virtual void toCode(
      std::ostream& out,
      const std::unordered_map<int, size_t>& varToStackOffset) const = 0;
  virtual void toStream(std::ostream& out) const = 0;
  friend std::ostream& operator<<(std::ostream& out, const Instruction& instr);
};

class Decl {
public:
  virtual ~Decl() {}
  virtual void toCode(std::ostream& out) = 0;

private:
  std::vector<InstrPtr> instrs_;
};

using DeclPtr = std::unique_ptr<Decl>;


class Program {
public:
  Program(std::vector<DeclPtr>&& decls);
  void toCode(std::ostream& asmFile);

private:
  std::vector<DeclPtr> decls_;
};

/********
 * Decl *
 ********/

class Function : public Decl {
public:
  Function(const std::string& name, std::vector<InstrPtr>&& instrs);
  void toCode(std::ostream& out) override;

private:
  std::bitset<NUM_AVAIL_REGS> regAlloc(const std::unordered_map<int, MachineReg>& coloring, const std::vector<int>& spilled);
  std::pair<std::vector<InstrPtr>, std::vector<InstrPtr>> preserveRegs(const std::bitset<NUM_AVAIL_REGS>& writtenRegs);

  std::string name_;
  std::vector<InstrPtr> instrs_;
  std::unordered_map<int, size_t> varToStackOffset_;
};

/***************
 * Instruction *
 ***************/

class Label : public Instruction {
public:
  Label(const std::string& name);
  constexpr InstrType getType() const noexcept override {
    return InstrType::LABEL;
  }
  bool spillTemps(std::vector<InstrPtr>& newInstrs) override;
  void assignRegs(const std::unordered_map<int, MachineReg>& coloring, std::bitset<NUM_AVAIL_REGS>& writtenRegs) override;
  void toCode(
      std::ostream& out,
      const std::unordered_map<int, size_t>& varToStackOffset) const override;
  void toStream(std::ostream& out) const override;

  const std::string& getName() const noexcept { return name_; }

private:
  std::string name_;
};

class Move : public Instruction {
public:
  Move(int src, int dst);
  constexpr InstrType getType() const noexcept override {
    return InstrType::MOVE;
  }
  bool spillTemps(std::vector<InstrPtr>& newInstrs) override;
  void assignRegs(const std::unordered_map<int, MachineReg>& coloring, std::bitset<NUM_AVAIL_REGS>& writtenRegs) override;
  void toCode(
      std::ostream& out,
      const std::unordered_map<int, size_t>& varToStackOffset) const override;
  void toStream(std::ostream& out) const override;

  int getSrc() const noexcept { return src_; }
  int getDst() const noexcept { return dst_; }

private:
  int src_;
  int dst_;
};


/* An instruction that always falls thru to the next one */
class Operation : public Instruction {
public:
  Operation(
      const std::string& asmCode,
      std::vector<int>&& srcs,
      std::vector<int>&& dsts);
  constexpr InstrType getType() const noexcept override {
    return InstrType::OPER;
  }
  bool spillTemps(std::vector<InstrPtr>& newInstrs) override;
  void assignRegs(const std::unordered_map<int, MachineReg>& coloring, std::bitset<NUM_AVAIL_REGS>& writtenRegs) override;
  void toCode(
      std::ostream& out,
      const std::unordered_map<int, size_t>& varToStackOffset) const override;
  void toStream(std::ostream& out) const override;

  const std::string& getAsm() const noexcept { return asmCode_; }
  const std::vector<int>& getSrcs() const noexcept { return srcs_; }
  const std::vector<int>& getDsts() const noexcept { return dsts_; }

private:
  std::string asmCode_;
  std::vector<int> srcs_;
  std::vector<int> dsts_;
};

/* A jmp instruction. Guaranteed to jump, will not fall thru */
class JumpOp : public Operation {
public:
  JumpOp(
      const std::string& asmCode,
      std::vector<int>&& srcs,
      std::vector<int>&& dsts,
      Label* jump);
  constexpr InstrType getType() const noexcept override {
    return InstrType::JUMP_OP;
  }
  virtual void toStream(std::ostream& out) const override;

  const Label* getJump() const noexcept { return jump_; }

private:
  Label* jump_;
};


/* Same as JumpOp, but might fall thru */
class CondJumpOp : public JumpOp {
public:
  using JumpOp::JumpOp;
  constexpr InstrType getType() const noexcept override {
    return InstrType::COND_JUMP_OP;
  }
};


class Return : public Instruction {
public:
  explicit constexpr Return(bool hasValue) : hasValue_(hasValue) {}
  constexpr InstrType getType() const noexcept override {
    return InstrType::RETURN;
  }
  virtual bool spillTemps(std::vector<InstrPtr>& newInstrs) override;
  virtual void assignRegs(
      const std::unordered_map<int, MachineReg>& coloring, std::bitset<NUM_AVAIL_REGS>& writtenRegs) override;
  virtual void toCode(
      std::ostream& out,
      const std::unordered_map<int, size_t>& varToStackOffset) const override;
  virtual void toStream(std::ostream& out) const override;

  constexpr bool hasValue() const noexcept { return hasValue_; }

private:
  bool hasValue_;
};


}  // namespace assem

#endif  // ASSEMBLY_HPP
