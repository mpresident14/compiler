#ifndef UTILS_HPP
#define UTILS_HPP

#include <climits>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

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
    default:
      return out << "NONE";
  }
}

static constexpr int NONE = INT_MIN;
static constexpr int SKIP_TOKEN = INT_MIN + 1;
static constexpr int S = 0;
static constexpr int SCONC = 0;

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

constexpr bool isToken(int symbol) noexcept { return symbol < 0; }
constexpr int tokenToFromIndex(int token) noexcept { return -token - 1; }
constexpr int symbolIndex(int symbol, size_t numVars) noexcept {
  return isToken(symbol) ? tokenToFromIndex(symbol) + numVars : symbol;
}

/* For use in array of both variables and tokens */
constexpr int indexToSymbol(size_t i, size_t numVars) noexcept {
  return i >= numVars ? numVars - i - 1 : i;
}

inline std::string symbolToString(
    int symbolId,
    const GrammarData& grammarData) {
  if (isToken(symbolId)) {
    return grammarData.tokens[tokenToFromIndex(symbolId)].name;
  }
  return grammarData.variables[symbolId].name;
}

#endif
