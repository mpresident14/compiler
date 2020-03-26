#ifndef BUILD_PARSER_HPP
#define BUILD_PARSER_HPP

#include "utils.hpp"
#include "dfa.hpp"
#include "rules.hpp"
#include "null_first.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <memory>
#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <prez/print_stuff.hpp>

DFA<DFARuleSet, int> buildParserDFA(const Grammar& grammar, size_t numSymbols);


#endif
