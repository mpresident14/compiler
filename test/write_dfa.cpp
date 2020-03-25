#include "regex_match.hpp"

#include <iostream>
#include <cstddef>
#include <string>

using namespace std;

/* See test/tokenize_grammar.hpp */
void generateRgxDFA() {
  vector<TokenPattern> patterns = { { "a", 1 /* CHAR */},
                                    { "for", 2 /* FOR */},
                                    { "while", 3 /* WHILE */},
                                    { "[1-9][0-9]*", 4 /* INT */},
                                    { "([a-z]|[A-Z])([a-z]|[A-Z])*", 5 /* IDENTIFIER */} };
  writeRegexDFA(patterns, "test/tokenize_grammar.hpp", "test/gen_regex_dfa.hpp");
}

int main() {
  generateRgxDFA();

  return 0;
}
