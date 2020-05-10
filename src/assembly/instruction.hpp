#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include "temp.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>

enum class InstrType { LABEL, MOVE, OPER };

class Instruction {
public:
  virtual ~Instruction() {}
  virtual InstrType getType() const noexcept = 0;
  virtual void getVars(std::vector<int>& vars) const noexcept = 0;
  virtual void spillTemps(
      const std::unordered_map<int, size_t>& varToStackOffset,
      std::vector<std::unique_ptr<Instruction>>& newInstrs) = 0;
  virtual void toStream(std::ostream& out) const = 0;
  friend std::ostream& operator<<(std::ostream& out, const Instruction& instr);
};


class Label : public Instruction {
public:
  Label(std::string&& name);
  virtual InstrType getType() const noexcept override;
  virtual void getVars(std::vector<int>& vars) const noexcept override;
  virtual void spillTemps(
      const std::unordered_map<int, size_t>& varToStackOffset,
      std::vector<std::unique_ptr<Instruction>>& newInstrs) override;
  virtual void toStream(std::ostream& out) const override;

private:
  std::string name_;
};

class Move : public Instruction {
public:
  Move(const std::string& asmCode, int src, int dst);
  virtual InstrType getType() const noexcept override;
  virtual void getVars(std::vector<int>& vars) const noexcept override;
  virtual void spillTemps(
      const std::unordered_map<int, size_t>& varToStackOffset,
      std::vector<std::unique_ptr<Instruction>>& newInstrs) override;
  virtual void toStream(std::ostream& out) const override;
private:
  std::string asmCode_;
  int src_;
  int dst_;
};

class Operation : public Instruction {
public:
Operation(const std::string& asmCode,
    std::vector<int>&& srcs,
    std::vector<int>&& dsts,
    std::optional<std::vector<std::string>>&& jumps);
  virtual InstrType getType() const noexcept override;
  virtual void getVars(std::vector<int>& vars) const noexcept override;
  virtual void spillTemps(
      const std::unordered_map<int, size_t>& varToStackOffset,
      std::vector<std::unique_ptr<Instruction>>& newInstrs) override;
  virtual void toStream(std::ostream& out) const override;
private:
  std::string replaceDsts(const std::unordered_map<int, size_t>& varToStackOffset) const;

  std::string asmCode_;
  std::vector<int> srcs_;
  std::vector<int> dsts_;
  // No value if falling through to next instruction,
  // empty list if jumping out of function entirely
  std::optional<std::vector<std::string>> jumps_;
};


class Function {
public:
  Function(std::string&& name, std::vector<std::unique_ptr<Instruction>>&& instrs);
  void allocate();
  friend std::ostream& operator<<(std::ostream& out, const Function& fn);

private:
  std::string name_;
  std::vector<std::unique_ptr<Instruction>> instrs_;
};


#endif
