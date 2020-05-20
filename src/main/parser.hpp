#ifndef PARSER_HPP
#define PARSER_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include "src/language/language.hpp"

#include <iostream>
#include <string>
namespace parser {
language::Program parse(const std::string& input);
language::Program parse(std::istream& input);
class ParseException : public std::exception {
public:
  ParseException(const std::string& errMsg) : errMsg_(errMsg) {}
  ParseException(const char* errMsg) : errMsg_(errMsg) {}
  const char* what() const noexcept override { return errMsg_.c_str(); }

private:
  std::string errMsg_;
};
}  // namespace parser
#endif