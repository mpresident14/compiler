#ifndef RULES_HPP
#define RULES_HPP

#include "utils.hpp"

#include <cstddef>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <bitset>

#include <prez/print_stuff.hpp>


template <size_t NumTokens>
struct DFARule {
  using BitSetToks = std::bitset<NumTokens>;

  int concrete;
  std::vector<int> symbols;
  size_t pos;
  // Allows us to change lookahead while it is inside of a RuleSet, which is ok b/c
  // lookahead is not involved in the hash function or equality for RuleSets
  mutable BitSetToks lookahead;

  bool atEnd() const { return pos == symbols.size(); }
  /* Given a rule "S -> A.B", returns B */
  // TODO: Remove throw and make noexcept when done
  int nextSymbol() const { return pos == symbols.size() ? NONE : symbols[pos]; }
  int nextNextSymbol() const { return pos >= symbols.size() - 1 ? NONE : symbols[pos + 1]; }

  /* Given a rule "S -> A.B", returns "S -> AB." */
  DFARule nextStep() const {
    if (pos == symbols.size()) {
      throw std::invalid_argument("Out of bounds");
    }
    return { concrete, symbols, pos + 1, lookahead };
  }

  // TODO: Make appropriate functions noexcept and const(expr)
  bool operator==(const DFARule& other) const {
    return concrete == other.concrete && symbols == other.symbols && pos == other.pos && lookahead == other.lookahead;
  }

  friend std::ostream& operator<<(std::ostream& out, const DFARule& rule) {
    out << rule.concrete << " -> ";
    size_t len = rule.symbols.size();
    for (size_t i = 0; i < len; ++i) {
      if (i == rule.pos) {
        out << '.';
      }
      out << rule.symbols[i] << ' ';
    }
    if (rule.pos == len) {
      out << '.';
    }
    out << " :: " << rule.lookahead;
    return out;
  }
};

// TODO: Fix these hash functions
/* Does not use lookahead set. See comment below */
template <size_t NumTokens>
struct DFARuleHash {
  size_t operator()(const DFARule<NumTokens>& rule) const noexcept {
    std::hash<int> IntHasher;
    size_t h1 = IntHasher(rule.concrete);
    size_t h2 = 0;
    for (int symbol : rule.symbols) {
      h2 += IntHasher(symbol);
    }
    size_t h3 = std::hash<size_t>()(rule.pos);
    return h3 + h1 ^ (h2 << 1);
  }
};

/* Nodes of the DFA. Has to be a set, not a vector, because two groups of rules
 * should be equal if they contain the same rules (in any order).
 * Individual rules in a RuleSet are not compared using the lookahead set
 * since there should never be two rules differing only by their lookahead set
 * in the same RuleSet. However, operator== in the DFARule class still compares
 * lookahead sets because RuleSet equality compares the rules with this operator,
 * and two RuleSets can differ based on the lookahead sets of the rules they contain.
 * */
template <size_t NumTokens>
struct DFARuleEq {
  bool operator()(const DFARule<NumTokens>& left, const DFARule<NumTokens>& right) const {
    return left.concrete == right.concrete && left.symbols == right.symbols && left.pos == right.pos;
  }
};
template <size_t NumTokens>
using DFARuleSet = std::unordered_set<DFARule<NumTokens>, DFARuleHash<NumTokens>, DFARuleEq<NumTokens>>;

namespace std {
  template <size_t NumTokens>
  struct hash<DFARuleSet<NumTokens>> {
    size_t operator()(const DFARuleSet<NumTokens>& ruleSet) const noexcept {
      DFARuleHash<NumTokens> hasher;
      size_t h = 0;
      for (const DFARule<NumTokens>& rule : ruleSet) {
        h += hasher(rule);
      }
      return h;
    }
  };
}  // namespace std



#endif
