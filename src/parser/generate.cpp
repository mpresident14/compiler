#include "src/parser/generate.hpp"
#include "src/parser/build_parser.hpp"
#include "src/parser/regex_eval.hpp"

#include <fstream>
#include <cstddef>
#include <sstream>

using namespace std;

namespace {
  /***********
   * TO CODE *
   ***********/

  template <typename T>
  void toCode(ostream& out, const vector<T>& v);
  void toCode(ostream& out, const string& str);
  void toCode(ostream& out, int n);
  void toCode(ostream& out, const Token& token);
  void toCode(ostream& out, const Concrete& concrete);
  void toCode(ostream& out, const Variable& var);
  void toCode(ostream& out, const GrammarData& grammarData);


  void toCode(ostream& out, const string& str) { out << '"' << str << '"'; }

  void toCode(ostream& out, int n) { out << to_string(n); }

  template <typename T>
  void toCode(ostream& out, const vector<T>& v) {
    out << '{';
    for_each(v.cbegin(), v.cend(), [&out](const T& item) {
      toCode(out, item);
      out << ',';
    });
    out << '}';
  }

  void toCode(ostream& out, const Token& token) {
    out << '{';
    toCode(out, token.name);
    out << ',';
    toCode(out, token.type);
    out << ',';
    toCode(out, token.precedence);
    out << '}';
    // Other fields not needed for shift-reducing
  }

  void toCode(ostream& out, const Concrete& concrete) {
    out << '{';
    toCode(out, concrete.varType);
    out << '}';
    // Other fields not needed for shift-reducing
  }

  void toCode(ostream& out, const Variable& var) {
    out << '{';
    toCode(out, var.name);
    out << ',';
    toCode(out, var.type);
    out << '}';
    // Other fields not needed for shift-reducing
  }

  void toCode(ostream& out, const GrammarData& grammarData) {
    out << '{';
    toCode(out, grammarData.tokens);
    out << ',';
    toCode(out, grammarData.concretes);
    out << ',';
    toCode(out, grammarData.variables);
    out << '}';
  }


  /****************
   * GRAMMAR DATA *
   ****************/

  void tokenDecl(ostream& out) {
    out << R"(struct Token {
      string name;
      string type;
      int precedence;
    };
    )";
  }

  void concreteDecl(ostream& out) {
    out << R"(struct Concrete {
      int varType;
    };
    )";
  }

  void variableDecl(ostream& out) {
    out << R"(struct Variable {
      string name;
      string type;
    };
    )";
  }

  void grammarDataDecl(ostream& out, const GrammarData& grammarData) {
    out << R"(struct GrammarData {
      vector<Token> tokens;
      vector<Concrete> concretes;
      vector<Variable> variables;
    };
    )";
    out << "GrammarData GRAMMAR_DATA = ";
    toCode(out, grammarData);
    out << ';';
  }

  /****************
   * STACK OBJECT *
   ****************/

  void stackObjDecl(ostream& out) {
    out << R"(struct StackObj {
      void* obj;
      int symbol;
      size_t line;
    };
    )";
  }

  void sObjDecl(ostream& out, const GrammarData& grammarData) {
    const char decl[] = R"(struct SObj {
      SObj(#0* r) : r_(r) {}
      ~SObj() { delete r_; }
      #0* r_;
    };
    )";
    replaceNumbersVec(out, decl, { grammarData.variables[1].type });
  }


  void deleteObjFn(ostream& out, const GrammarData& grammarData) {
    out << R"(void deleteObj(const StackObj& s) {
      switch (s.symbol) {)";

    size_t numTokens = grammarData.tokens.size();
    // Case statement for each token with data
    for (size_t i = 0; i < numTokens; ++i) {
      const Token& token = grammarData.tokens[i];
      if (!token.type.empty()) {
        out << "case " << tokenToFromIndex(i) << ':';
        replaceNumbersVec(
            out, token.dtorStmt, { "*(" + token.type + "*) s.obj" });
        out << " delete (" << token.type << "*) s.obj; break;";
      }
    }

    // Case statement for each variable except S
    size_t numVars = grammarData.variables.size();
    for (size_t i = 1; i < numVars; ++i) {
      const Variable& var = grammarData.variables[i];
      out << "case " << i << ':';
      replaceNumbersVec(out, var.dtorStmt, { "*(" + var.type + "*) s.obj" });
      out << " delete (" << var.type << "*) s.obj; break;";
    }

    out << "default: return;}}";
  }


  void deleteObjPtrFn(ostream& out, const GrammarData& grammarData) {
    out << R"(void deleteObjPtr(const StackObj& s) {
      switch (s.symbol) {)";

    size_t numTokens = grammarData.tokens.size();
    // Case statement for each token with data
    for (size_t i = 0; i < numTokens; ++i) {
      const Token& token = grammarData.tokens[i];
      if (!token.type.empty()) {
        out << "case " << tokenToFromIndex(i) << ':' << " delete ("
            << token.type << "*) s.obj; break;";
      }
    }

    // Case statement for each variable except S
    size_t numVars = grammarData.variables.size();
    for (size_t i = 1; i < numVars; ++i) {
      const Variable& var = grammarData.variables[i];
      out << "case " << i << ':' << " delete (" << var.type
          << "*) s.obj; break;";
    }

    out << "default: return;}}";
  }


  void constructObjFn(ostream& out, const GrammarData& grammarData) {
    out << R"(void* constructObj(int concrete, StackObj* args) {
      switch (concrete) {)";
    size_t numConcretes = grammarData.concretes.size();
    // Case statement for each concrete except SCONC (handled specially below)
    for (size_t i = 1; i < numConcretes; ++i) {
      const Concrete& concrete = grammarData.concretes[i];
      const Variable& var = grammarData.variables[concrete.varType];
      out << "case " << i << ": return new " << var.type << '(';

      replaceNumbers(
          out,
          concrete.ctorExpr,
          [&concrete, &grammarData](const string& digits) {
            int i = stoi(digits);
            checkBounds(i, concrete.argSymbols);
            int argSymbol = concrete.argSymbols[i];
            string symbolName;
            if (isToken(argSymbol)) {
              symbolName = grammarData.tokens[tokenToFromIndex(argSymbol)].type;
            } else {
              symbolName = grammarData.variables[argSymbol].type;
            }
            return string("*(")
                .append(symbolName)
                .append("*) args[")
                .append(digits)
                .append("].obj");
          });

      out << ");";
    }

    // Root type of grammar is the first type listed
    out << "case 0: return new SObj((" << grammarData.variables[1].type
        << "*)args[0].obj);";
    out << R"(default: throw invalid_argument("Can't construct. Out of options.");}})";
  }


  void constructFn(ostream& out) {
    out << R"(StackObj construct(int concrete, vector<StackObj>& stk, size_t reduceStart, int varType) {
      size_t line = reduceStart == stk.size() ? 0 : stk[reduceStart].line;
      return StackObj{ constructObj(concrete, &stk[reduceStart]), varType, line};
    })";
  }


  void constructTokenObjFn(ostream& out, const GrammarData& grammarData) {
    out << R"(StackObj constructTokenObj(int token, const string_view& str) {
      switch (token) {)";
    size_t numTokens = grammarData.tokens.size();
    for (size_t i = 0; i < numTokens; ++i) {
      const Token& token = grammarData.tokens[i];
      if (!token.type.empty()) {
        out << "case " << tokenToFromIndex(i) << ':' << "return { new "
            << token.type << '(' << token.ctorExpr << "), token, currentLine };break;";
      }
    }
    out << R"(default: return {nullptr, token, currentLine}; }})";
  }


  /********
   * DFAs *
   ********/
  void lexerDFA(ostream& out, const GrammarData grammarData) {
    out << "namespace lexer {";
    rgxDFAToCode(out, grammarData);
    out << '}';
  }


  void parserDFA(ostream& out, const GrammarData grammarData) {
    out << "namespace parser {";
    condensedDFAToCode(out, grammarData);
    out << '}';
  }

  /**********
   * LEXING *
   **********/

  void currentLineDecl(ostream& out) {
    out << "static size_t currentLine = 1;";
  }

  void tokenizeFn(ostream& out) {
    out << R"(
      void consumeWhiteSpace(string_view& input) {
        size_t i = 0;
        char c;
        while (isspace((c = input[i]))) {
          if (c == '\n') {
            ++currentLine;
          }
          ++i;
        }
        input = input.substr(i);
      }

      optional<StackObj> getToken(string_view& input) {
        size_t i = 0;
        const size_t len = input.size();
        size_t lastAcceptingPos;
        int lastAcceptingToken = NONE;
        size_t newlineCount = 0;
        size_t lastAcceptingNewlineCount = 0;
        lexer::Node* currentNode = lexer::root.get();

        while (currentNode) {
          int token = currentNode->v_;
          if (token != NONE) {
            lastAcceptingToken = token;
            lastAcceptingPos = i;
            lastAcceptingNewlineCount = newlineCount;
          }

          if (i == len) {
            break;
          }
          char c;
          currentNode = currentNode->step((c = input[i]));
          ++i;
          if (c == '\n') {
            ++newlineCount;
          }
        }

        if (lastAcceptingToken == NONE) {
          return {};
        }

        StackObj stackObj =
            constructTokenObj(lastAcceptingToken, input.substr(0, lastAcceptingPos));
        input = input.substr(lastAcceptingPos);
        currentLine += lastAcceptingNewlineCount;
        return { move(stackObj) };
      }


      vector<StackObj> tokenize(const string& input) {
        if (input.empty()) {
          return {};
        }

        vector<StackObj> tokens;
        string_view inputView = input;

        consumeWhiteSpace(inputView);
        while (!inputView.empty()) {
          optional<StackObj> optionalObj = getToken(inputView);
          if (optionalObj.has_value()) {
            if (GRAMMAR_DATA.tokens[tokenToFromIndex(optionalObj->symbol)].precedence != SKIP_TOKEN) {
              tokens.push_back(move(*optionalObj));
            }
          } else {
            ostringstream error;
            vector<string> prevTokenNames;
            auto startIter =
                tokens.size() < 25 ? tokens.cbegin() : tokens.cend() - 25;
            transform(
                move(startIter),
                tokens.cend(),
                back_inserter(prevTokenNames),
                [](const StackObj& stackObj) { return GRAMMAR_DATA.tokens[tokenToFromIndex(stackObj.symbol)].name; });
            error << "Lexer error on line " << currentLine << " at: " << inputView.substr(0, 25) << '\n'
                << "Previous tokens were: " << prevTokenNames;
            for_each(tokens.cbegin(), tokens.cend(), deleteObj);
            throw runtime_error(error.str());
          }

          consumeWhiteSpace(inputView);
        }

        return tokens;
      }
    )";
  }

  void tokenizeFileFn(ostream& out) {
    out << R"(vector<StackObj> tokenize(istream& input) {
        return tokenize(string(istreambuf_iterator<char>{input}, istreambuf_iterator<char>{}));
      }
    )";
  }

  void lexerHppDecls(ostream& out) {
    out << R"(void deleteObj(const StackObj& s);
      std::vector<StackObj> tokenize(const std::string& input);
      std::vector<StackObj> tokenize(std::istream& input);
    )";
  }

  /***********
   * PARSING *
   ***********/

  void assocDecl(ostream& out) {
    out << "enum class Assoc { LEFT, RIGHT, NOT, NONE };";
  }

  void dfaRuleDecl(ostream& out) {
    out << R"(struct DFARule {
      int concrete;
      vector<int> symbols;
      size_t pos;
      mutable vector<bool> lookahead;
    };
    )";
  }


  void ruleDataDecl(ostream& out) {
    out << R"(struct RuleData {
      optional<DFARule> reducibleRule;
      int precedence;
      Assoc assoc;
    };
    )";
  }


  void parseDecl(ostream& out, const GrammarData& grammarData) {
    const string& rootType = grammarData.variables[1].type;
    out << rootType << " parse(const std::string& input);" << rootType
        << " parse(std::istream& input);";
  }


  void parseHelperFns(ostream& out) {
    out << R"(
      void cleanPtrsFrom(const vector<StackObj>& stackObjs, size_t i) {
        size_t size = stackObjs.size();
        for (; i < size; ++i) {
          deleteObj(stackObjs[i]);
        }
      }

      void parseError(
          const vector<StackObj>& stk,
          const vector<StackObj>& inputTokens,
          size_t i) {
        ostringstream errMsg;
        vector<string> stkSymbolNames;
        vector<string> remainingTokenNames;
        auto stkObjToName = [](StackObj stkObj) {
          if (isToken(stkObj.symbol)) {
            return GRAMMAR_DATA.tokens[tokenToFromIndex(stkObj.symbol)].name;
          }
          return GRAMMAR_DATA.variables[stkObj.symbol].name;
        };

        transform(
            stk.begin(), stk.end(), back_inserter(stkSymbolNames), stkObjToName);
        transform(
            inputTokens.begin() + i,
            inputTokens.end(),
            back_inserter(remainingTokenNames),
            stkObjToName);

        errMsg << "Parse error on line " << stk[stk.size() - 1].line << ":\n\tStack: " << stkSymbolNames
              << "\n\tRemaining tokens: " << remainingTokenNames;
        throw invalid_argument(errMsg.str());
      }
    )";
  }

  void tryReduceFn(ostream& out) {
    out << R"(int tryReduce(
        const parser::Node* node,
        int nextToken,
        const vector<StackObj>& stk,
        const vector<Token>& tokens) {
      const RuleData& ruleData = node->v_;
      if (!ruleData.reducibleRule.has_value()) {
        return NONE;
      }
      const DFARule& rule = *ruleData.reducibleRule;
      if (nextToken != NONE && !rule.lookahead[tokenToFromIndex(nextToken)]) {
        return NONE;
      }
      if (!equal(
              rule.symbols.crbegin(),
              rule.symbols.crend(),
              stk.crbegin(),
              [](int symbol, const StackObj& stkObj) {
        return stkObj.symbol == symbol;
      })) {
        return NONE;
      }
      if (!node->ts_.contains(nextToken)) {
        return ruleData.reducibleRule->concrete;
      }
      int shiftPrecedence = tokens[tokenToFromIndex(nextToken)].precedence;
      if (ruleData.precedence == NONE && shiftPrecedence == NONE) {
        return NONE;
      }
      if (ruleData.precedence > shiftPrecedence
          || (ruleData.precedence == shiftPrecedence && ruleData.assoc == Assoc::LEFT)) {
        return ruleData.reducibleRule->concrete;
      }
      return NONE;
    })";
  }


  void shiftReduceFn(ostream& out, const GrammarData& grammarData) {
    const char code[] = R"(
        #0 shiftReduce(vector<StackObj>& inputTokens) {
        if (inputTokens.empty()) {
          parseError({}, inputTokens, 0);
        }
        vector<StackObj> stk = { move(inputTokens[0]) };
        vector<parser::Node*> dfaPath = { parser::root.get() };
        size_t i = 1;
        size_t inputSize = inputTokens.size();
        while (!(i == inputSize && stk.size() == 1 && stk[0].symbol == S)) {
          parser::Node* currentNode = dfaPath.back()->step(stk.back().symbol);
          if (currentNode == nullptr) {
            cleanPtrsFrom(stk, 0);
            cleanPtrsFrom(inputTokens, i);
            parseError(stk, inputTokens, i);
          }
          dfaPath.push_back(currentNode);

          int nextInputToken = i == inputSize ? NONE : inputTokens[i].symbol;
          int concrete =
              tryReduce(currentNode, nextInputToken, stk, GRAMMAR_DATA.tokens);
          if (concrete != NONE) {
            size_t reduceStart =
                stk.size() - currentNode->v_.reducibleRule->symbols.size();
            StackObj newObj = construct(
                concrete,
                stk,
                reduceStart,
                GRAMMAR_DATA.concretes[concrete].varType);
            if (newObj.symbol == S) {
              stk.pop_back();
            } else {
              size_t stkSize = stk.size();
              for (size_t j = 0; j < stkSize - reduceStart; ++j) {
                deleteObjPtr(stk.back());
                stk.pop_back();
                dfaPath.pop_back();
              }
            }
            stk.push_back(newObj);
          } else {

            if (i == inputSize) {
              cleanPtrsFrom(stk, 0);
              parseError(stk, inputTokens, i);
            }
            StackObj token = inputTokens[i];
            stk.push_back(move(token));
            ++i;
          }
        }

        SObj* start = (SObj*)(stk[0].obj);
        #0 root = *start->r_;
        delete start;
        return root;
      }
    )";

    replaceNumbersVec(out, code, { grammarData.variables[1].type });
  }


  void parseFn(ostream& out, const GrammarData& grammarData) {
    const string& rootType = grammarData.variables[1].type;
    out << rootType << R"(
      parse(const string& input) {
        vector<StackObj> stackObjs = tokenize(input);
        return shiftReduce(stackObjs);
      }
    )" << rootType
        << R"(
      parse(istream& input) {
        return parse(string(istreambuf_iterator<char>{input}, istreambuf_iterator<char>{}));
      }
    )";
  }


  /********
   * MISC *
   ********/

  void parserHppIncludes(ostream& out) {
    out << R"(
      #include <iostream>
      #include <string>
    )";
  }

  void lexerHppIncludes(ostream& out) {
    out << R"(
      #include <vector>
      #include <string>
      #include <iostream>
    )";
  }

  void cppIncludes(ostream& out) {
    out << R"(
      #include <vector>
      #include <cstddef>
      #include <algorithm>
      #include <functional>
      #include <optional>
      #include <stdexcept>
      #include <memory>
      #include <sstream>
      #include <climits>
      #include <unordered_map>
      #include <string_view>
      #include <streambuf>
      #include <prez/print_stuff.hpp>
    )";
  }


  void noneInt(ostream& out) {
    out << R"(
      constexpr int NONE = INT_MIN;
      constexpr int SKIP_TOKEN = INT_MIN + 1;
    )";
  }

  void sInt(ostream& out) {
    out << R"(
      constexpr int S = 0;
    )";
  }

  void isTokenFn(ostream& out) {
    out << R"(
      bool isToken(int symbol) { return symbol < 0; }
    )";
  }

  void tokenToFromIndexFn(ostream& out) {
    out << R"(
      int tokenToFromIndex(int token) { return -token - 1; }
    )";
  }

  string replaceAll(const string& str, char from, const string& to) {
    string s;
    s.reserve(str.size());

    for (char c : str) {
      if (c == from) {
        s.append(to);
      } else {
        s.push_back(c);
      }
    }
    return s;
  }


  /********************
   * DRIVER FUNCTIONS *
   ********************/

  constexpr char generatedWarning[] = "/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */\n";

  string parserHppCode(const string& namespaceName, const string& headerGuard, const string& addlHdrIncludes, const GrammarData& grammarData) {
    stringstream out;

    out << "#ifndef "<< headerGuard << "\n#define " << headerGuard << '\n' << endl;
    out << generatedWarning;
    out << addlHdrIncludes;
    parserHppIncludes(out);
    out << "namespace " << namespaceName << '{';
    parseDecl(out, grammarData);
    out << "}\n#endif";

    return out.str();
  }

  string lexerHppCode(const string& namespaceName, const string& headerGuard) {
    stringstream out;

    out << "#ifndef "<< headerGuard << "\n#define " << headerGuard << '\n' << endl;
    out << generatedWarning;
    lexerHppIncludes(out);
    out << "namespace " << namespaceName << '{';
    stackObjDecl(out);
    lexerHppDecls(out);
    out << "}\n#endif";

    return out.str();
  }

  string parserCppCode(
      const string& parserFilePath,
      const string& namespaceName,
      const string& addlCode,
      const GrammarData& grammarData) {
    stringstream out;

    out << generatedWarning;
    out << "#include \"" << parserFilePath << ".hpp\"\n";
    cppIncludes(out);
    out << addlCode
        << "using namespace std;"
        << "namespace {";
    noneInt(out);
    sInt(out);
    isTokenFn(out);
    tokenToFromIndexFn(out);
    tokenDecl(out);
    concreteDecl(out);
    variableDecl(out);
    grammarDataDecl(out, grammarData);
    stackObjDecl(out);
    sObjDecl(out, grammarData);
    currentLineDecl(out);
    deleteObjPtrFn(out, grammarData);
    deleteObjFn(out, grammarData);
    constructObjFn(out, grammarData);
    constructFn(out);
    constructTokenObjFn(out, grammarData);
    lexerDFA(out, grammarData);
    assocDecl(out);
    dfaRuleDecl(out);
    ruleDataDecl(out);
    parserDFA(out, grammarData);
    tokenizeFn(out);
    parseHelperFns(out);
    tryReduceFn(out);
    shiftReduceFn(out, grammarData);
    out << "} namespace " << namespaceName << '{';
    parseFn(out, grammarData);
    out << '}';

    return out.str();
  }

   string lexerCppCode(
      const string& lexerFilePath,
      const string& namespaceName,
      const string& addlCode,
      const GrammarData& grammarData) {
    stringstream out;

    out << generatedWarning;
    out << "#include \"" << lexerFilePath << ".hpp\"\n";
    cppIncludes(out);
    out << addlCode
        << "using namespace std;"
        << "using namespace " << namespaceName << ';'
        << "namespace {";
    noneInt(out);
    tokenToFromIndexFn(out);
    tokenDecl(out);
    concreteDecl(out);
    variableDecl(out);
    grammarDataDecl(out, grammarData);
    currentLineDecl(out);
    constructTokenObjFn(out, grammarData);
    lexerDFA(out, grammarData);
    out << "} namespace " << namespaceName << '{';
    deleteObjFn(out, grammarData);
    tokenizeFn(out);
    tokenizeFileFn(out);
    out << '}';

    return out.str();
  }

}  // namespace


void generateParserCode(
    const string& parserFilePath,
    const string& addlHdrIncludes,
    const string& addlCode,
    const GrammarData& grammarData) {
  string namespaceName = replaceAll(parserFilePath, '/', "::");
  string headerGuard = replaceAll(parserFilePath, '/', "_") + "_HPP";
  transform(headerGuard.begin(), headerGuard.end(), headerGuard.begin(), ::toupper);

  std::ofstream hppFile;
  hppFile.open(parserFilePath + ".hpp");
  hppFile << parserHppCode(namespaceName, headerGuard, addlHdrIncludes, grammarData);
  hppFile.close();

  std::ofstream cppFile;
  cppFile.open(parserFilePath + ".cpp");
  cppFile << parserCppCode(parserFilePath, namespaceName, addlCode, grammarData);
  cppFile.close();
}


void generateLexerCode(
    const string& lexerFilePath,
    const string& addlCode,
    const GrammarData& grammarData) {
  string namespaceName = replaceAll(lexerFilePath, '/', "::");
  string headerGuard = replaceAll(lexerFilePath, '/', "_") + "_HPP";
  transform(headerGuard.begin(), headerGuard.end(), headerGuard.begin(), ::toupper);

  std::ofstream hppFile;
  hppFile.open(lexerFilePath + ".hpp");
  hppFile << lexerHppCode(namespaceName, headerGuard);
  hppFile.close();

  std::ofstream cppFile;
  cppFile.open(lexerFilePath + ".cpp");
  cppFile << lexerCppCode(lexerFilePath, namespaceName, addlCode, grammarData);
  cppFile.close();
}

