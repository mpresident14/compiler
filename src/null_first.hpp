#ifndef NULL_FIRST_HPP
#define NULL_FIRST_HPP

#include "utils.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <bitset>


namespace {
  using namespace std;

  template <size_t NumVars>
  using BitVarRef = typename bitset<NumVars>::reference;

  /* Iterate to find the least fixed point */
  template <size_t NumVars>
  void computeNullabilities(bitset<NumVars>& nullabilities, vector<vector<BitVarRef<NumVars>>>* equations) {
    bool changed = true;
    while (changed) {
      changed = false;
      for (size_t i = 0; i < NumVars; ++i) {
        // If the nullability for this symbol is already true, no need to evaluate it again
        if (nullabilities[i]) {
          continue;
        }

        const vector<vector<BitVarRef<NumVars>>>& disjunctions = equations[i];
        // Nullable if any of the conjunctions evaluate to true
        bool newValue =
            any_of(disjunctions.cbegin(), disjunctions.cend(), [](const vector<BitVarRef<NumVars>>& conj) {
              return all_of(
                  conj.cbegin(), conj.cend(), [](const BitVarRef<NumVars>& bitref) { return bitref; });
            });
        if (newValue != nullabilities[i]) {
          nullabilities[i] = move(newValue);
          changed = true;
        }
      }
    }
  }

  template <size_t NumTokens>
  struct UnionEquation {
    bitset<NumTokens> tokenSet;
    vector<bitset<NumTokens>*> setRefs;
  };

  /* Iterate to find the least fixed point */
  template <size_t NumTokens>
  void computeFirsts(vector<bitset<NumTokens>>& firsts, vector<UnionEquation<NumTokens>> equations) {
    bool changed = true;
    while (changed) {
      changed = false;
      size_t numVariables = equations.size();
      for (size_t i = 0; i < numVariables; ++i) {
        const UnionEquation<NumTokens>& unionEq = equations[i];
        // Take union of all the bitsets
        bitset<NumTokens> newValue = unionEq.tokenSet;
        for (const bitset<NumTokens>* bitset : unionEq.setRefs) {
          newValue |= *bitset;
        }
        if (newValue != firsts[i]) {
          firsts[i] = move(newValue);
          changed = true;
        }
      }
    }
  }
}

/* For each symbol in the grammar, the equations for each rule on the rhs
 * are a disjunction of conjunctions, which we represent with a
 * vector<vector<BitVarRef> */
template <size_t NumVars>
std::bitset<NumVars> getNullabilities(const Grammar& grammar) {
  using namespace std;
  using BitVarRef = typename bitset<NumVars>::reference;

  bitset<NumVars> nullabilities;
  vector<vector<BitVarRef>> equations[NumVars];

  for (size_t var = 0; var < NumVars; ++var) {
    for (const GrammarRule& rule : grammar[var]) {
      // Epsilon is always nullable, so this symbol is nullable
      // NOTE: EPSILON is a special symbol we include when
      // the user leaves empty arguments, so a rule with epsilon will
      // only contain epsilon.
      if (rule.symbols[0] == EPSILON) {
        nullabilities[var] = true;
        break;
      }
      // Tokens are never nullable, so stop considering this rule
      if (std::any_of(rule.symbols.cbegin(), rule.symbols.cend(), &isToken<NumVars>)) {
        continue;
      }
      // Otherwise build the conjunction bitset
      vector<BitVarRef> conjunctions;
      for (int rhsSymbol : rule.symbols) {
        conjunctions.push_back(nullabilities[rhsSymbol]);
      }
      equations[var].push_back(move(conjunctions));
    }
  }

  computeNullabilities<NumVars>(nullabilities, equations);
  return nullabilities;
}

template <size_t NumTokens, size_t NumVars>
std::vector<std::bitset<NumTokens>> getFirsts(const Grammar& grammar) {
  using namespace std;

  vector<bitset<NumTokens>> firsts(NumVars);
  vector<UnionEquation<NumTokens>> equations(NumVars);
  auto nullabilities = getNullabilities<NumVars>(grammar);

  for (size_t var = 0; var < NumVars; ++var) {
    UnionEquation<NumTokens>& unionEq = equations[var];
    for (const GrammarRule& rule : grammar[var]) {
      for (int rhsSymbol : rule.symbols) {
        // If rule is empty, there is no first, so skip it
        if (rhsSymbol == EPSILON) {
          break;
        }
        // Tokens are never nullable, so first can only be the token
        if (isToken<NumVars>(rhsSymbol)) {
          unionEq.tokenSet[offsetToken<NumVars>(rhsSymbol)] = true;
          break;
        }

        // Add first of this variable to the equation. If the variable is nullable
        // also add the next symbol, and so forth
        unionEq.setRefs.push_back(&firsts[rhsSymbol]);
        if (!nullabilities[rhsSymbol]) {
          break;
        }
      }
    }
  }

  computeFirsts<NumTokens>(firsts, equations);
  return firsts;
}


#endif
