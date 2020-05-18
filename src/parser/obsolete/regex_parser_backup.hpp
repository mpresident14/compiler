#ifndef REGEX_PARSER_HPP
#define REGEX_PARSER_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include "src/parser/regex.hpp"

#include <iostream>
#include <string>
namespace regex_parser {
Regex* parse(const std::string& input);
Regex* parse(std::istream& input);
}  // namespace regex_parser
#endif