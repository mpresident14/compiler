#include "regex.hpp"
#include "regex_grammar.hpp"
#include "parse.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;
// using namespace parsers;

int main() {
  DFA_t dfa = buildDFA();
  unique_ptr<Regex> r = parse(dfa,
      {StackObj{new char('a'), Symbol::CHAR, Concrete::NONE},
      StackObj{nullptr, Symbol::BAR, Concrete::NONE},
      StackObj{new char('b'), Symbol::CHAR, Concrete::NONE}});

  cout << r.get() << endl;
  return 0;
}
