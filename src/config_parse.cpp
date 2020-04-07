#include "config_parse.hpp"
#include "config_lexer.hpp"

#include <vector>
#include <string_view>
#include <stdexcept>
#include <cctype>
#include <fstream>
#include <iostream>

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

  vector<StackObj> tokens = tokenize(R"(#header %code% %code2% "Hello, world!" "Hello, world!")");

  // vector<StackObj> tokens = tokenize(R"(
  //   "Hello, world!"
  //   )");

  for (auto& sObj : tokens) {
    cout << sObj.symbol << endl;
    if (sObj.symbol < -8) {
      cout << ((string*) sObj.obj)->size() << ": " << *(string*) sObj.obj << endl;
    }
  }

  for_each(tokens.cbegin(), tokens.cend(), deleteObj);

  return 0;
}
