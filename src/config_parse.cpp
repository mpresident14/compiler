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
#include <unordered_map>
#include <cstdint>

#include <prez/print_stuff.hpp>

using namespace std;
using namespace src::config_lexer;

namespace {
  string addlHdrIncludes;
  string addlCode;
  GrammarData grammarData;
  vector<Token>& gdTokens = grammarData.tokens;
  vector<Concrete>& gdConcretes = grammarData.concretes;
  vector<Variable>& gdVariables = grammarData.variables;

  unordered_map<string, size_t> tokenNameToIndex;
  unordered_map<string, size_t> varNameToIndex;

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


  /*******************
   * CONSUME HELPERS *
   *******************/
  bool maybeConsume(int tokenId, const vector<StackObj>& tokens, size_t& pos) {
    if (tokens.size() == pos || tokens[pos].symbol != tokenId) {
      return false;
    }
    ++pos;
    return true;
  }
  void consume(int tokenId, const vector<StackObj>& tokens, size_t& pos) {
    if (!maybeConsume(tokenId, tokens, pos)) {
      throw runtime_error("Parse error at: " + tokensToStrings(tokens) + ". Expected " + tokenToString(tokenId));
    }
  }
  string* maybeConsumeString(int tokenId, const vector<StackObj>& tokens, size_t& pos) {
    if (tokens.size() == pos || tokens[pos].symbol != tokenId) {
      return nullptr;
    }
    return (string*) tokens[pos++].obj;
  }
  string consumeString(int tokenId, const vector<StackObj>& tokens, size_t& pos) {
    string* strPtr = maybeConsumeString(tokenId, tokens, pos);
    if (!strPtr) {
      throw runtime_error("Parse error at: " + tokensToStrings(tokens) + ". Expected " + tokenToString(tokenId));
    }
    return *strPtr;
  }


  // TODO: move() all strings from tokens instead of copying
  // TODO: Use a class so we don't have to pass around tokens and pos everywhere
  // TODO: Need to check to make sure things aren't empty,
  // e.g., at least one token and grammar variable

  void parseHeader(const vector<StackObj>& tokens, size_t& pos) {
    consume(HEADER, tokens, pos);
    addlHdrIncludes = consumeString(CODE, tokens, pos);
  }

  void parseSource(const vector<StackObj>& tokens, size_t& pos) {
    consume(SOURCE, tokens, pos);
    addlCode = consumeString(CODE, tokens, pos);
  }

  bool maybeParseToken(const vector<StackObj>& tokens, size_t& pos) {
    // Check for an identifier to see if there are any more tokens
    string* name = maybeConsumeString(IDENT, tokens, pos);
    if (!name) {
      return false;
    }

    gdTokens.push_back(Token());
    Token& gdToken = gdTokens.back();
    gdToken.name = *name;
    // Keep track of the index for this name so that we can use it when parsing #prec
    tokenNameToIndex.emplace(*name, gdTokens.size() - 1);
    gdToken.regex = consumeString(STRLIT, tokens, pos);
    // An arrow signifies that the token holds data
    if (maybeConsume(ARROW, tokens, pos)) {
      gdToken.type = consumeString(IDENT, tokens, pos);
      gdToken.ctorExpr = consumeString(CODE, tokens, pos);
      gdToken.dtorStmt = consumeString(CODE, tokens, pos);
    }
    return true;
  }

  void parseTokens(const vector<StackObj>& tokens, size_t& pos) {
    consume(TOKENS, tokens, pos);
    while (maybeParseToken(tokens, pos));
  }

  bool maybeParsePrec(const vector<StackObj>& tokens, size_t& pos, size_t prec) {
    // Check for an identifier to see if there are any more precedence lines
    string* name = maybeConsumeString(IDENT, tokens, pos);
    if (!name) {
      return false;
    }

    // Look up the index of the token in the map
    auto iter = tokenNameToIndex.find(*name);
    if (iter == tokenNameToIndex.end()) {
      throw runtime_error("Unknown token " + *name);
    }

    Token& gdToken = gdTokens[iter->second];
    gdToken.precedence = prec;
    // Figure out the associativity (which is required)
    if (maybeConsume(LEFTASSOC, tokens, pos)) {
      gdToken.assoc = Assoc::LEFT;
    } else if (maybeConsume(RIGHTASSOC, tokens, pos)) {
      gdToken.assoc = Assoc::RIGHT;
    } else {
      consume(NONASSOC, tokens, pos);
      gdToken.assoc = Assoc::NOT;
    }
    return true;
  }

  void parsePrecs(const vector<StackObj>& tokens, size_t& pos) {
    consume(PREC, tokens, pos);
    size_t prec = 1;
    while (maybeParsePrec(tokens, pos, prec++));
  }


  bool maybeParseGrammarDecl(const vector<StackObj>& tokens, size_t& pos) {
    // Check for an identifier to see if there are any more declarations
    string* name = maybeConsumeString(IDENT, tokens, pos);
    if (!name) {
      return false;
    }

    gdVariables.push_back(Variable());
    Variable& gdVariable = gdVariables.back();
    gdVariable.name = *name;
    varNameToIndex.emplace(*name, gdVariables.size() - 1);
    consume(ARROW, tokens, pos);
    gdVariable.type = consumeString(IDENT, tokens, pos);
    string* dtor = maybeConsumeString(CODE, tokens, pos);
    if (dtor) {
      gdVariable.dtorStmt = *dtor;
    }
    return true;
  }

  /* This must be called right after parseGrammarDecl() */
  void parseGrammarDef(const vector<StackObj>& tokens, size_t& pos, size_t concNum) {
    Variable& gdVariable = gdVariables.back();
    gdConcretes.push_back(Concrete());
    Concrete& gdConcrete = gdConcretes.back();
    gdVariable.concreteTypes.push_back(gdConcretes.size() - 1);
    gdConcrete.name = gdVariable.name + to_string(concNum);
    gdConcrete.varType = gdVariables.size() - 1;
    // Store argSymbols as string*s for now until we have seen all the
    // variables. Then, we will convert them to correct integral values.
    string* conc;
    while ((conc = maybeConsumeString(IDENT, tokens, pos))) {
      gdConcrete.argSymbols.push_back((intptr_t) conc);
    }

    if (maybeConsume(PREC, tokens, pos)) {
      string tokenName = consumeString(IDENT, tokens, pos);
      // Look up the index of the token in the map
      auto iter = tokenNameToIndex.find(tokenName);
      if (iter == tokenNameToIndex.end()) {
        throw runtime_error("Unknown token " + tokenName);
      }
      const Token& token = gdTokens[iter->second];
      if (token.precedence == NONE) {
        cerr << "WARNING: Token " << token.name << " is used to override a rule's precedence, "
            "but has no precedence set." << endl;
      }
      gdConcrete.precedence = gdTokens[iter->second].precedence;
    }

    gdConcrete.ctorExpr = consumeString(CODE, tokens, pos);
  }

  bool maybeParseGrammarVar(const vector<StackObj>& tokens, size_t& pos) {
    if (!maybeParseGrammarDecl(tokens, pos)) {
      return false;
    }

    consume(DEFINED, tokens, pos);
    size_t concNum = 0;
    parseGrammarDef(tokens, pos, concNum++);
    while (maybeConsume(BAR, tokens, pos)) {
      parseGrammarDef(tokens, pos, concNum++);
    }
    return true;
  }


  void parseGrammar(const vector<StackObj>& tokens, size_t& pos) {
    gdVariables.push_back(Variable{ "S", "Start", { SCONC }, "" });
    gdConcretes.push_back(Concrete{ "SCONC", S, NONE, {1}, "Start(#0)" });
    consume(GRAMMAR, tokens, pos);
    while (maybeParseGrammarVar(tokens, pos));

    // Translate the string pointers to token/variable ids now that we
    // have parsed the whole file (skip SCONC)
    size_t numConcs = gdConcretes.size();
    for (size_t i = 1; i < numConcs; ++i) {
      Concrete& concrete = gdConcretes[i];
      size_t len = concrete.argSymbols.size();
      for (size_t j = 0; j < len; ++j) {
        string symbolName = *(string*) concrete.argSymbols[j];
        // Check if it is a token first
        auto tokIter = tokenNameToIndex.find(symbolName);
        if (tokIter == tokenNameToIndex.end()) {
          // Otherwise, check if it is a variable
          auto varIter = varNameToIndex.find(symbolName);
          if (varIter == varNameToIndex.end()) {
            throw runtime_error("Unknown symbol " + symbolName);
          } else {
            concrete.argSymbols[j] = varIter->second;
          }
        } else {
          concrete.argSymbols[j] = tokenToFromIndex(tokIter->second);
        }
      }
    }
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

  ifstream configFile("test/utils/expr_config.txt");
  vector<StackObj> tokens;

  try {
    tokens = tokenize(configFile);
  } catch (runtime_error& e) {
    cout << e.what() << endl;
  }

  size_t pos = 0;
  parseHeader(tokens, pos);
  parseSource(tokens, pos);
  parseTokens(tokens, pos);
  parsePrecs(tokens, pos);
  parseGrammar(tokens, pos);
  cout << addlHdrIncludes << endl;
  cout << addlCode << endl;
  for (const Token& tok : gdTokens) {
    cout << tok.name << endl;
    cout << tok.regex << endl;
    cout << tok.type << endl;
    cout << tok.ctorExpr << endl;
    cout << tok.dtorStmt << endl;
    cout << tok.precedence << endl;
    cout << tok.assoc << endl;
  }
  for (const Variable& var : gdVariables) {
    cout << var.name << endl;
    cout << var.type << endl;
    cout << var.concreteTypes << endl;
    cout << var.dtorStmt << endl;
  }
  for (const Concrete& conc : gdConcretes) {
    cout << conc.name << endl;
    cout << conc.varType << endl;
    cout << conc.precedence << endl;
    cout << conc.argSymbols << endl;
    cout << conc.ctorExpr << endl;
  }


  for_each(tokens.cbegin(), tokens.cend(), deleteObj);

  return 0;
}
