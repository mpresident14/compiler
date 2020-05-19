#ifndef REGEX_PARSER_HPP
#define REGEX_PARSER_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include "src/parser/regex.hpp"

#include <iostream>
#include <string>
namespace regex_parser {
Regex* parse(const std::string& input);
Regex* parse(std::istream& input);
class ParseException : public std::exception {
public:
  ParseException(const std::string& errMsg) : errMsg_(errMsg) {}
  ParseException(const char* errMsg) : errMsg_(errMsg) {}
  const char* what() const noexcept override { return errMsg_.c_str(); }

private:
  std::string errMsg_;
};
}  // namespace regex_parser
#endif