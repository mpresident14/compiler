#include "generate.hpp"
#include "config_parse.hpp"

#include <iostream>

using namespace std;

// TODO: Add namespaces to all code

int main(int argc, char** argv) {
  // TODO: Use getopt here, make sure file as .gp extension
  if (argc != 3) {
    cerr << "Enter a config and destination file." << endl;
    return 1;
  }

  ParseInfo parseInfo = parseConfig(argv[1]);
  generateParserCode(argv[2], parseInfo.addlHppCode, parseInfo.addlCppCode, parseInfo.grammarData);
}
