#include "null_first_follow.hpp"

#include <prez/print_stuff.hpp>

using namespace std;

/* Iterate to find the least fixed point */
void computeNullabilities(BitSet& nullabilities, vector<vector<BitRef>>* equations) {
  size_t size = nullabilities.size();
  bool changed = true;
  while (changed) {
    changed = false;
    for (size_t i = 0; i < size; ++i) {
      // If the nullability for this symbol is already true, no need to evaluate it again
      if (nullabilities[i]) {
        continue;
      }

      const vector<vector<BitRef>>& disjunctions = equations[i];
      // Nullable if any of the conjunctions evaluate to true
      bool newValue = std::any_of(
          disjunctions.cbegin(),
          disjunctions.cend(),
          [](const vector<BitRef>& conj) {
            return std::all_of(
                conj.cbegin(),
                conj.cend(),
                [](const BitRef& bitref) { return bitref; });
          });
      if (newValue != nullabilities[i]) {
        nullabilities[i] = newValue;
        changed = true;
      }
    }
  }
}


/* For each symbol in the grammar, the equations for each rule on the rhs
 * are a disjunction of conjunctions, which we represent with a
 * vector<vector<BitRef> */
BitSet getNullabilities() {
  BitSet nullabilities;
  constexpr size_t size = nullabilities.size();

  vector<vector<BitRef>> equations[size];
  for (const auto& symbolAndRules : GRAMMAR) {
    Symbol symbol = symbolAndRules.first;
    for (const Rule& rule : symbolAndRules.second) {
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
      for (const Symbol& symbol : rule.rhs) {
        conjunctions.push_back(nullabilities[static_cast<int>(symbol)]);
      }
      equations[static_cast<int>(symbol)].push_back(move(conjunctions));
    }
  }

  computeNullabilities(nullabilities, equations);
  return nullabilities;
}
