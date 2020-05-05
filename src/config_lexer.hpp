#ifndef SRC_CONFIG_LEXER_HPP
#define SRC_CONFIG_LEXER_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include <vector>
#include <string>
#include <iostream>
namespace src::config_lexer {
  struct StackObj {
    void* obj;
    int symbol;
    size_t line;
  };
  void deleteObj(const StackObj& s);
  std::vector<StackObj> tokenize(const std::string& input);
  std::vector<StackObj> tokenize(std::istream& input);
}  // namespace src::config_lexer
#endif