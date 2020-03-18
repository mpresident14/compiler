#include "null_first_follow.hpp"

#include <iostream>
#include <cstddef>
#include <string>
#include <bitset>

using namespace std;


using BitSet = std::bitset<static_cast<int>(Symbol::STARTTOKENS)>;
using BitSetRef = BitSet::reference;

/* Returns vector of pointers to nullability values of symbols in the rule.
 * If nullability of rule is always true or always false, returns an empty
 * vector and sets always accordingly. */
vector<BitSetRef> conjunction(const Rule& rule, BitSet nullabilities, bool* always){
  vector<BitSetRef> result;
  for (const Symbol& symbol : rule.rhs) {
    // Tokens are never nullable
    if (isToken(symbol)) {
      *always = false;
      return {};
    }

    // Epsilon is always nullable
    if (symbol == Symbol::EPSILON) {
      *always = true;
      return {};
    }

    result.push_back(nullabilities[static_cast<int>(symbol)]);
  }
  return result;
}


void initNull() {
  BitSet nullabilities;
}
