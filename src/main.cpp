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
  unique_ptr<Regex> r = parse(dfa, lex("^a(b|c)c\\*|[a-z]"));
  cout << r.get() << endl;
  return 0;
}
