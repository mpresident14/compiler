#ifndef RULES_HPP
#define RULES_HPP

#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <prez/print_stuff.hpp>

struct DFARule {
  Concrete lhs;
  std::vector<Symbol> rhs;
  size_t pos;
  BitSetToks lookahead;

  bool atEnd() const { return pos == rhs.size(); }
  /* Given a rule "S -> A.B", returns B */
  // TODO: Remove throw and make noexcept when done
  Symbol nextSymbol() const {
    if (pos == rhs.size()) {
      throw std::invalid_argument("Out of bounds");
    }
    return rhs[pos];
  }
  /* Given a rule "S -> A.B", returns "S -> AB." */
  DFARule nextStep() const {
    if (pos == rhs.size()) {
      throw std::invalid_argument("Out of bounds");
    }
    return {lhs, rhs, pos + 1, lookahead};
  }
  // TODO: Make appropriate functions noexcept and const(expr)
  bool operator==(const DFARule& other) const {
    return lhs == other.lhs && rhs == other.rhs && pos == other.pos && lookahead == other.lookahead;
  }
  friend std::ostream& operator<<(std::ostream& out, const DFARule& rule) {
    out << "( " << rule.lhs << " -> ";
    size_t len = rule.rhs.size();
    for (size_t i = 0; i < len; ++i) {
      if (i == rule.pos) {
        out << '.';
      }
      out << rule.rhs[i] << ' ';
    }
    if (rule.pos == len) {
      out << '.';
    }
    out << ')';
    return out;
  }
};

// TODO: Fix these hash functions
namespace std {
  template <>
  struct hash<DFARule> {
    size_t operator()(const DFARule& rule) const noexcept {
      size_t h1 = std::hash<Concrete>()(rule.lhs);
      size_t h2 = 0;
      std::hash<Symbol> hasher;
      for (const Symbol& symbol : rule.rhs) {
        h2 += hasher(symbol);
      }
      size_t h3 = std::hash<size_t>()(rule.pos);
      return h3 + h1 ^ (h2 << 1);
    }
  };
}  // namespace std

/* Nodes of the DFA. Has to be a set, not a vector, because two groups of rules
 * should be equal if they contain the same rules (in any order) */
using RuleSet = std::unordered_set<DFARule>;

namespace std {
  template <>
  struct hash<RuleSet> {
    size_t operator()(const RuleSet& ruleSet) const noexcept {
      std::hash<DFARule> hasher;
      size_t h = 0;
      for (const DFARule& rule : ruleSet) {
        h += hasher(rule);
      }
      return h;
    }
  };
}  // namespace std

struct GrammarRule {
  const Concrete lhs;
  const std::vector<Symbol> rhs;
};

using Grammar = std::unordered_map<Symbol, std::vector<GrammarRule>>;

#endif
