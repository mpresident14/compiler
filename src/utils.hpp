#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <vector>
#include <unordered_map>

/* Fixed for all grammars */
static constexpr int NONE = -2;
static constexpr int EPSILON = -1;

inline bool isToken(int symbol, size_t numVars) { return symbol >= (int)numVars; }
inline int offsetToken(int token, size_t numVars) { return token - numVars; }


struct GrammarRule {
  const int concrete;
  const std::vector<int> symbols;
};
using Grammar = std::vector<std::vector<GrammarRule>>;

inline void bitOr(std::vector<bool>& bits, const std::vector<bool>& other) {
  size_t len = bits.size();
  for (size_t i = 0; i < len; ++i) {
    bits[i] = bits[i] | other[i];
  }
}

#endif
