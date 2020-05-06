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
#include <cstdint>
#include <stdexcept>
#include <sstream>

#include <prez/print_stuff.hpp>


/* Fixed for all grammars */
enum class Assoc { NONE, LEFT, RIGHT, NOT };
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
static constexpr int SKIP_TOKEN = INT_MIN + 1;
static constexpr int S = 0;
static constexpr int SCONC = 0;

constexpr bool isToken(int symbol) noexcept { return symbol < 0; }
constexpr int tokenToFromIndex(int token) noexcept { return -token - 1; }
constexpr int symbolIndex(int symbol, size_t numVars) noexcept {
  return isToken(symbol) ? tokenToFromIndex(symbol) + numVars : symbol;
}
/* For use in array of both variables and tokens */
constexpr int indexToSymbol(size_t i, size_t numVars) noexcept {
  return i >= numVars ? numVars - i - 1 : i;
}

struct Token {
  std::string name;
  std::string type;
  int precedence = NONE;
  Assoc assoc = Assoc::NONE;
  std::string ctorExpr;
  std::string dtorStmt;
  std::string regex;
};


struct Concrete {
  std::string name;
  int varType;
  int precedence = NONE;
  // See parseGrammarDef() in config_parse.cpp for why this is intptr_t
  std::vector<intptr_t> argSymbols;
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


inline std::string symbolToString(int symbolId, const GrammarData& grammarData) {
  if (isToken(symbolId)) {
    return grammarData.tokens[tokenToFromIndex(symbolId)].name;
  }
  return grammarData.variables[symbolId].name;
}


inline std::string symbolsToStrings(const std::vector<intptr_t>& symbols, const GrammarData& grammarData) {
  std::vector<std::string> symbolNames;
  transform(
      symbols.begin(),
      symbols.end(),
      back_inserter(symbolNames),
      [&grammarData](int symbol){
        return symbolToString(symbol, grammarData);
      });
  std::stringstream s;
  s << symbolNames;
  return s.str();
}


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


template<typename T>
void checkBounds(int i, const std::vector<T>& vec) {
  if (i < 0) {
    throw std::runtime_error("Index " + std::to_string(i) + " is < 0.");
  }
  if ((size_t) i >= vec.size()) {
    std::stringstream err;
    err << "Index " << std::to_string(i) << " is greater than the number of elements in "
        << vec;
    throw std::runtime_error(err.str());
  }
}


#endif
