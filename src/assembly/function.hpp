#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "src/assembly/instruction.hpp"

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
  void toCode(std::ostream& out);

private:
  void regAlloc();

  std::string name_;
  std::vector<InstrPtr> instrs_;
  std::unordered_map<int, size_t> varToStackOffset_;
};

#endif
