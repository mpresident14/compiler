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
    case Assoc::LEFT:
      return out << "LEFT";
    case Assoc::RIGHT:
      return out << "RIGHT";
    case Assoc::NOT:
      return out << "NOT";
    case Assoc::NONE:
      return out << "NONE";
  }
}

static constexpr int NONE = INT_MIN;
static constexpr int EPSILON = INT_MAX;
static constexpr int S = 0;
static constexpr int SCONC = 0;

inline bool isToken(int symbol) { return symbol < 0; }
inline int tokenToFromIndex(int token) { return -token - 1; }
inline int symbolIndex(int symbol, size_t numVars) {
  return isToken(symbol) ? tokenToFromIndex(symbol) + numVars : symbol;
}
/* For use in array of both variables and tokens */
inline int indexToSymbol(size_t i, size_t numVars) {
  return i >= numVars ? numVars - i - 1 : i;
}


struct Token {
  std::string name;
  std::string type;
  int precedence;
  Assoc assoc;
  std::string ctorExpr;
  std::string dtorStmt;
  std::string regex;
};

struct Concrete {
  std::string name;
  int varType;
  int precedence;
  std::vector<int> argSymbols;
  std::string ctorExpr;
};

struct Variable {
  std::string name;
  std::string type;
  std::vector<int> concreteTypes;
  std::string dtorStmt;
};

struct GrammarData {
  std::vector<Token> tokens;
  std::vector<Concrete> concretes;
  std::vector<Variable> variables;
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


template <
    typename Fn,
    std::enable_if_t<std::is_invocable_v<Fn, std::string>, int> = 0>
void replaceNumbers(std::ostream& out, std::string_view fmt, Fn&& convertNum) {
  size_t i = 0;
  size_t len = fmt.size();
  while (i < len) {
    if (fmt[i] == '#') {
      std::string digits;
      char c;
      while (isdigit((c = fmt[++i]))) {
        digits.push_back(c);
      }
      out << convertNum(digits);
    } else {
      out << fmt[i++];
    }
  }
}

/* Replace #0, #1, etc, with vector[0], vector[1], etc */
// TODO: Special case for when vector::size < 10 so we can just convert the
// character instead of creating a string
inline void replaceNumbersVec(
    std::ostream& out,
    std::string_view fmt,
    const std::vector<std::string>& args) {
  replaceNumbers(out, fmt, [&args](const std::string& digits) {
    return args[stoi(digits)];
  });
}


#endif
