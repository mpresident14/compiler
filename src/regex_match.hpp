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
#include <optional>
#include <stdexcept>
#include <sstream>
#include <utility>

using TokenPattern = std::pair<std::string, Symbol>;

bool matches(const std::string& pattern, const std::string& input);
std::vector<StackObj> tokenize(const std::string& input, const std::vector<TokenPattern>& tokenPatterns);

#endif
