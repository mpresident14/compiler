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


string toCode(const string& str) {
  return "\"" + str + "\"";
}

string toCode(int n) {
  return to_string(n);
}

template <typename Container>
string& toCode(const Container& c) {
  stringstream out;
  out << '{';
  for_each(c.cbegin(), c.cend(), [&out](auto item) {
        item << toCode(n) << ',';
      });
  out << '}';
}


string toCode(const Token& token) {

}

string toCode(const Concrete& concrete) {

}

string toCode(const Variable& variable) {

}

string toCode(const GrammarData& grammarData) {

}


// void generateParserDFA() {
//   std::ofstream outFile;
//   outFile.open("test/gen_parser.hpp");
//   condensedDFAToCode(outFile, GRAMMAR_DATA);
//   outFile.close();
// }


void generateCode(const GrammarData& grammarData) {
  std::ofstream outFile;
  outFile.open("parser.cpp");


  outFile.close();
}


int main() {
  generateCode(GRAMMAR_DATA);

  return 0;
}
