#ifndef CONFIG_PARSE_HPP
#define CONFIG_PARSE_HPP

#include "utils.hpp"

#include <string>

struct ParseInfo {
  GrammarData grammarData;
  std::string addlHppCode;
  std::string addlCppCode;
};

ParseInfo parseConfig(const std::string& fileName);

#endif
