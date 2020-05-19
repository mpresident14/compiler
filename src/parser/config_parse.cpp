#include "src/parser/config_parse.hpp"

#include "src/parser/config_grammar.hpp"
#include "src/parser/config_lexer.hpp"
#include "src/misc/errors.hpp"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <prez/print_stuff.hpp>

using namespace std;
using namespace config_lexer;

namespace {
  string addlHppCode;
  string addlCppCode;
  // Concrete #0 is S, so skip it
  vector<size_t> concreteLines = {0};
  GrammarData grammarData;
  vector<Token>& gdTokens = grammarData.tokens;
  vector<Concrete>& gdConcretes = grammarData.concretes;
  vector<Variable>& gdVariables = grammarData.variables;

  unordered_map<string, size_t> tokenNameToIndex;
  unordered_map<string, size_t> varNameToIndex;
  unordered_map<string, int> precNameToPrec;
  unordered_map<const string*, int> symbolToLineMap;
  stringstream errors;
  string globFileName;


  class TokenStream {
  public:
    TokenStream(vector<StackObj>& tokens) : tokens_(tokens) {}
    ~TokenStream() = default;
    TokenStream(const TokenStream& other) = delete;
    TokenStream& operator=(const TokenStream& other) = delete;
    TokenStream(TokenStream&& other) = delete;
    TokenStream& operator=(TokenStream&& other) = delete;

    void parseError(int tokenId) {
      errors << globFileName << errorColored << " on line " << tokens_[pos_].getLine()
             << ". Expected " << symbolToString(tokenId, CONFIG_GRAMMAR)
             << ". Got "
             << symbolToString(tokens_[pos_].getSymbol(), CONFIG_GRAMMAR) << '\n';
    }

    bool maybeConsume(int tokenId) {
      if (tokens_.size() == pos_ || tokens_[pos_].getSymbol() != tokenId) {
        return false;
      }
      ++pos_;
      return true;
    }

    void consume(int tokenId) {
      if (!maybeConsume(tokenId)) {
        parseError(tokenId);
      }
    }

    string* maybeConsumeString(int tokenId) {
      if (tokens_.size() == pos_ || tokens_[pos_].getSymbol() != tokenId) {
        return nullptr;
      }
      return (string*)tokens_[pos_++].getObj();
    }

    string consumeString(int tokenId) {
      string* strPtr = maybeConsumeString(tokenId);
      if (!strPtr) {
        parseError(tokenId);
      }
      return *strPtr;
    }

    size_t currentLine() const noexcept {
      return pos_ == 0 ? 0 : tokens_[pos_ - 1].getLine();
    }

  private:
    vector<StackObj>& tokens_;
    size_t pos_ = 0;
  };


  void parseHeader(TokenStream& tokenStream) {
    tokenStream.consume(HEADER);
    addlHppCode = tokenStream.consumeString(CODE);
  }

  void parseSource(TokenStream& tokenStream) {
    tokenStream.consume(SOURCE);
    addlCppCode = tokenStream.consumeString(CODE);
  }

  bool maybeParseToken(TokenStream& tokenStream) {
    bool skip = false;
    // Check for an identifier to see if there are any more tokens
    string* name = tokenStream.maybeConsumeString(IDENT);
    if (!name) {
      if (tokenStream.maybeConsume(SKIP)) {
        skip = true;
      } else {
        return false;
      }
    }


    gdTokens.push_back(Token());
    Token& gdToken = gdTokens.back();
    gdToken.regex = tokenStream.consumeString(STRLIT);

    // We indicate to the lexer that the token should be skipped by setting the
    // precedence to SKIP_TOKEN (this is useful for things such as comments)
    if (skip) {
      gdToken.precedence = SKIP_TOKEN;
      return true;
    }

    if (tokenNameToIndex.contains(*name)) {
      errors << globFileName << errorColored << " on line " << tokenStream.currentLine()
              << ": Duplicate token " << *name;
    }

    gdToken.name = *name;
    // Keep track of the index for this name so that we can use it when parsing
    // #prec
    tokenNameToIndex.emplace(move(*name), gdTokens.size() - 1);
    // An arrow signifies that the token holds data
    if (tokenStream.maybeConsume(ARROW)) {
      gdToken.type = tokenStream.consumeString(TYPE);
      gdToken.ctorExpr = tokenStream.consumeString(CODE);
      string* dtor = tokenStream.maybeConsumeString(CODE);
      if (dtor) {
        gdToken.dtorStmt = move(*dtor);
      }
    }
    return true;
  }

  void parseTokens(TokenStream& tokenStream) {
    tokenStream.consume(TOKENS);
    while (maybeParseToken(tokenStream))
      ;
  }

  bool maybeParsePrec(TokenStream& tokenStream, int prec) {
    vector<string*> precNames;
    // Check for an identifier to see if there are any more precedence lines
    string* precName;
    while ((precName = tokenStream.maybeConsumeString(IDENT))) {
      precNames.push_back(precName);
    }
    if (precNames.empty()) {
      return false;
    }

    // Figure out the associativity (which is required if precedence is
    // specified)
    Assoc assoc;
    if (tokenStream.maybeConsume(LEFTASSOC)) {
      assoc = Assoc::LEFT;
    } else if (tokenStream.maybeConsume(RIGHTASSOC)) {
      assoc = Assoc::RIGHT;
    } else {
      tokenStream.consume(NONASSOC);
      assoc = Assoc::NOT;
    }

    for (string* name : precNames) {
      // Look up the index of the token in the map
      auto iter = tokenNameToIndex.find(*name);
      if (iter == tokenNameToIndex.end()) {
        // This is not an actual token, just a placeholder to override a rule's
        // precedence
        precNameToPrec.emplace(move(*name), prec);
        continue;
      }

      Token& gdToken = gdTokens[iter->second];
      gdToken.precedence = prec;
      gdToken.assoc = assoc;
    }

    return true;
  }

  void parsePrecs(TokenStream& tokenStream) {
    tokenStream.consume(PREC);
    int prec = 1;
    while (maybeParsePrec(tokenStream, prec++))
      ;
  }

  bool maybeParseGrammarDecl(TokenStream& tokenStream) {
    // Check for an identifier to see if there are any more declarations
    string* name = tokenStream.maybeConsumeString(IDENT);
    if (!name) {
      return false;
    }

    gdVariables.push_back(Variable());
    Variable& gdVariable = gdVariables.back();
    gdVariable.name = *name;
    varNameToIndex.emplace(move(*name), gdVariables.size() - 1);
    tokenStream.consume(ARROW);
    gdVariable.type = tokenStream.consumeString(TYPE);
    string* dtor = tokenStream.maybeConsumeString(CODE);
    if (dtor) {
      gdVariable.dtorStmt = *dtor;
    }
    return true;
  }

  /* This must be called right after parseGrammarDecl() */
  void parseGrammarDef(TokenStream& tokenStream, size_t concNum) {
    Variable& gdVariable = gdVariables.back();
    gdConcretes.push_back(Concrete());
    Concrete& gdConcrete = gdConcretes.back();
    gdVariable.concreteTypes.push_back(gdConcretes.size() - 1);
    gdConcrete.name = gdVariable.name + to_string(concNum);
    gdConcrete.varType = gdVariables.size() - 1;
    concreteLines.push_back(tokenStream.currentLine());
    // Store argSymbols as string*s for now until we have seen all the
    // variables. Then, we will convert them to correct integral values.
    // Also store the line number in a map for error checking
    string* argSymbol;
    while ((argSymbol = tokenStream.maybeConsumeString(IDENT))) {
      gdConcrete.argSymbols.push_back((intptr_t)argSymbol);
      symbolToLineMap.emplace(argSymbol, tokenStream.currentLine());
    }

    if (tokenStream.maybeConsume(PREC)) {
      string tokenName = tokenStream.consumeString(IDENT);
      int prec = -1;
      // Look up the index of the token in the map
      auto iter = tokenNameToIndex.find(tokenName);
      if (iter == tokenNameToIndex.end()) {
        auto precIter = precNameToPrec.find(tokenName);
        if (precIter == precNameToPrec.end()) {
          errors << globFileName << errorColored << " on line " << tokenStream.currentLine()
                 << ": Unknown token " << tokenName << '\n';
        } else {
          prec = precIter->second;
        }
      } else {
        prec = gdTokens[iter->second].precedence;
      }

      if (prec == NONE) {
        errors << globFileName << warningColored << " on line " << tokenStream.currentLine()
             << ": Token " << tokenName
             << " is used to override a rule's precedence, "
                "but has no precedence set.\n"
             << endl;
      }
      gdConcrete.precedence = prec;
    }

    gdConcrete.ctorExpr = tokenStream.consumeString(CODE);
  }

  bool maybeParseGrammarVar(TokenStream& tokenStream) {
    if (!maybeParseGrammarDecl(tokenStream)) {
      return false;
    }

    tokenStream.consume(DEFINED);
    size_t concNum = 0;
    parseGrammarDef(tokenStream, concNum++);
    while (tokenStream.maybeConsume(BAR)) {
      parseGrammarDef(tokenStream, concNum++);
    }
    return true;
  }

  void parseGrammar(TokenStream& tokenStream) {
    gdVariables.push_back(Variable{ "S", "Start", { SCONC }, "" });
    gdConcretes.push_back(Concrete{ "SCONC", S, NONE, { 1 }, "Start(#0)" });
    tokenStream.consume(GRAMMAR);
    while (maybeParseGrammarVar(tokenStream));

    // Translate the string pointers to token/variable ids now that we
    // have parsed the whole file (skip SCONC)
    size_t numConcs = gdConcretes.size();
    for (size_t i = 1; i < numConcs; ++i) {
      Concrete& concrete = gdConcretes[i];
      size_t len = concrete.argSymbols.size();
      for (size_t j = 0; j < len; ++j) {
        const string& symbolName = *(string*)concrete.argSymbols[j];

        // Check if it is a token first
        auto tokIter = tokenNameToIndex.find(symbolName);
        if (tokIter == tokenNameToIndex.end()) {
          // Otherwise, check if it is a variable
          auto varIter = varNameToIndex.find(symbolName);
          if (varIter == varNameToIndex.end()) {
            errors << globFileName << errorColored << " on line " << symbolToLineMap.at(&symbolName)
                   << ": Unknown symbol " << symbolName << '\n';
          } else {
            concrete.argSymbols[j] = varIter->second;
          }
        } else {
          concrete.argSymbols[j] = tokenToFromIndex(tokIter->second);
        }
      }
    }
  }

}  // namespace


ParseInfo parseConfig(const string& fileName) {
  globFileName = fileName + ": ";
  ifstream configFile(fileName);
  vector<StackObj> tokens = tokenize(configFile);
  if (tokens.empty()) {
    errors << globFileName << errorColored << ": File " + fileName + " is empty.\n";
  }

  TokenStream tokenStream(tokens);
  parseHeader(tokenStream);
  parseSource(tokenStream);
  parseTokens(tokenStream);
  parsePrecs(tokenStream);
  parseGrammar(tokenStream);

  if (gdTokens.empty()) {
    errors << globFileName << errorColored << ": No tokens were provided.\n";
  }
  if (gdVariables.empty()) {
    errors << globFileName << errorColored << ": No grammar variables were provided.\n";
  }

  const string& errStr = errors.str();
  if (!errStr.empty()) {
    throw runtime_error(errStr.c_str());
  }

  return { grammarData, addlHppCode, addlCppCode, concreteLines };
}
