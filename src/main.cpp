#include "nfa.hpp"
#include "rules.hpp"

#include <cstddef>
#include <iostream>
#include <string>

using namespace std;

int main() {
  NFA_t nfa = buildNFA();
  cout << nfa.run({Symbol::INT, Symbol::PLUS, Symbol::INT, Symbol::DOLLAR}) << endl;
  cout << nfa.run({Symbol::INT, Symbol::DOLLAR}) << endl;

  cout << nfa.run({Symbol::INT}) << endl;
  cout << nfa.run({Symbol::EXPR, Symbol::DOLLAR}) << endl;
  cout << nfa.run({Symbol::INT, Symbol::PLUS, Symbol::INT}) << endl;
  cout << nfa.run({Symbol::INT, Symbol::PLUS, Symbol::EXPR}) << endl;
  cout << nfa << endl;
  return 0;
}
