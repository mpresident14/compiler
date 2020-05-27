#include "src/parser/generate.hpp"

#include "src/misc/error_store.hpp"
#include "src/parser/build_parser.hpp"
#include "src/parser/regex_merge.hpp"
#include "src/parser/regex_parser.hpp"

#include <cstddef>
#include <fstream>
#include <sstream>

#include <boost/utility/string_view.hpp>

using namespace std;

namespace {

ErrorStore errorStore;

/***********
 * TO CODE *
 ***********/

inline void toCode(ostream& out, const string& str);
inline void toCode(ostream& out, int n);
inline void toCode(ostream& out, Assoc assoc);
template <typename T>
inline void toCode(ostream& out, const vector<T>& v);
inline void toCode(ostream& out, const Token& token);
inline void toCode(ostream& out, const Concrete& concrete);
inline void toCode(ostream& out, const Variable& var);
inline void toCode(ostream& out, const GrammarData& gd);

inline void toCode(ostream& out, const string& str) {
  out << '"' << str << '"';
}

inline void toCode(ostream& out, int n) { out << to_string(n); }

inline void toCode(ostream& out, Assoc assoc) { out << "Assoc::" << assoc; }

template <typename T>
inline void toCode(ostream& out, const vector<T>& v) {
  out << '{';
  for_each(v.cbegin(), v.cend(), [&out](const T& item) {
    toCode(out, item);
    out << ',';
  });
  out << '}';
}

inline void toCode(ostream& out, const Token& token) {
  out << '{';
  toCode(out, token.name);
  out << ',';
  toCode(out, token.type);
  out << ',';
  toCode(out, token.precedence);
  out << ',';
  toCode(out, token.assoc);
  out << '}';
  // Other fields not needed for shift-reducing
}

inline void toCode(ostream& out, const Concrete& concrete) {
  out << '{';
  toCode(out, concrete.varType);
  out << '}';
  // Other fields not needed for shift-reducing
}

inline void toCode(ostream& out, const Variable& var) {
  out << '{';
  toCode(out, var.name);
  out << ',';
  toCode(out, var.type);
  out << '}';
  // Other fields not needed for shift-reducing
}

inline void toCode(ostream& out, const GrammarData& gd) {
  out << '{';
  toCode(out, gd.tokens);
  out << ',';
  toCode(out, gd.concretes);
  out << ',';
  toCode(out, gd.variables);
  out << '}';
}

/*********************
 * STRING OPERATIONS *
 *********************/
void replaceAll(
    ostream& out,
    string_view str,
    const string& from,
    const string& to) {
  size_t startPos = 0;
  size_t endPos = 0;
  while ((endPos = str.find(from, startPos)) != string::npos) {
    out << str.substr(startPos, endPos - startPos) << to;
    startPos = endPos + from.length();
  }
  out << str.substr(startPos);
}

string replaceAll(string_view str, char from, char to) {
  string s;
  s.reserve(str.size());

  for (char c : str) {
    if (c == from) {
      s.push_back(to);
    } else {
      s.push_back(c);
    }
  }
  return s;
}

pair<string, string> getNamespaceAndGuard(string_view filePath) {
  string_view namespaceName = filePath.substr(filePath.find_last_of('/') + 1);

  string headerGuard = replaceAll(namespaceName, '/', '_');
  transform(
      headerGuard.begin(), headerGuard.end(), headerGuard.begin(), ::toupper);
  return { string(namespaceName), move(headerGuard.append("_HPP")) };
}


/****************
 * GRAMMAR DATA *
 ****************/

void tokenDecl(ostream& out) {
  out << R"(struct Token {
      string name;
      string type;
      int precedence;
      Assoc assoc;
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

void gdDecl(ostream& out, const GrammarData& gd) {
  out << R"(struct GrammarData {
      vector<Token> tokens;
      vector<Concrete> concretes;
      vector<Variable> variables;
    };
    )";
  out << "GrammarData GRAMMAR_DATA = ";
  toCode(out, gd);
  out << ';';
}

/****************
 * STACK OBJECT *
 ****************/

void stackObjDef(ostream& out, const GrammarData& gd) {
  out << R"(
      class StackObj {
      public:
        StackObj(int symbol, void* obj, size_t line)
          : symbol_(symbol), obj_(obj), line_(line) {}
        StackObj(StackObj&& other)
          : symbol_(other.symbol_), obj_(other.obj_), line_(other.line_), released_(other.released_) {
          other.obj_ = nullptr;
        }
        StackObj(const StackObj& other) = delete;
        StackObj& operator=(const StackObj& other) = delete;
        StackObj& operator=(StackObj&& other) = delete;
    )";

  // Destructor
  out << R"(
      ~StackObj() {
        if (!obj_) {
          return;
        }

        switch (symbol_) {
    )";
  size_t numTokens = gd.tokens.size();
  // Case statement for each token with data
  for (size_t i = 0; i < numTokens; ++i) {
    const Token& token = gd.tokens[i];
    if (!token.type.empty()) {
      out << "case " << tokenToFromIndex(i) << ':';
      if (!token.dtorStmt.empty()) {
        out << "if (!released_) {";
        replaceAll(
            out,
            token.dtorStmt,
            "#obj",
            "(*static_cast<" + token.type + "*>(obj_))");
        out << '}';
      }
      out << "delete static_cast<" << token.type << "*>(obj_); break;";
    }
  }

  size_t numVars = gd.variables.size();
  // Case statement for each variable
  for (size_t i = 0; i < numVars; ++i) {
    const Variable& var = gd.variables[i];
    out << "case " << i << ':';
    if (!var.dtorStmt.empty()) {
      out << "if (!released_) {";
      replaceAll(
          out, var.dtorStmt, "#obj", "(*static_cast<" + var.type + "*>(obj_))");
      out << '}';
    }
    out << "delete static_cast<" << var.type << "*>(obj_); break;";
  }
  out << "default: return;}}";

  out << R"(
        void* releaseObj() noexcept {
          released_ = true;
          return obj_;
        }
        int getSymbol() const noexcept { return symbol_; }
        size_t getLine() const noexcept { return line_; }
        void* getObj() const noexcept { return obj_; }
        void unrelease() noexcept { released_ = false; }

      private:
        const int symbol_;
        void* obj_;
        size_t line_;
        bool released_ = false;
      };
    )";
}

void startDecl(ostream& out, const GrammarData& gd) {
  string type = gd.variables[1].type;
  out << "struct Start {"
         "Start("
      << type << "&& r) : r_(move(r)) {}" << type << " r_; };";
}

void streamSymbolNames(
    ostream& out,
    const vector<intptr_t>& symbols,
    const GrammarData& gd) {
  vector<string> symbolNames;
  transform(
      symbols.begin(),
      symbols.end(),
      back_inserter(symbolNames),
      [&gd](int symbol) { return symbolToString(symbol, gd); });
  out << symbolNames;
}


string convertArgNum(
    string_view argIndexStr,
    int argIndex,
    const Concrete& concrete,
    const GrammarData& gd) {
  const vector<intptr_t>& argSymbols = concrete.argSymbols;
  // These are user-provided numbers, so check the bounds
  if (argIndex < 0) {
    stringstream& error = errorStore.addError(concrete.declLine);
    error << "Index " << argIndex << " is < 0 for rule ";
    streamSymbolNames(error, argSymbols, gd);
    error << '\n';
    return "";
  }
  if ((size_t)argIndex >= argSymbols.size()) {
    stringstream& error = errorStore.addError(concrete.declLine);
    error << "Index " << argIndex
          << " is greater than the number of elements in rule ";
    streamSymbolNames(error, argSymbols, gd);
    error << '\n';
    return "";
  }

  int argSymbol = argSymbols[argIndex];
  string symbolName = isToken(argSymbol)
                          ? gd.tokens[tokenToFromIndex(argSymbol)].type
                          : gd.variables[argSymbol].type;
  // Make sure the symbol has data associated with it (only necessary for
  // tokens)
  if (symbolName.empty()) {
    stringstream& error = errorStore.addError(concrete.declLine);
    error << "Token " << symbolToString(argSymbol, gd)
          << " is passed as an argument, but has no data associated with it.\n";
    return "";
  }

  return string("(*static_cast<")
      .append(symbolName)
      .append("*>(args[")
      .append(argIndexStr)
      .append("].releaseObj()))");
}

void replacePounds(
    ostream& out,
    const Concrete& concrete,
    const GrammarData& gd) {
  size_t i = 0;
  string_view ctor = concrete.ctorExpr;
  size_t len = ctor.size();
  const char line[] = "line";

  while (i < len) {
    if (ctor[i] == '#') {
      ++i;
      string_view afterPound = ctor.substr(i);
      if (afterPound.starts_with(line)) {
        out << "args[0].getLine()";
        i += sizeof(line);
      } else {
        size_t numDigits;
        int argIndex = stoi(string(afterPound), &numDigits);
        i += numDigits;
        out << convertArgNum(
            afterPound.substr(0, numDigits), argIndex, concrete, gd);
      }
    } else {
      out << ctor[i++];
    }
  }
}

void constructObjFn(ostream& out, const GrammarData& gd) {
  out << R"(void* constructObj(int concrete, StackObj* args) {
      switch (concrete) {)";
  size_t numConcretes = gd.concretes.size();
  // Case statement for each concrete except SCONC (handled specially below)
  for (size_t i = 1; i < numConcretes; ++i) {
    const Concrete& concrete = gd.concretes[i];
    const Variable& var = gd.variables[concrete.varType];
    out << "case " << i << ": return new " << var.type << '(';

    // Catch stoi aerrors
    try {
      replacePounds(out, concrete, gd);
    } catch (const invalid_argument& e) {
      errorStore.addError(concrete.declLine, "Invalid argument #");
    } catch (const out_of_range& e) {
      errorStore.addError(concrete.declLine, "Argument # out of range of int");
    }
    out << ");";
  }

  // Root type of grammar is the first type listed
  out << "case 0: return new Start(move(*static_cast<" << gd.variables[1].type
      << "*>(args[0].releaseObj())));";
  out << R"(default: throw ParseException("Can't construct object. Parser programmer error.");}})";
}

void constructFn(ostream& out) {
  out << R"(StackObj construct(int concrete, vector<StackObj>& stk, size_t reduceStart) {
      size_t line = reduceStart == stk.size() ? 0 : stk[reduceStart].getLine();
      return StackObj(
          GRAMMAR_DATA.concretes[concrete].varType,
          constructObj(concrete, &stk[reduceStart]),
          line);
    })";
}

void constructTokenObjFn(ostream& out, const GrammarData& gd) {
  out << R"(optional<StackObj> constructTokenObj(int token, string_view str) {
      switch (token) {)";
  size_t numTokens = gd.tokens.size();
  for (size_t i = 0; i < numTokens; ++i) {
    const Token& token = gd.tokens[i];
    if (token.precedence == SKIP_TOKEN) {
      out << "case " << tokenToFromIndex(i) << ": return {};";
    } else if (!token.type.empty()) {
      out << "case " << tokenToFromIndex(i) << ':'
          << "return { StackObj(token, "
             "new "
          << token.type << '(';
      replaceAll(out, token.ctorExpr, "#str", "str");
      out << "), currentLine) }; break;";
    }
  }
  out << R"(default: return { StackObj(token, nullptr, currentLine) }; }})";
}

/********
 * DFAs *
 ********/
void lexerDFA(ostream& out, const GrammarData& gd) {
  out << "namespace lexer {";
  try {
    mergedRgxDFAToCode(out, gd);
  } catch (const regex_parser::ParseException& e) {
    errorStore.addError(0, e.what());
  }
  out << '}';
}

void parserDFA(
    ostream& out,
    const GrammarData& gd,
    const ParseFlags& parseFlags) {
  out << "namespace parser {";
  condensedDFAToCode(out, gd, parseFlags);
  out << '}';
}

/**********
 * LEXING *
 **********/

void currentLineDecl(ostream& out) { out << "static size_t currentLine = 1;"; }

void tokenizeFn(ostream& out) {
  out << R"(
      optional<StackObj> getToken(string_view& input, bool& err) {
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
          err = true;
          return {};
        }

        optional<StackObj> optStackObj =
            constructTokenObj(lastAcceptingToken, input.substr(0, lastAcceptingPos));
        input = input.substr(lastAcceptingPos);
        currentLine += lastAcceptingNewlineCount;
        return optStackObj;
      }


      vector<StackObj> tokenize(const string& input) {
        if (input.empty()) {
          return {};
        }

        vector<StackObj> tokens;
        string_view inputView = input;

        while (!inputView.empty()) {
          bool err = false;
          optional<StackObj> optStackObj = getToken(inputView, err);
          if (err) {
            ostringstream error;
            vector<string> prevTokenNames;
            auto startIter =
                tokens.size() < 25 ? tokens.cbegin() : tokens.cend() - 25;
            transform(
                move(startIter),
                tokens.cend(),
                back_inserter(prevTokenNames),
                [](const StackObj& stackObj) { return GRAMMAR_DATA.tokens[tokenToFromIndex(stackObj.getSymbol())].name; });
            error << "Lexer \033[1;31merror\033[0m on line " << currentLine << " at: " << inputView.substr(0, 25) << '\n'
                << "Previous tokens were: " << prevTokenNames;
            throw ParseException(error.str());
          }

          if (optStackObj.has_value()) {
            tokens.push_back(move(*optStackObj));
          }
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

void tokenizeDecl(ostream& out) {
  out << R"(
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
    };
    )";
}

void parseDecl(ostream& out, const GrammarData& gd) {
  const string& rootType = gd.variables[1].type;
  out << rootType << " parse(const std::string& input);" << rootType
      << " parse(std::istream& input);";
}

void parseExceptionDecl(ostream& out) {
  out << R"(class ParseException : public std::exception {
      public:
        ParseException(const std::string& errMsg) : errMsg_(errMsg) {}
        ParseException(const char* errMsg) : errMsg_(errMsg) {}
        const char* what() const noexcept override {
          return errMsg_.c_str();
        }
      private:
        std::string errMsg_;
      };
      )";
}

void parseHelperFns(ostream& out) {
  out << R"(
      void parseError(
          vector<StackObj>& stk,
          const vector<StackObj>& inputTokens,
          size_t tokenPos) {

        for_each(stk.begin(), stk.end(), mem_fun_ref(&StackObj::unrelease));

        ostringstream errMsg;
        vector<string> stkSymbolNames;
        vector<string> remainingTokenNames;
        auto stkObjToName = [](const StackObj& stkObj) {
          if (isToken(stkObj.getSymbol())) {
            return GRAMMAR_DATA.tokens[tokenToFromIndex(stkObj.getSymbol())].name;
          }
          return GRAMMAR_DATA.variables[stkObj.getSymbol()].name;
        };

        transform(
            stk.begin(), stk.end(), back_inserter(stkSymbolNames), stkObjToName);
        transform(
            inputTokens.begin() + tokenPos,
            inputTokens.end(),
            back_inserter(remainingTokenNames),
            stkObjToName);

        errMsg << "Parse \033[1;31merror\033[0m on line " << stk.back().getLine() << ":\n\tStack: " << stkSymbolNames
              << "\n\tRemaining tokens: " << remainingTokenNames;
        throw ParseException(errMsg.str());
      }
    )";
}

void tryReduceFn(ostream& out) {
  out << R"(int tryReduce(
        const parser::Node* node,
        int nextToken,
        vector<StackObj>& stk,
        const vector<StackObj>& inputTokens,
        size_t tokenPos) {
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
        return stkObj.getSymbol() == symbol;
      })) {
        return NONE;
      }
      if (!node->ts_.contains(nextToken)) {
        return rule.concrete;
      }

      const Token& nextTokenObj = GRAMMAR_DATA.tokens[tokenToFromIndex(nextToken)];
      int shiftPrecedence = nextTokenObj.precedence;

      if (ruleData.precedence == NONE && shiftPrecedence == NONE) {
        return NONE;
      }

      if (ruleData.precedence > shiftPrecedence) {
        return rule.concrete;
      }
      if (ruleData.precedence == shiftPrecedence) {
        if (nextTokenObj.assoc == Assoc::LEFT) {
          return rule.concrete;
        }
        if (nextTokenObj.assoc == Assoc::NOT) {
          parseError(stk, inputTokens, tokenPos);
        }
      }
      return NONE;
    })";
}

void shiftReduceFn(ostream& out, const GrammarData& gd) {
  out << gd.variables[1].type << R"(shiftReduce(vector<StackObj>& inputTokens) {
        // vector<StackObj> stk;
        // if (inputTokens.empty()) {
        //   parseError(stk, inputTokens, 0);
        // }

        // stk.push_back(move(inputTokens[0]));
        // vector<parser::Node*> dfaPath = { parser::root.get() };
        // size_t i = 1;
        // size_t inputSize = inputTokens.size();





        vector<StackObj> stk;
        vector<parser::Node*> dfaPath;
        size_t i = 0;
        size_t inputSize = inputTokens.size();

        while (!(i == inputSize && stk.size() == 1 && stk[0].getSymbol() == S)) {
          parser::Node* currentNode =
              stk.empty()
                  ? parser::root.get()
                  : dfaPath.back()->step(stk.back().getSymbol());
          if (currentNode == nullptr) {
            parseError(stk, inputTokens, i);
          }
          dfaPath.push_back(currentNode);

          int nextInputToken = i == inputSize ? NONE : inputTokens[i].getSymbol();
          int concrete =
              tryReduce(currentNode, nextInputToken, stk, inputTokens, i);
          if (concrete != NONE) {
            size_t reduceStart =
                stk.size() - currentNode->v_.reducibleRule->symbols.size();
            StackObj newObj = construct(concrete, stk, reduceStart);
            size_t stkSize = stk.size();
            for (size_t j = 0; j < stkSize - reduceStart; ++j) {
              stk.pop_back();
              dfaPath.pop_back();
            }
            stk.push_back(move(newObj));
          } else {
            if (nextInputToken == NONE) {
              parseError(stk, inputTokens, i);
            }
            stk.push_back(move(inputTokens[i]));
            ++i;
          }
        }

        Start* start = static_cast<Start*>(stk[0].releaseObj());
        return move(start->r_);
      }
    )";
}

void parseFn(ostream& out, const GrammarData& gd) {
  const string& rootType = gd.variables[1].type;
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
      #include <iostream>
      #include <string>
      #include <vector>
    )";
}

void cppIncludes(ostream& out) {
  out << R"(
      #include <algorithm>
      #include <climits>
      #include <cstddef>
      #include <functional>
      #include <memory>
      #include <optional>
      #include <sstream>
      #include <stdexcept>
      #include <streambuf>
      #include <string_view>
      #include <unordered_map>
      #include <vector>

      #include <prez/print_stuff.hpp>
    )";
}

void noneInt(ostream& out) {
  out << R"(
      constexpr int NONE = INT_MIN;
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


/********************
 * DRIVER FUNCTIONS *
 ********************/

constexpr char generatedWarning[] =
    "/* GENERATED FILE. DO NOT OVERWRITE BY HAND. */\n";

string parserHppCode(
    const string& namespaceName,
    const string& headerGuard,
    const string& addlHdrIncludes,
    const GrammarData& gd) {
  stringstream out;

  out << "#ifndef " << headerGuard << "\n#define " << headerGuard << '\n'
      << endl;
  out << generatedWarning;
  out << addlHdrIncludes;
  parserHppIncludes(out);
  out << "namespace " << namespaceName << '{';
  parseDecl(out, gd);
  parseExceptionDecl(out);
  out << "}\n#endif";

  return out.str();
}

string lexerHppCode(
    const string& namespaceName,
    const string& headerGuard,
    const GrammarData& gd) {
  stringstream out;

  out << "#ifndef " << headerGuard << "\n#define " << headerGuard << '\n'
      << endl;
  out << generatedWarning;
  lexerHppIncludes(out);
  out << "namespace " << namespaceName << '{';
  stackObjDef(out, gd);
  tokenizeDecl(out);
  parseExceptionDecl(out);
  out << "}\n#endif";

  return out.str();
}

string parserCppCode(
    const ParseFlags& parseFlags,
    const string& namespaceName,
    const ParseInfo& parseInfo) {
  stringstream out;
  const GrammarData& gd = parseInfo.gd;

  out << generatedWarning;
  out << "#include \"" << parseFlags.parserFilePath << ".hpp\"\n";
  cppIncludes(out);
  out << parseInfo.addlCppCode << "using namespace std;"
      << "using namespace " << namespaceName << ";"
      << "namespace {";
  noneInt(out);
  sInt(out);
  isTokenFn(out);
  tokenToFromIndexFn(out);
  assocDecl(out);
  tokenDecl(out);
  concreteDecl(out);
  variableDecl(out);
  gdDecl(out, gd);
  startDecl(out, gd);
  stackObjDef(out, gd);
  currentLineDecl(out);
  constructObjFn(out, gd);
  constructFn(out);
  constructTokenObjFn(out, gd);
  lexerDFA(out, gd);
  dfaRuleDecl(out);
  ruleDataDecl(out);
  parserDFA(out, gd, parseFlags);
  tokenizeFn(out);
  parseHelperFns(out);
  tryReduceFn(out);
  shiftReduceFn(out, gd);
  out << "} namespace " << namespaceName << '{';
  parseFn(out, gd);
  out << '}';

  return out.str();
}

string lexerCppCode(
    const string& lexerFilePath,
    const string& namespaceName,
    const string& addlCode,
    const GrammarData& gd) {
  stringstream out;

  out << generatedWarning;
  out << "#include \"" << lexerFilePath << ".hpp\"\n";
  cppIncludes(out);
  out << addlCode << "using namespace std;"
      << "using namespace " << namespaceName << ";"
      << "using namespace " << namespaceName << ';' << "namespace {";
  noneInt(out);
  tokenToFromIndexFn(out);
  assocDecl(out);
  tokenDecl(out);
  concreteDecl(out);
  variableDecl(out);
  gdDecl(out, gd);
  currentLineDecl(out);
  constructTokenObjFn(out, gd);
  lexerDFA(out, gd);
  out << "} namespace " << namespaceName << '{';
  tokenizeFn(out);
  tokenizeFileFn(out);
  out << '}';

  return out.str();
}

}  // namespace

void generateParserCode(
    const ParseInfo& parseInfo,
    const ParseFlags& parseFlags) {
  const string& parserFilePath = parseFlags.parserFilePath;

  auto thePair = getNamespaceAndGuard(parserFilePath);
  const string& namespaceName = thePair.first;
  const string& headerGuard = thePair.second;

  ofstream hppFile(parserFilePath + ".hpp");
  if (!hppFile.is_open()) {
    throw invalid_argument("Could not open file " + parserFilePath + ".hpp");
  }
  ofstream cppFile(parserFilePath + ".cpp");
  if (!cppFile.is_open()) {
    throw invalid_argument("Could not open file " + parserFilePath + ".cpp");
  }

  string hppCode = parserHppCode(
      namespaceName, headerGuard, parseInfo.addlHppCode, parseInfo.gd);
  string cppCode = parserCppCode(parseFlags, namespaceName, parseInfo);
  errorStore.displayErrors();
  hppFile << hppCode;
  cppFile << cppCode;
}


void generateLexerCode(
    const string& lexerFilePath,
    const string& addlCode,
    const GrammarData& gd) {
  auto thePair = getNamespaceAndGuard(lexerFilePath);
  const string& namespaceName = thePair.first;
  const string& headerGuard = thePair.second;

  ofstream hppFile(lexerFilePath + ".hpp");
  if (!hppFile.is_open()) {
    throw invalid_argument("Could not open file " + lexerFilePath + ".hpp");
  }
  ofstream cppFile(lexerFilePath + ".cpp");
  if (!cppFile.is_open()) {
    throw invalid_argument("Could not open file " + lexerFilePath + ".cpp");
  }

  string hppCode = lexerHppCode(namespaceName, headerGuard, gd);
  string cppCode = lexerCppCode(lexerFilePath, namespaceName, addlCode, gd);
  errorStore.displayErrors();
  hppFile << hppCode;
  cppFile << cppCode;
}
