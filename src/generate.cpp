#include "build_parser.hpp"
#include "regex_eval.hpp"
// TODO: remove when done testing
#include "test/lr1_grammar.hpp"

#include <fstream>
#include <ostream>
#include <cstddef>
#include <string>
#include <sstream>

using namespace std;

/*********
 * UTILS *
 *********/

void toCode(ostream& out, const string& str) {
  out << '"' << str << '"';
}

void toCode(ostream& out, int n) {
  out << to_string(n);
}

void toCode(ostream& out, const Assoc& assoc) {
  out << "Assoc::" << assoc;
}

template <typename Container>
void toCode(ostream& out, const Container& c) {
  out << '{';
  for_each(c.cbegin(), c.cend(), [&out](auto item) {
        toCode(out, item);
        out << ',';
      });
  out << '}';
}


/****************
 * GRAMMAR DATA *
 ****************/

void toCode(ostream& out,const Token& token) {
  out << '{';
  toCode(out, token.name);
  out << ',';
  toCode(out, token.type);
  out << ',';
  toCode(out, token.precedence);
  out << '}';
  // Other fields not needed for shift-reducing
}

void toCode(ostream& out,const Concrete& concrete) {
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

void toCode(ostream& out,const GrammarData& grammarData) {
  out << '{';
  toCode(out, grammarData.tokens);
  out << ',';
  toCode(out, grammarData.concretes);
  out << ',';
  toCode(out, grammarData.variables);
  out << '}';
}


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
  };
  )";
}

void sObjDecl(ostream& out, const GrammarData& grammarData) {
  vector<string> rootType = {grammarData.variables[1].type};
  const char decl[] = R"(struct SObj {
    SObj(#0* r) : r_(r) {}
    ~SObj() { delete r_; }
    #0* r_;
  };
  )";
  replaceNumbersVec(out, decl, rootType);
}


void deleteObjFn(ostream& out, const GrammarData& grammarData) {
  out << R"(void deleteObj(const StackObj& s) {
    switch (s.symbol) {)";

  size_t numVars = grammarData.variables.size();
  size_t numTokens = grammarData.tokens.size();
  // Case statement for each token with data
  for (size_t i = 0; i < numTokens; ++i) {
    const Token& token = grammarData.tokens[i];
    if (!token.type.empty()) {
      out << "case " << indexToSymbol(i, numVars) << ':';
      replaceNumbersVec(out, token.dtorStmt, {"*(" + token.type + "*) s.obj"});
      out << " delete (" << token.type << "*) s.obj; break;";
    }
  }

  // Case statement for each variable except S
  for (size_t i = 1; i < numVars; ++i) {
    const Variable& var = grammarData.variables[i];
    out << "case " << i << ':';
    replaceNumbersVec(out, var.dtorStmt, {"*(" + var.type + "*) s.obj"});
    out << " delete (" << var.type << "*) s.obj; break;";
  }

  out << "default: return;}}";
}


void deleteObjPtrFn(ostream& out, const GrammarData& grammarData) {
  out << R"(void deleteObjPtr(const StackObj& s) {
    switch (s.symbol) {)";

  size_t numVars = grammarData.variables.size();
  size_t numTokens = grammarData.tokens.size();
  // Case statement for each token with data
  for (size_t i = 0; i < numTokens; ++i) {
    const Token& token = grammarData.tokens[i];
    if (!token.type.empty()) {
      out << "case " << indexToSymbol(i, numVars) << ':'
          << " delete (" << token.type << "*) s.obj; break;";
    }
  }

  // Case statement for each variable except S
  for (size_t i = 1; i < numVars; ++i) {
    const Variable& var = grammarData.variables[i];
    out << "case " << i << ':'
        << " delete (" << var.type << "*) s.obj; break;";
  }

  out << "default: return;}}";
}


void constructObjFn(ostream& out, const GrammarData& grammarData) {
  out << R"(void* constructObj(int concrete, StackObj* args) {
    switch (concrete) {)";
  // auto convertNum = [&grammarData](const string& digits){
  //   return "*(" + grammarData.
  // }

  size_t numConcretes = grammarData.concretes.size();
  // Case statement for each concrete except SCONC (handled specially below)
  for (size_t i = 1; i < numConcretes; ++i) {
    const Concrete& concrete = grammarData.concretes[i];
    const Variable& var = grammarData.variables[concrete.varType];
    out << "case " << i << ": return new " << var.type << '(';

    replaceNumbers(out, concrete.ctorExpr, [&concrete, &grammarData](const string& digits){
      int argSymbol = concrete.argSymbols[stoi(digits)];
      string symbolName;
      if (isToken(argSymbol)) {
        symbolName = grammarData.tokens[tokensIndex(argSymbol)].type;
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
  out << "case 0: return new SObj((" << grammarData.variables[1].type << "*)args[0].obj);";
  out << R"(default: throw invalid_argument("Can't construct. Out of options.");}})";
}


/********
 * MISC *
 ********/

void includes(ostream& out) {
  out << R"(#include <vector>
    #include <string>
    #include <iostream>
    #include <cstddef>
    #include <algorithm>
    #include <functional>
    #include <optional>
    #include <stdexcept>
  )";
}


/********************
 * DRIVER FUNCTIONS *
 ********************/
string hppCode(const string& classFile) {
  stringstream out;

  out << "#include \"" + classFile + "\"\n";
  includes(out);

  return out.str();
}

string cppCode(const GrammarData& grammarData) {
  stringstream out;

  out << R"(#include "parser.hpp"
    using namespace std;
  )";
  tokenDecl(out);
  concreteDecl(out);
  variableDecl(out);
  grammarDataDecl(out, grammarData);
  stackObjDecl(out);
  sObjDecl(out, grammarData);
  deleteObjPtrFn(out, grammarData);
  deleteObjFn(out, grammarData);
  constructObjFn(out, grammarData);



  // condensedDFAToCode(outFile, GRAMMAR_DATA);
  return out.str();
}


void generateCode(const string& classFile, const GrammarData& grammarData) {
  std::ofstream hppFile;
  hppFile.open("parser.hpp");
  hppFile << hppCode(classFile);
  hppFile.close();

  std::ofstream cppFile;
  cppFile.open("parser.cpp");
  cppFile << cppCode(grammarData);
  cppFile.close();
}


int main() {
  generateCode("test/lr1_grammar.hpp", GRAMMAR_DATA);

  return 0;
}
