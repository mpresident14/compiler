#ifndef CONFIG_PARSE_HPP
#define CONFIG_PARSE_HPP

#include "src/misc/logger.hpp"
#include "src/parser/utils.hpp"

#include <ostream>
#include <string>
#include <vector>

struct ParseInfo {
  GrammarData gd;
  std::string addlHppCode;
  std::string addlCppCode;
};

ParseInfo parseConfig(
    const std::string& fileName,
    std::ostream& warnings);

#endif
