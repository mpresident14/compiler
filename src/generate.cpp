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
    int symbol;
    int concrete;
  };
  )";
}


void deleteObjCode(ostream& out, const GrammarData& grammarData) {
  out << R"(void deleteObj(const StackObj& s) {
    switch (s.symbol) {)";

  size_t numVars = grammarData.variables.size();
  size_t numTokens = grammarData.tokens.size();
  // Case statement for each token with data
  for (size_t i = 0; i < numTokens; ++i) {
    const Token& token = grammarData.tokens[i];
    if (!token.type.empty()) {
      out << "case " << indexToSymbol(i, numVars) << ':';
      replaceNumbers(out, token.dtorStmt, {"*(" + token.type + "*) s.obj"});
      out << " delete (" << token.type << "*) s.obj; break;";
    }
  }

  // Case statement for each variable except S
  for (size_t i = 1; i < numVars; ++i) {
    const Variable& var = grammarData.variables[i];
    out << "case " << i << ':';
    replaceNumbers(out, var.dtorStmt, {"*(" + var.type + "*) s.obj"});
    out << " delete (" << var.type << "*) s.obj; break;";
  }

  out << "default: return;}}";
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
string hppCode() {
  stringstream out;

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
  deleteObjCode(out, grammarData);



  // condensedDFAToCode(outFile, GRAMMAR_DATA);
  return out.str();
}


void generateCode(const GrammarData& grammarData) {
  std::ofstream hppFile;
  hppFile.open("parser.hpp");
  hppFile << hppCode();
  hppFile.close();

  std::ofstream cppFile;
  cppFile.open("parser.cpp");
  cppFile << cppCode(grammarData);
  cppFile.close();
}


int main() {
  generateCode(GRAMMAR_DATA);

  return 0;
}
