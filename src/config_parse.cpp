#include "config_parse.hpp"
#include "config_lexer.hpp"
#include "config_grammar.hpp"

#include <vector>
#include <string_view>
#include <stdexcept>
#include <cctype>
#include <fstream>
#include <iostream>
#include <optional>

#include <prez/print_stuff.hpp>

using namespace std;
using namespace src::config_lexer;

namespace {
  string addlHdrIncludes;
  string addlCode;
  GrammarData grammarData;
  vector<Token>& tokens = grammarData.tokens;
  vector<Concrete>& concretes = grammarData.concretes;
  vector<Variable>& variables = grammarData.variables;

  string tokenToString(int tokenId) {
    return CONFIG_GRAMMAR.tokens[tokenToFromIndex(tokenId)].name;
  }
  string tokensToStrings(const vector<StackObj>& tokens) {
    vector<string> tokenNames;
    transform(
        tokens.begin(),
        tokens.end(),
        back_inserter(tokenNames),
        [](const StackObj& stackObj){ return tokenToString(stackObj.symbol); });
    stringstream s;
    s << tokenNames;
    return s.str();
  }

  bool maybeConsume(int tokenId, const vector<StackObj>& tokens, size_t pos) {
    if (tokens.size() == pos || tokens[pos].symbol != tokenId) {
      return false;
    }
    ++pos;
    return true;
  }
  void consume(int tokenId, const vector<StackObj>& tokens, size_t pos) {
    if (!maybeConsume(tokenId, tokens, pos)) {
      throw runtime_error("Parse error at: " + tokensToStrings(tokens) + ". Expected " + tokenToString(tokenId));
    }
  }
  string* maybeConsumeString(int tokenId, const vector<StackObj>& tokens, size_t pos) {
    if (tokens.size() == pos || tokens[pos].symbol != tokenId) {
      return nullptr;
    }
    return (string*) tokens[pos++].obj;
  }
  string consumeString(int tokenId, const vector<StackObj>& tokens, size_t pos) {
    string* strPtr = maybeConsumeString(tokenId, tokens, pos);
    if (!strPtr) {
      throw runtime_error("Parse error at: " + tokensToStrings(tokens) + ". Expected " + tokenToString(tokenId));
    }
    return *strPtr;
  }
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

  ifstream configFile;
  configFile.open("test/utils/expr_config.txt");
  vector<StackObj> tokens;

  try {
    tokens = tokenize(configFile);
    // tokens = tokenize("%\ncode%");
  } catch (runtime_error& e) {
    cout << e.what() << endl;
    configFile.close();
  }

  cout << tokensToStrings(tokens) << endl;

  for (auto token : tokens) {
    if (token.symbol <= -9) {
      cout << *(string*) token.obj << endl;
    }
  }



  for_each(tokens.cbegin(), tokens.cend(), deleteObj);

  return 0;
}
