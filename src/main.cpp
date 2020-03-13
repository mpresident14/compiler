#include "parse.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;


int main() {
  DFA_t dfa = buildDFA();
  // cout << dfa.run({Symbol::INT, Symbol::PLUS, Symbol::INT, Symbol::DOLLAR}) << endl;
  // cout << dfa.run({Symbol::INT, Symbol::DOLLAR}) << endl;

  // cout << dfa.run({Symbol::INT}) << endl;
  // cout << dfa.run({Symbol::EXPR, Symbol::DOLLAR}) << endl;
  // cout << dfa.run({Symbol::INT, Symbol::PLUS, Symbol::INT}) << endl;
  // cout << dfa.run({Symbol::INT, Symbol::PLUS, Symbol::EXPR}) << endl;
  // cout << dfa << endl;

  cout << dfa << endl;
  return 0;
}
