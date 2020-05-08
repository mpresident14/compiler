#include "src/parser/config_parse.hpp"
#include "src/parser/config_lexer.hpp"
#include "src/parser/config_grammar.hpp"

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
using namespace src::parser::config_lexer;

// TODO: Use consistent signed/unsigned vars for prec, varType, symbol, etc.


namespace {
  string addlHppCode;
  string addlCppCode;
  GrammarData grammarData;
  vector<Token>& gdTokens = grammarData.tokens;
  vector<Concrete>& gdConcretes = grammarData.concretes;
  vector<Variable>& gdVariables = grammarData.variables;

  unordered_map<string, size_t> tokenNameToIndex;
  unordered_map<string, size_t> varNameToIndex;
  unordered_map<string, int> precNameToPrec;

  void streamTokenNames(ostream& out, const vector<StackObj>& tokens) {
    vector<string> tokenNames;
    transform(
        tokens.begin(),
        tokens.end(),
        back_inserter(tokenNames),
        [](const StackObj& stackObj){ return symbolToString(stackObj.getSymbol(), CONFIG_GRAMMAR); });
    out << tokenNames;
  }


  /*******************
   * CONSUME HELPERS *
   *******************/
  void parseError(int tokenId, const vector<StackObj>& tokens, size_t pos) {
    stringstream errMsg;
    errMsg << "Parse error on line " << tokens[pos].getLine() << " before tokens: ";
    streamTokenNames(errMsg, tokens);
    errMsg << ".\nExpected " + symbolToString(tokenId, CONFIG_GRAMMAR);
    throw runtime_error(errMsg.str());
  }

  bool maybeConsume(int tokenId, const vector<StackObj>& tokens, size_t& pos) {
    if (tokens.size() == pos || tokens[pos].getSymbol() != tokenId) {
      return false;
    }
    ++pos;
    return true;
  }

  void consume(int tokenId, const vector<StackObj>& tokens, size_t& pos) {
    if (!maybeConsume(tokenId, tokens, pos)) {
      parseError(tokenId, tokens, pos);
    }
  }

  string* maybeConsumeString(int tokenId, vector<StackObj>& tokens, size_t& pos) {
    if (tokens.size() == pos || tokens[pos].getSymbol() != tokenId) {
      return nullptr;
    }
    return (string*) tokens[pos++].getObj();
  }

  string consumeString(int tokenId, vector<StackObj>& tokens, size_t& pos) {
    string* strPtr = maybeConsumeString(tokenId, tokens, pos);
    if (!strPtr) {
      parseError(tokenId, tokens, pos);
    }
    return *strPtr;
  }


  // TODO: move() all strings from tokens instead of copying
  // TODO: Use a class so we don't have to pass around tokens and pos everywhere
  // TODO: Need to check to make sure things aren't empty,
  // e.g., at least one token and grammar variable

  void parseHeader(vector<StackObj>& tokens, size_t& pos) {
    consume(HEADER, tokens, pos);
    addlHppCode = consumeString(CODE, tokens, pos);
  }

  void parseSource(vector<StackObj>& tokens, size_t& pos) {
    consume(SOURCE, tokens, pos);
    addlCppCode = consumeString(CODE, tokens, pos);
  }

  bool maybeParseToken(vector<StackObj>& tokens, size_t& pos) {
    bool skip = false;
    // Check for an identifier to see if there are any more tokens
    string* name = maybeConsumeString(IDENT, tokens, pos);
    if (!name) {
      if (maybeConsume(SKIP, tokens, pos)) {
        skip = true;
      } else {
        return false;
      }
    }

    gdTokens.push_back(Token());
    Token& gdToken = gdTokens.back();
    gdToken.regex = consumeString(STRLIT, tokens, pos);

    // We indicate to the lexer that the token should be skipped by setting the
    // precedence to SKIP_TOKEN (this is useful for things such as comments)
    if (skip) {
      gdToken.precedence = SKIP_TOKEN;
      return true;
    }

    gdToken.name = *name;
    // Keep track of the index for this name so that we can use it when parsing #prec
    tokenNameToIndex.emplace(*name, gdTokens.size() - 1);
    // An arrow signifies that the token holds data
    if (maybeConsume(ARROW, tokens, pos)) {
      gdToken.type = consumeString(IDENT, tokens, pos);
      gdToken.ctorExpr = consumeString(CODE, tokens, pos);
      gdToken.dtorStmt = consumeString(CODE, tokens, pos);
    }
    return true;
  }


  void parseTokens(vector<StackObj>& tokens, size_t& pos) {
    consume(TOKENS, tokens, pos);
    while (maybeParseToken(tokens, pos));
  }

  bool maybeParsePrec(vector<StackObj>& tokens, size_t& pos, int prec) {
    vector<string*> precNames;
    // Check for an identifier to see if there are any more precedence lines
    string* precName;
    while ((precName = maybeConsumeString(IDENT, tokens, pos))) {
      precNames.push_back(precName);
    }
    if (precNames.empty()) {
      return false;
    }

    // Figure out the associativity (which is required)
    Assoc assoc;
    if (maybeConsume(LEFTASSOC, tokens, pos)) {
      assoc = Assoc::LEFT;
    } else if (maybeConsume(RIGHTASSOC, tokens, pos)) {
      assoc = Assoc::RIGHT;
    } else {
      consume(NONASSOC, tokens, pos);
      assoc = Assoc::NOT;
    }

    for (string* name : precNames) {
      // Look up the index of the token in the map
      auto iter = tokenNameToIndex.find(*name);
      if (iter == tokenNameToIndex.end()) {
        // This is not an actual token, just a placeholder to override a rule's precedence
        precNameToPrec.emplace(*name, prec);
        continue;
      }

      Token& gdToken = gdTokens[iter->second];
      gdToken.precedence = prec;
      gdToken.assoc = assoc;
    }

    return true;
  }

  void parsePrecs(vector<StackObj>& tokens, size_t& pos) {
    consume(PREC, tokens, pos);
    int prec = 1;
    while (maybeParsePrec(tokens, pos, prec++));
  }


  bool maybeParseGrammarDecl(vector<StackObj>& tokens, size_t& pos) {
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
  void parseGrammarDef(vector<StackObj>& tokens, size_t& pos, size_t concNum) {
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
      int prec;
      // Look up the index of the token in the map
      auto iter = tokenNameToIndex.find(tokenName);
      if (iter == tokenNameToIndex.end()) {
        auto precIter = precNameToPrec.find(tokenName);
        if (precIter == precNameToPrec.end()) {
          throw runtime_error("Unknown token " + tokenName);
        } else {
          prec = precIter->second;
        }
      } else {
        prec = gdTokens[iter->second].precedence;
      }

      if (prec == NONE) {
        cerr << "WARNING: Token " << tokenName << " is used to override a rule's precedence, "
            "but has no precedence set." << endl;
      }
      gdConcrete.precedence = prec;
    }

    gdConcrete.ctorExpr = consumeString(CODE, tokens, pos);
  }

  bool maybeParseGrammarVar(vector<StackObj>& tokens, size_t& pos) {
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


  void parseGrammar(vector<StackObj>& tokens, size_t& pos) {
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



ParseInfo parseConfig(const string& fileName) {
  ifstream configFile(fileName);
  vector<StackObj> tokens = tokenize(configFile);
  if (tokens.empty()) {
    throw runtime_error("File " + fileName + " is empty.");
  }

  size_t pos = 0;
  parseHeader(tokens, pos);
  parseSource(tokens, pos);
  parseTokens(tokens, pos);
  parsePrecs(tokens, pos);
  parseGrammar(tokens, pos);

  return { grammarData, addlHppCode, addlCppCode };
}

