#ifndef NULL_FIRST_FOLLOW_HPP
#define NULL_FIRST_FOLLOW_HPP

#include "grammar1.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <bitset>
#include <algorithm>
#include <array>

constexpr size_t numVariables = toInt(Symbol::STARTTOKENS);
constexpr size_t numTokens = toInt(Symbol::EPSILON) - toInt(Symbol::STARTTOKENS) - 1;
using BitSetVars = std::bitset<numVariables>;
using BitSetToks = std::bitset<numTokens>;
using BitRef = BitSetVars::reference;

BitSetVars getNullabilities();
std::vector<BitSetToks> getFirsts();

#endif
