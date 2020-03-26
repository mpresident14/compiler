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

#include <prez/print_stuff.hpp>

namespace vis_for_test {
  void addRhses(
      std::queue<DFARule>& ruleQueue,
      const DFARule& fromRule,
      const Grammar& grammar,
      const std::vector<vector<bool>>& firsts);

  void epsilonTransition(
      DFARuleSet& ruleSet,
      const Grammar& grammar,
      const std::vector<std::vector<bool>>& firsts);

  std::vector<const DFA<DFARuleSet, int>::Node*> createTransitions(
      DFA<DFARuleSet, int>& dfa,
      const DFA<DFARuleSet, int>::Node* node,
      const Grammar& grammar,
      const std::vector<std::vector<bool>>& firsts,
      size_t numSymbols);

  DFA_t initDFA(const Grammar& grammar, const vector<BitSetToks>& firsts);
}  // namespace vis_for_test

DFA<DFARuleSet, int> buildParserDFA(const Grammar& grammar, size_t numSymbols);


#endif
