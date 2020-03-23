#include "regex_match.hpp"
#include "regex.hpp"
#include "regex_lexer.hpp"
#include "regex_grammar.hpp"
#include "parse.hpp"
#include "dfa.hpp"

#include <iostream>
#include <cstddef>
#include <string>

using namespace std;

void generateRgxDFA() {
  vector<TokenPattern> patterns = {
    { "a", Symbol::CHAR }, { "[1-9][0-9]*", Symbol::DASH }, { "for", Symbol::BAR }
  };
  writeRegexDFA(patterns, "test/gen_regex_dfa.hpp");
}

int main() {
  generateRgxDFA();

  return 0;
}
