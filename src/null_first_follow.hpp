#ifndef NULL_FIRST_FOLLOW_HPP
#define NULL_FIRST_FOLLOW_HPP

#include "grammar1.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <bitset>
#include <algorithm>

using BitSet = std::bitset<static_cast<int>(Symbol::STARTTOKENS)>;
using BitRef = BitSet::reference;

BitSet getNullabilities();

#endif
