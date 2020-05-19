#ifndef CONFIG_PARSE_HPP
#define CONFIG_PARSE_HPP

#include "src/parser/utils.hpp"

#include <string>
#include <vector>

struct ParseInfo {
  GrammarData grammarData;
  std::string addlHppCode;
  std::string addlCppCode;
  std::string fileName;
  std::vector<size_t> concreteLines;
};

ParseInfo parseConfig(const std::string& fileName);

#endif
