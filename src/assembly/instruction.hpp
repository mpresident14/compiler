#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include "temp.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <memory>
#include <vector>

enum class InstrType { LABEL, MOVE, OPER };

class Instruction {
  virtual InstrType getType() const noexcept = 0;
};


class Label : public Instruction {
public:
  Label(const std::string& name);
  virtual InstrType getType() const noexcept override;
private:
  std::string name_;
};

class Move : public Instruction {
public:
  Move(const std::string& asmCode, std::unique_ptr<Temp> src, std::unique_ptr<Temp> dst);
  virtual InstrType getType() const noexcept override;
private:
  std::string asmCode_;
  std::unique_ptr<Temp> src_;
  std::unique_ptr<Temp> dst_
};

class Operation : public Instruction {
public:
Operation::Operation(const std::string& asmCode,
    std::vector<std::unique_ptr<Temp>> reads,
    std::vector<std::unique_ptr<Temp>> writes);
  virtual InstrType getType() const noexcept override;
private:
  std::string asmCode_;
  std::vector<std::unique_ptr<Temp>> reads_;
  std::vector<std::unique_ptr<Temp>> writes_;
};


#endif
