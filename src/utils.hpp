#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <vector>
#include <unordered_map>

/* Fixed for all grammars */
static constexpr int NONE = -2;
static constexpr int EPSILON = -1;

template <size_t NumVars>
inline bool isToken(int symbol) { return symbol >= (int) NumVars; }

template <size_t NumVars>
inline int offsetToken(int token) { return token - NumVars; }


struct GrammarRule {
  const int concrete;
  const std::vector<int> symbols;
};

using Grammar = std::vector<std::vector<GrammarRule>>;

#endif
