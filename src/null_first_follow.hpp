#ifndef NULL_FIRST_FOLLOW_HPP
#define NULL_FIRST_FOLLOW_HPP

#include "grammar1.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <string>

BitSetVars getNullabilities();
std::vector<BitSetToks> getFirsts();

#endif
