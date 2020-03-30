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


void toCode(ostream& out,const Token& token) {
  out << '{';
  toCode(out, token.name);
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

void toCode(ostream& out,const GrammarData& grammarData) {
  out << '{';
  toCode(out, grammarData.tokens);
  out << ',';
  toCode(out, grammarData.concretes);
  out << '}';
  // Variables not needed for shift reducing
}


void tokenDecl(ostream& out) {
  out << R"(struct Token {
    string name;
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

void grammarDataDecl(ostream& out, const GrammarData& grammarData) {
  out << R"(struct GrammarData {
    vector<Token> tokens;
    vector<Concrete> concretes;
  };
  )";
  out << "GrammarData GRAMMAR_DATA = ";
  toCode(out, grammarData);
  out << ';';
}


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
 * Driver Functions *
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
  grammarDataDecl(out, grammarData);


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
