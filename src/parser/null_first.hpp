#ifndef NULL_FIRST_HPP
#define NULL_FIRST_HPP

#include "src/parser/utils.hpp"

#include <vector>

std::vector<bool> getNullabilities(const GrammarData& grammarData);
std::vector<std::vector<bool>> getFirsts(const GrammarData& grammarData);

#endif
