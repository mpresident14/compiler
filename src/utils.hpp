#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <climits>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

/* Fixed for all grammars */
enum class Assoc { LEFT, RIGHT, NOT, NONE };
inline std::ostream& operator<<(std::ostream& out, const Assoc& assoc) {
  switch (assoc) {
    case Assoc::LEFT: return out << "LEFT";
    case Assoc::RIGHT: return out << "RIGHT";
    case Assoc::NOT: return out << "NOT";
    case Assoc::NONE: return out << "NONE";
  }
}

static constexpr int NONE = INT_MIN;
static constexpr int EPSILON = INT_MAX;
static constexpr int S = 0;
static constexpr int SCONC = 0;

inline bool isToken(int symbol) { return symbol < 0; }
inline int tokensIndex(int token) { return -token - 1; }
inline int symbolIndex(int symbol, size_t numVars) {
  return isToken(symbol) ? tokensIndex(symbol) + numVars : symbol;
}
inline int indexToSymbol(size_t i, size_t numVars) {
  return i >= numVars ? numVars - i - 1 : i;
}


struct GrammarRule {
  const int concrete;
  const std::vector<int> symbols;
};
using Grammar = std::vector<std::vector<GrammarRule>>;

struct GrammarData {
  std::vector<int> concToSymb;     // Indexed by Concrete
  std::vector<int> overridePrecs;  // Indexed by Concrete
  std::vector<int> tokenPrecs;     // Indexed by Token
  std::vector<Assoc> tokenAssoc;   // Indexed by Token
};

inline void bitOrEquals(
    std::vector<bool>& bits,
    const std::vector<bool>& other) {
  size_t len = bits.size();
  for (size_t i = 0; i < len; ++i) {
    bits[i] = bits[i] | other[i];
  }
}

inline std::vector<bool> bitOr(
    const std::vector<bool>& lhs,
    const std::vector<bool>& rhs) {
  size_t len = lhs.size();
  std::vector<bool> result(len);
  for (size_t i = 0; i < len; ++i) {
    result[i] = lhs[i] | rhs[i];
  }
  return result;
}

template<size_t I, typename Tup, std::enable_if_t<I == std::tuple_size_v<Tup>, int> = 0>
void replaceStrs(std::ostream& out, std::string_view fmt, const Tup&) {
  out << fmt;
}

template<size_t I, typename Tup, std::enable_if_t<I != std::tuple_size_v<Tup>, int> = 0>
void replaceStrs(std::ostream& out, std::string_view fmt, const Tup& args) {
  size_t i = 0;
  size_t len = fmt.size();
  while (i < len) {
    if (fmt[i] == '#') {
      out << std::get<I>(args);
      return replaceStrs<I+1>(out, fmt.substr(i+1), args);
    } else {
      out << fmt[i++];
    }
  }
}

/* Replace #0, #1, etc, with vector[0], vector[1], etc */
template<typename... Args>
void replaceStrs(std::ostream& out, std::string_view fmt, const Args&... args) {
  replaceStrs<0>(out, fmt, std::make_tuple(args...));
}

#endif
