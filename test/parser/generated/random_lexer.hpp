#ifndef TEST_PARSER_GENERATED_RANDOM_LEXER_HPP
#define TEST_PARSER_GENERATED_RANDOM_LEXER_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include <vector>
#include <string>
#include <iostream>
namespace test::parser::generated::random_lexer {
  struct StackObj {
    void* obj;
    int symbol;
    size_t line;
  };
  void deleteObj(const StackObj& s);
  std::vector<StackObj> tokenize(const std::string& input);
  std::vector<StackObj> tokenize(std::istream& input);
}  // namespace test::parser::generated::random_lexer
#endif