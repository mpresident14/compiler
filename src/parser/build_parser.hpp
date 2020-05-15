#ifndef BUILD_PARSER_HPP
#define BUILD_PARSER_HPP

#include "src/parser/dfa.hpp"
#include "src/parser/rules.hpp"
#include "src/parser/utils.hpp"

#include <optional>
#include <ostream>

struct RuleData {
  std::optional<DFARule> reducibleRule;
  int precedence;

  bool operator==(const RuleData rhs) const noexcept {
    return reducibleRule == rhs.reducibleRule;
  }
};

DFA<DFARuleSet, int, DFARuleSetHash> buildParserDFA(
    const GrammarData& grammarData);
RuleData condenseRuleSet(
    const DFARuleSet& ruleSet,
    const GrammarData& grammarData);
void condensedDFAToCode(std::ostream& out, const GrammarData& grammarData);

#endif
