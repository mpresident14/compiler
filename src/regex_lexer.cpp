#include "regex_lexer.hpp"

inline StackObj datalessObj(Symbol symbol) {
  return StackObj{ nullptr, symbol, Concrete::NONE };
}

std::vector<StackObj> lex(const std::string& input) {
  std::vector<StackObj> tokens;
  bool escaped = false;
  tokens.reserve(input.size());

  for (char c : input) {
    if (escaped) {
      tokens.push_back({ new char(c), Symbol::CHAR, Concrete::NONE });
      escaped = false;
      continue;
    }

    switch (c) {
      case '|':
        tokens.push_back(datalessObj(Symbol::BAR));
        break;
      case '*':
        tokens.push_back(datalessObj(Symbol::STAR));
        break;
      case '^':
        tokens.push_back(datalessObj(Symbol::CARET));
        break;
      case '[':
        tokens.push_back(datalessObj(Symbol::LBRACKET));
        break;
      case ']':
        tokens.push_back(datalessObj(Symbol::RBRACKET));
        break;
      case '(':
        tokens.push_back(datalessObj(Symbol::LPAREN));
        break;
      case ')':
        tokens.push_back(datalessObj(Symbol::RPAREN));
        break;
      case '-':
        tokens.push_back(datalessObj(Symbol::DASH));
        break;
      case '\\':
        escaped = true;
        break;
      default:
        tokens.push_back({ new char(c), Symbol::CHAR, Concrete::NONE });
    }
  }

  return tokens;
}
