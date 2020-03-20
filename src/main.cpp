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
  unique_ptr<Regex> r1 = parse(dfa,
      {StackObj{new char('a'), Symbol::CHAR, Concrete::NONE},
      StackObj{nullptr, Symbol::BAR, Concrete::NONE},
      StackObj{new char('b'), Symbol::CHAR, Concrete::NONE},
      StackObj{nullptr, Symbol::EMPTY, Concrete::NONE},
      StackObj{new char('c'), Symbol::CHAR, Concrete::NONE}});
  cout << r1.get() << endl;

  unique_ptr<Regex> r2 = parse(dfa,
      {StackObj{new char('a'), Symbol::CHAR, Concrete::NONE},
      StackObj{nullptr, Symbol::EMPTY, Concrete::NONE},
      StackObj{new char('b'), Symbol::CHAR, Concrete::NONE},
      StackObj{nullptr, Symbol::BAR, Concrete::NONE},
      StackObj{new char('c'), Symbol::CHAR, Concrete::NONE}});
  cout << r2.get() << endl;
  return 0;
}
