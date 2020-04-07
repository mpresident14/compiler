#ifndef CONFIG_LEXER_HPP
#define CONFIG_LEXER_HPP

#include "utils.hpp"

#include <vector>
#include <string>

struct StackObj {
  // Can't delete from here since it is a void*, see constructObj
  void* obj;
  int symbol;
};

std::vector<StackObj> tokenize(const std::string& input);

#endif
