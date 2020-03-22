#ifndef REGEX_EVAL_HPP
#define REGEX_EVAL_HPP

#include "regex.hpp"
#include "regex_lexer.hpp"
#include "regex_grammar.hpp"
#include "parse.hpp"
#include "dfa.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <queue>


bool matches(const std::string& pattern, const std::string& input);

#endif
