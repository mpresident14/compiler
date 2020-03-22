#ifndef REGEX_MATCH_HPP
#define REGEX_MATCH_HPP

#include "regex.hpp"
#include "regex_lexer.hpp"
#include "regex_grammar.hpp"
#include "parse.hpp"
#include "dfa.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <queue>
#include <numeric>
#include <functional>


bool matches(const std::string& pattern, const std::string& input);

#endif
