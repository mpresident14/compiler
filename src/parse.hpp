#ifndef PARSE_HPP
#define PARSE_HPP

#include "dfa.hpp"
#include "lr0_grammar.hpp"
// #include "slr1_grammar.hpp"

#include <cstddef>
#include <memory>
#include <queue>
#include <unordered_set>
#include <vector>
#include <bitset>

#include <prez/print_stuff.hpp>


// NOTE: Starting point of the grammar must have a special name ROOT_SYM so that we
// know where to start building the DFA from.

/**********************
 *  DFA CONSTRUCTION  *
 **********************/
using DFA_t = DFA<RuleSet, Symbol>;

/* Adds all "symbol -> .rhs" rules to rule list */
void addRhses(RuleSet& ruleSet, Symbol symbol);
void addRhses(std::queue<Rule>& ruleQueue, Symbol symbol);

/* Adds possible rules to node's state via epsilon transition in DFA.
 * Ex: S -> A.B, then add all rules B -> ??? */
void epsilonTransition(RuleSet& ruleSet);

/* For each rule of this node, construct the transitions to successors. */
std::vector<const DFA_t::Node*> createTransitions(DFA_t& dfa, const DFA_t::Node* node);

/* Constructs the starting node of the DFA */
DFA_t initDFA();

// TODO: We shouldn't have to build the DFA every time someone calls parse().
// Instead, we should just hard code all the nodes after building the first time
DFA_t buildDFA();

/**********************
 *   SHIFT-REDUCING   *
 **********************/

struct StackObj {
  // No deleter since pointers will be passed to the client
  void* obj;
  Symbol symbol;
  Concrete type;

  void deleteObj() const;
};

std::unique_ptr<ROOT_TYPE> parse(const DFA_t& dfa, const std::vector<StackObj>& inputTokens);

#endif
