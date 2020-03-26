#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <climits>

/* Fixed for all grammars */
static constexpr int NONE = INT_MAX;
static constexpr int EPSILON = INT_MAX - 1;
static constexpr int S = 0;
static constexpr int SCONC = 0;
static constexpr int ROOT_SYMBOL = 1;

inline bool isToken(int symbol) { return symbol < 0; }
inline int tokensIndex(int token) { return -token - 1; }
inline int symbolIndex(int symbol, size_t numVars) {
  return isToken(symbol) ? tokensIndex(symbol) + numVars : symbol;
}
inline int indexToSymbol(size_t i, size_t numVars) { return i >= numVars ? numVars - i - 1 : i; }


struct GrammarRule {
  const int concrete;
  const std::vector<int> symbols;
};
using Grammar = std::vector<std::vector<GrammarRule>>;

inline void bitOrEquals(std::vector<bool>& bits, const std::vector<bool>& other) {
  size_t len = bits.size();
  for (size_t i = 0; i < len; ++i) {
    bits[i] = bits[i] | other[i];
  }
}

inline std::vector<bool> bitOr(const std::vector<bool>& lhs, const std::vector<bool>& rhs) {
  size_t len = lhs.size();
  std::vector<bool> result(len);
  for (size_t i = 0; i < len; ++i) {
    result[i] = lhs[i] | rhs[i];
  }
  return result;
}

#endif
