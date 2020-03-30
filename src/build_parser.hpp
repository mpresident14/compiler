#ifndef BUILD_PARSER_HPP
#define BUILD_PARSER_HPP

#include "utils.hpp"
#include "dfa.hpp"
#include "rules.hpp"
#include "null_first.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <memory>
#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <optional>
#include <sstream>

#include <prez/print_stuff.hpp>


struct RuleData {
  std::optional<DFARule> reducibleRule;
  int precedence;
  Assoc assoc;

  bool operator==(const RuleData rhs) const noexcept {
    return reducibleRule == rhs.reducibleRule;
  }

  // inline ostream& operator<<(ostream& out, const RuleData& ruleData) {
  //   if (!ruleData.reducibleRule.has_value()) {
  //     return out << "{}";
  //   }
  //   return out << *ruleData.reducibleRule;
  // }
};

// TODO: Fix these hash functions
namespace std {
  template <>
  struct hash<RuleData> {
    size_t operator()(const RuleData& ruleData) const noexcept {
      if (!ruleData.reducibleRule.has_value()) {
        return 0;
      }
      return DFARuleHash()(*ruleData.reducibleRule);
    }
  };
}  // namespace std

using CondensedDFA = DFA<RuleData, int>;
using CondensedNode = CondensedDFA::Node;

DFA<DFARuleSet, int> buildParserDFA(const GrammarData& grammarData);
RuleData condenseRuleSet(const DFARuleSet& ruleSet, const GrammarData& grammarData);
void condensedDFAToCode(std::ostream& out, const GrammarData& grammarData);

#endif
