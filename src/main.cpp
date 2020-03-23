#include "regex_grammar.hpp"
#include "regex.hpp"
#include "regex_lexer.hpp"
#include "parse.hpp"
#include "regex_match.hpp"
#include "dfa.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;
// using namespace parsers;

int main() {
  vector<TokenPattern> patterns = {
    { "a", Symbol::CHAR }, { "[1-9][0-9]*", Symbol::DASH }, { "for", Symbol::BAR }
  };


  writeRegexDFA(patterns);
  return 0;
}
