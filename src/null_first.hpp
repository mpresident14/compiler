#ifndef NULL_FIRST_HPP
#define NULL_FIRST_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <string>

namespace {
  /* Iterate to find the least fixed point */
  void computeNullabilities(BitSetVars& nullabilities, std::vector<std::vector<BitRef>>* equations) {
    using namespace std;

    bool changed = true;
    while (changed) {
      changed = false;
      for (size_t i = 0; i < numVariables; ++i) {
        // If the nullability for this symbol is already true, no need to evaluate it again
        if (nullabilities[i]) {
          continue;
        }

        const vector<vector<BitRef>>& disjunctions = equations[i];
        // Nullable if any of the conjunctions evaluate to true
        bool newValue =
            std::any_of(disjunctions.cbegin(), disjunctions.cend(), [](const vector<BitRef>& conj) {
              return std::all_of(
                  conj.cbegin(), conj.cend(), [](const BitRef& bitref) { return bitref; });
            });
        if (newValue != nullabilities[i]) {
          nullabilities[i] = move(newValue);
          changed = true;
        }
      }
    }
  }
}

/* For each symbol in the grammar, the equations for each rule on the rhs
 * are a disjunction of conjunctions, which we represent with a
 * vector<vector<BitRef> */
inline BitSetVars getNullabilities() {
  using namespace std;

  BitSetVars nullabilities;
  vector<vector<BitRef>> equations[numVariables];

  for (const auto& symbolAndRules : GRAMMAR) {
    Symbol symbol = symbolAndRules.first;
    for (const GrammarRule& rule : symbolAndRules.second) {
      // Epsilon is always nullable, so this symbol is nullable
      // NOTE: Symbol::EPSILON is a special symbol we include when
      // the user leaves empty arguments, so a rule with epsilon will
      // only contain epsilon.
      if (rule.rhs[0] == Symbol::EPSILON) {
        nullabilities[static_cast<int>(symbol)] = true;
        break;
      }
      // Tokens are never nullable, so stop considering this rule
      if (std::any_of(rule.rhs.cbegin(), rule.rhs.cend(), isToken)) {
        continue;
      }
      // Otherwise build the conjunction bitset
      vector<BitRef> conjunctions;
      for (const Symbol& rhsSymbol : rule.rhs) {
        conjunctions.push_back(nullabilities[static_cast<int>(rhsSymbol)]);
      }
      equations[static_cast<int>(symbol)].push_back(move(conjunctions));
    }
  }

  computeNullabilities(nullabilities, equations);
  return nullabilities;
}

namespace {
  struct UnionEquation {
    BitSetToks tokenSet;
    std::vector<BitSetToks*> setRefs;
  };

  /* Iterate to find the least fixed point */
  void computeFirsts(std::vector<BitSetToks>& firsts, UnionEquation* equations) {
    using namespace std;

    bool changed = true;
    while (changed) {
      changed = false;
      for (size_t i = 0; i < numVariables; ++i) {
        const UnionEquation& unionEq = equations[i];
        // Take union of all the bitsets
        BitSetToks newValue = unionEq.tokenSet;
        for (const BitSetToks* bitset : unionEq.setRefs) {
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

inline std::vector<BitSetToks> getFirsts() {
  using namespace std;

  vector<BitSetToks> firsts(numVariables);
  UnionEquation equations[numVariables];
  BitSetVars nullabilities = getNullabilities();

  for (const auto& symbolAndRules : GRAMMAR) {
    Symbol symbol = symbolAndRules.first;
    UnionEquation& unionEq = equations[toInt(symbol)];
    for (const GrammarRule& rule : symbolAndRules.second) {
      for (const Symbol& rhsSymbol : rule.rhs) {
        // If rule is empty, there is no first, so skip it
        if (rhsSymbol == Symbol::EPSILON) {
          break;
        }
        // Tokens are never nullable, so first can only be the token
        if (isToken(rhsSymbol)) {
          unionEq.tokenSet[toIntTokenOffset(rhsSymbol)] = true;
          break;
        }

        // Add first of this variable to the equation. If the variable is nullable
        // also add the next symbol, and so forth
        unionEq.setRefs.push_back(&firsts[toInt(rhsSymbol)]);
        if (!nullabilities[toInt(rhsSymbol)]) {
          break;
        }
      }
    }
  }

  computeFirsts(firsts, equations);
  return firsts;
}


#endif
