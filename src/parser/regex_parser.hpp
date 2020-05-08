#ifndef SRC_PARSER_REGEX_PARSER_HPP
#define SRC_PARSER_REGEX_PARSER_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include "src/parser/regex.hpp"

#include <iostream>
#include <string>
namespace src::parser::regex_parser {
  Regex* parse(const std::string& input);
  Regex* parse(std::istream& input);
}  // namespace src::parser::regex_parser
#endif