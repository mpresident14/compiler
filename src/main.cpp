#include "regex_grammar.hpp"
#include "regex.hpp"
#include "regex_lexer.hpp"
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
  unique_ptr<Regex> r1 = parse(dfa, lex("a|b*c"));
  cout << r1.get() << endl;

  unique_ptr<Regex> r2 = parse(dfa, lex("ab*|c"));
  cout << r2.get() << endl;

  unique_ptr<Regex> r3 = parse(dfa, lex("^ab*|[a-z]"));
  cout << r3.get() << endl;
  return 0;
}
