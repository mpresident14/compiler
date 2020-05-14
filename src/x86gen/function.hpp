#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "src/x86gen/instruction.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <cstddef>
#include <unordered_map>


class Function {
public:
  Function(
      std::string&& name,
      std::vector<InstrPtr>&& instrs);
  ~Function() = default;
  Function(const Function&) = delete;
  Function(Function&&) = default;
  Function& operator=(const Function&) = delete;
  Function& operator=(Function&&) = default;

  void toCode(std::ostream& out);

private:
  void regAlloc();

  std::string name_;
  std::vector<InstrPtr> instrs_;
  std::unordered_map<int, size_t> varToStackOffset_;
};

class Program {
public:
  Program(std::string&& name, std::vector<Function>&& fns);
  ~Program() = default;
  Program(const Program&) = delete;
  Program(Program&&) = default;
  Program& operator=(const Program&) = delete;
  Program& operator=(Program&&) = default;

  void toCode(std::ostream& out);

private:
  std::string name_;
  std::vector<Function> fns_;
};

#endif
