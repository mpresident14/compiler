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

enum class InstrType { LABEL, MOVE, OPER, RETURN };

class Instruction;
using InstrPtr = std::unique_ptr<Instruction>;


class Instruction {
public:
  virtual ~Instruction() {}
  virtual InstrType getType() const noexcept = 0;
  virtual bool spillTemps(std::vector<InstrPtr> &newInstrs) = 0;
  virtual void
  assignRegs(const std::unordered_map<int, MachineReg> &coloring) = 0;
  virtual void
  toCode(std::ostream &out,
         const std::unordered_map<int, size_t> &varToStackOffset) const = 0;
  virtual void toStream(std::ostream &out) const = 0;
  friend std::ostream &operator<<(std::ostream &out, const Instruction &instr);
};

class Label : public Instruction {
public:
  Label(std::string &&name);
  InstrType getType() const noexcept override;
  bool spillTemps(std::vector<InstrPtr> &newInstrs) override;
  void assignRegs(const std::unordered_map<int, MachineReg> &coloring) override;
  void toCode(
      std::ostream &out,
      const std::unordered_map<int, size_t> &varToStackOffset) const override;
  void toStream(std::ostream &out) const override;

  const std::string &getName() const noexcept;

private:
  std::string name_;
};

class Move : public Instruction {
public:
  Move(const std::string &asmCode, int src, int dst);
  InstrType getType() const noexcept override;
  bool spillTemps(std::vector<InstrPtr> &newInstrs) override;
  void assignRegs(const std::unordered_map<int, MachineReg> &coloring) override;
  void toCode(
      std::ostream &out,
      const std::unordered_map<int, size_t> &varToStackOffset) const override;
  void toStream(std::ostream &out) const override;

  const std::string &getAsm() const noexcept;
  int getSrc() const noexcept;
  int getDst() const noexcept;

private:
  std::string asmCode_;
  int src_;
  int dst_;
};

// TODO: jumps_ is normally empty, maybe specialize a class for jumping

class Operation : public Instruction {
public:
  Operation(const std::string &asmCode, std::vector<int> &&srcs,
            std::vector<int> &&dsts,
            std::optional<std::vector<Instruction *>> &&jumps);
  InstrType getType() const noexcept override;
  bool spillTemps(std::vector<InstrPtr> &newInstrs) override;
  void assignRegs(const std::unordered_map<int, MachineReg> &coloring) override;
  void toCode(
      std::ostream &out,
      const std::unordered_map<int, size_t> &varToStackOffset) const override;
  void toStream(std::ostream &out) const override;

  const std::string &getAsm() const noexcept;
  const std::vector<int> &getSrcs() const noexcept;
  const std::vector<int> &getDsts() const noexcept;
  const std::optional<std::vector<Instruction *>> &getJumps() const noexcept;

private:
  std::string asmCode_;
  std::vector<int> srcs_;
  std::vector<int> dsts_;
  // No value if always falls through to next instruction,
  // empty list if jumping out of function entirely
  std::optional<std::vector<Instruction *>> jumps_;
};

class Return : public Instruction {
public:
  explicit constexpr Return(bool hasValue) : hasValue_(hasValue) {}
  virtual InstrType getType() const noexcept override;
  virtual bool spillTemps(std::vector<InstrPtr> &newInstrs) override;
  virtual void
  assignRegs(const std::unordered_map<int, MachineReg> &coloring) override;
  virtual void toCode(
      std::ostream &out,
      const std::unordered_map<int, size_t> &varToStackOffset) const override;
  virtual void toStream(std::ostream &out) const override;

private:
  bool hasValue_;
};

#endif
