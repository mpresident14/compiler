#include "config_parse.hpp"
#include "config_lexer.hpp"
#include "config_grammar.hpp"

#include <vector>
#include <string_view>
#include <stdexcept>
#include <cctype>
#include <fstream>
#include <iostream>

#include <prez/print_stuff.hpp>

using namespace std;


namespace {
  // string addlHdrIncludes;
  // string addlCode;
  // GrammarData grammarData;
  // vector<Token>& tokens = grammarData.tokens;
  // vector<Concrete>& concretes = grammarData.concretes;
  // vector<Variable>& variables = grammarData.variables;
} // namespace



// GrammarData parseConfig(const string& fileName) {
//   ifstream config;
//   config.open(fileName);
//   string inputStr(istreambuf_iterator<char>{config}, istreambuf_iterator<char>{});
//   string_view input(inputStr);
//   config.close();
//   return grammarData;
// }

int main() {
  // parseConfig("test/expr_config.txt");
  // cout << addlHdrIncludes << endl;
  // cout << addlCode << endl;

  vector<StackObj> tokens = tokenize(R"(-> #header %code% %code2% "Hello, \"world!" "Hello, world!")");

  // vector<StackObj> tokens = tokenize(R"(
  //   "Hello, world!"
  //   )");

  vector<string> tokenNames;
  auto stkObjToName = [](StackObj stkObj) {
    return CONFIG_GRAMMAR.tokens[tokenToFromIndex(stkObj.symbol)].name;
  };
  transform(
      tokens.begin(), tokens.end(), back_inserter(tokenNames), stkObjToName);
  cout << tokenNames << endl;

  for (auto token : tokens) {
    if (token.symbol <= -9) {
      cout << *(string*) token.obj << endl;
    }
  }



  for_each(tokens.cbegin(), tokens.cend(), deleteObj);

  return 0;
}
