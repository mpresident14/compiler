#ifndef BUILD_PARSER_HPP
#define BUILD_PARSER_HPP

#include "rules.hpp"
#include "utils.hpp"
#include "dfa.hpp"

#include <optional>
#include <ostream>


struct RuleData {
  std::optional<DFARule> reducibleRule;
  int precedence;
  Assoc assoc;

  bool operator==(const RuleData rhs) const noexcept {
    return reducibleRule == rhs.reducibleRule;
  }
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


DFA<DFARuleSet, int> buildParserDFA(const GrammarData& grammarData);
RuleData condenseRuleSet(
    const DFARuleSet& ruleSet,
    const GrammarData& grammarData);
void condensedDFAToCode(std::ostream& out, const GrammarData& grammarData);

#endif
