#ifndef REGEX_LEXER_HPP
#define REGEX_LEXER_HPP

#include "regex_grammar.hpp"
#include "regex.hpp"

#include <cstddef>
#include <string>
#include <vector>

std::vector<StackObj> lex(const std::string& input);

#endif
