#ifndef NULL_FIRST_HPP
#define NULL_FIRST_HPP

#include "src/parser/utils.hpp"

#include <vector>
#include <utility>

std::vector<bool> getNullabilities(const GrammarData& grammarData);
std::pair<std::vector<bool>, std::vector<std::vector<bool>>>
getNullsAndFirsts(const GrammarData& grammarData);
void bitOrEquals(std::vector<bool>& bits, const std::vector<bool>& other);

#endif
