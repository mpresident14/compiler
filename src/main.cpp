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
  DFA_t dfa = buildParserDFA();
  Regex* r = parse(dfa, lex("a"));
  cout << r << endl;
  delete r;
  return 0;
}
