#ifndef NULL_FIRST_HPP
#define NULL_FIRST_HPP

#include "src/parser/utils.hpp"

#include <vector>
#include <utility>

#include <boost/dynamic_bitset.hpp>

boost::dynamic_bitset<> getNullabilities(const GrammarData& grammarData);
std::pair<boost::dynamic_bitset<>, std::vector<boost::dynamic_bitset<>>>
getNullsAndFirsts(const GrammarData& grammarData);

#endif
