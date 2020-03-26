#include "null_first.hpp"

namespace {
  using namespace std;
  using BitSetVars = vector<bool>;
  using BitSetToks = vector<bool>;
  using BitRef = typename vector<bool>::reference;

  /* Iterate to find the least fixed point */
  void computeNullabilities(
      BitSetVars& nullabilities,
      const vector<vector<vector<BitRef>>>& equations) {
    bool changed = true;
    size_t numVars = nullabilities.size();
    while (changed) {
      changed = false;
      for (size_t i = 0; i < numVars; ++i) {
        // If the nullability for this symbol is already true, no need to evaluate it again
        if (nullabilities[i]) {
          continue;
        }

        const vector<vector<BitRef>>& disjunctions = equations[i];
        // Nullable if any of the conjunctions evaluate to true
        bool newValue =
            any_of(disjunctions.cbegin(), disjunctions.cend(), [](const vector<BitRef>& conj) {
              return all_of(
                  conj.cbegin(), conj.cend(), [](const BitRef& bitref) { return bitref; });
            });
        if (newValue != nullabilities[i]) {
          nullabilities[i] = move(newValue);
          changed = true;
        }
      }
    }
  }

  struct UnionEquation {
    UnionEquation(size_t numTokens) : tokenSet(numTokens) {}
    BitSetToks tokenSet;
    vector<BitSetToks*> bitSetTokRefs;
  };

  /* Iterate to find the least fixed point */
  void computeFirsts(vector<BitSetToks>& firsts, const vector<UnionEquation>& equations) {
    bool changed = true;
    while (changed) {
      changed = false;
      size_t numVariables = equations.size();
      for (size_t i = 0; i < numVariables; ++i) {
        const UnionEquation& unionEq = equations[i];
        // Take union of all the bitsets
        BitSetToks newValue = unionEq.tokenSet;
        for (const BitSetToks* bitset : unionEq.bitSetTokRefs) {
          bitOrEquals(newValue, *bitset);
        }
        if (newValue != firsts[i]) {
          firsts[i] = move(newValue);
          changed = true;
        }
      }
    }
  }
}  // namespace


/* For each symbol in the grammar, the equations for each rule on the rhs
 * are a disjunction of conjunctions, which we represent with a
 * vector<vector<BitVarRef> */
std::vector<bool> getNullabilities(const Grammar& grammar) {
  using namespace std;
  using BitSetVars = vector<bool>;
  using BitRef = typename vector<bool>::reference;

  size_t numVars = grammar.size();
  BitSetVars nullabilities(numVars);
  vector<vector<vector<BitRef>>> equations(numVars);

  for (size_t var = 0; var < numVars; ++var) {
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
      if (std::any_of(rule.symbols.cbegin(), rule.symbols.cend(), isToken)) {
        continue;
      }
      // Otherwise build the conjunction bitset
      vector<BitRef> conjunctions;
      for (int rhsSymbol : rule.symbols) {
        conjunctions.push_back(nullabilities[rhsSymbol]);
      }
      equations[var].push_back(move(conjunctions));
    }
  }

  computeNullabilities(nullabilities, equations);
  return nullabilities;
}

std::vector<std::vector<bool>> getFirsts(const Grammar& grammar, size_t numTokens) {
  using namespace std;
  using BitSetVars = vector<bool>;
  using BitSetToks = vector<bool>;

  size_t numVars = grammar.size();

  // Initialize firsts with empty bitset vectors
  vector<BitSetToks> firsts;
  firsts.reserve(numVars);
  for (size_t i = 0; i < numVars; ++i) {
    firsts.push_back(BitSetToks(numTokens));
  }

  // Initialize equations with empty bitset vectors
  vector<UnionEquation> equations;
  equations.reserve(numVars);
  for (size_t i = 0; i < numVars; ++i) {
    equations.push_back(UnionEquation(numTokens));
  }

  BitSetVars nullabilities = getNullabilities(grammar);

  for (size_t var = 0; var < numVars; ++var) {
    UnionEquation& unionEq = equations[var];
    for (const GrammarRule& rule : grammar[var]) {
      for (int rhsSymbol : rule.symbols) {
        // If rule is empty, there is no first, so skip it
        if (rhsSymbol == EPSILON) {
          break;
        }
        // Tokens are never nullable, so first can only be the token
        if (isToken(rhsSymbol)) {
          unionEq.tokenSet[tokensIndex(rhsSymbol)] = true;
          break;
        }

        // Add first of this variable to the equation. If the variable is nullable
        // also add the next symbol, and so forth
        unionEq.bitSetTokRefs.push_back(&firsts[rhsSymbol]);
        if (!nullabilities[rhsSymbol]) {
          break;
        }
      }
    }
  }

  computeFirsts(firsts, equations);
  return firsts;
}
