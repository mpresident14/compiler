#ifndef PARSER_HPP
#define PARSER_HPP

/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */

#include "src/language/language.hpp"

#include <iostream>
#include <string>
namespace parser {
language::Program parse(const std::string& input);
language::Program parse(std::istream& input);
}  // namespace parser
#endif