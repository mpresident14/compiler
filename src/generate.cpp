#include "build_parser.hpp"
#include "regex_eval.hpp"
// TODO: remove when done testing
#include "test/lr1_grammar.hpp"

#include <fstream>
#include <ostream>
#include <cstddef>
#include <string>

using namespace std;


void generateParseDFA() {
  std::ofstream outFile;
  outFile.open("test/gen_parser.hpp");
  condensedDFAToCode(outFile, GRAMMAR, GRAMMAR_DATA);
  outFile.close();
}


int main()
{
  generateParseDFA();

  return 0;
}
