#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include "temp.hpp"

#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace x86_64 {


enum class InstrType { LABEL, MOVE, OPER, JUMP_OP, COND_JUMP_OP, RETURN };

class Instruction;
using InstrPtr = std::unique_ptr<Instruction>;


class Instruction {
public:
  virtual ~Instruction() {}
  virtual InstrType getType() const noexcept = 0;
  virtual bool spillTemps(std::vector<InstrPtr>& newInstrs) = 0;
  virtual void assignRegs(
      const std::unordered_map<int, MachineReg>& coloring) = 0;
  virtual void toCode(
      std::ostream& out,
      const std::unordered_map<int, size_t>& varToStackOffset) const = 0;
  virtual void toStream(std::ostream& out) const = 0;
  friend std::ostream& operator<<(std::ostream& out, const Instruction& instr);
};

class Decl {
public:
  virtual ~Decl() {}
  virtual void toX86_64(std::ostream& out);
private:
  std::vector<InstrPtr> instrs_;
};

using DeclPtr = std::unique_ptr<Decl>;


struct Program {
  void toX86_64(const std::string& fileName);

  std::string fileName_;
  std::vector<DeclPtr> decls;
};

/********
 * Decl *
 ********/

class Function : public Decl {
public:
  Function(const std::string& name, std::vector<InstrPtr>&& instrs);
  void toX86_64(std::ostream& out) override;

private:
  void regAlloc();

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
  void assignRegs(const std::unordered_map<int, MachineReg>& coloring) override;
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
  void assignRegs(const std::unordered_map<int, MachineReg>& coloring) override;
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
  void assignRegs(const std::unordered_map<int, MachineReg>& coloring) override;
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


class JumpOp : public Operation {
public:
  JumpOp(
      const std::string& asmCode,
      std::vector<int>&& srcs,
      std::vector<int>&& dsts,
      std::vector<Instruction*>&& jumps);
  constexpr InstrType getType() const noexcept override {
    return InstrType::JUMP_OP;
  }
  virtual void toStream(std::ostream& out) const override;

  const std::vector<Instruction*>& getJumps() const noexcept { return jumps_; }

private:
  // If empty then we are jumping out of
  // the function entirely
  std::vector<Instruction*> jumps_;
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
      const std::unordered_map<int, MachineReg>& coloring) override;
  virtual void toCode(
      std::ostream& out,
      const std::unordered_map<int, size_t>& varToStackOffset) const override;
  virtual void toStream(std::ostream& out) const override;

  constexpr bool hasValue() const noexcept { return hasValue_; }

private:
  bool hasValue_;
};


}

#endif
