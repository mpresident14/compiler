#ifndef RULES_HPP
#define RULES_HPP

#include "dfa.hpp"
#include "slr1_grammar.hpp"

#include <cstddef>
#include <iostream>
#include <iterator>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

#include <prez/print_stuff.hpp>

using namespace std;


/***********
 *  RULES  *
 ***********/

struct Rule {
  const Concrete lhs;
  const vector<Symbol> rhs;
  const size_t pos;

  bool atEnd() const { return pos == rhs.size(); }
  /* Given a rule "S -> A.B", returns B */
  // TODO: Remove throw when done
  Symbol nextSymbol() const {
    if (pos == rhs.size()) {
      throw std::invalid_argument("Out of bounds");
    }
    return rhs[pos];
  }
  /* Given a rule "S -> A.B", returns "S -> AB." */
  Rule nextStep() const {
    if (pos == rhs.size()) {
      throw std::invalid_argument("Out of bounds");
    }
    return {lhs, rhs, pos + 1};
  }
  // TODO: Make appropriate functions noexcept and const(expr)
  bool operator==(const Rule& other) const {
    return lhs == other.lhs && rhs == other.rhs && pos == other.pos;
  }
  friend std::ostream& operator<<(std::ostream& out, const Rule& rule) {
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
  struct hash<Rule> {
    size_t operator()(const Rule& rule) const noexcept {
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
using RuleSet = unordered_set<Rule>;

namespace std {
  template <>
  struct hash<RuleSet> {
    size_t operator()(const RuleSet& ruleSet) const noexcept {
      std::hash<Rule> hasher;
      size_t h = 0;
      for (const Rule& rule : ruleSet) {
        h += hasher(rule);
      }
      return h;
    }
  };
}  // namespace std

using Grammar = unordered_map<Symbol, vector<Rule>>;


#endif
