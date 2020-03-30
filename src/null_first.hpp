#ifndef NULL_FIRST_HPP
#define NULL_FIRST_HPP

#include "utils.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <bitset>

std::vector<bool> getNullabilities(const GrammarData& grammarData);
std::vector<std::vector<bool>> getFirsts(const GrammarData& grammarData);

#endif
