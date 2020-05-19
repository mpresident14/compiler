#include "src/parser/config_parse.hpp"
#include "src/parser/generate.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv) {
  const char* errMsg =
      "Usage: parsegen -g <grammar_file> [-f <output_file_basename>]";

  string outputFile;
  string gpFile;
  char c;
  while ((c = getopt(argc, argv, "g:f:")) != -1) {
    switch (c) {
      case 'g':
        gpFile = optarg;
        break;
      case 'f':
        outputFile = optarg;
        break;
      default:
        cerr << errMsg << endl;
        return 1;
    }
  }

  if (gpFile.empty()) {
    cerr << errMsg << endl;
    return 1;
  }

  if (!gpFile.ends_with(".pgen")) {
    cerr << "Grammar file must have a .pgen extension" << endl;
    return 1;
  }

  if (outputFile.empty()) {
    outputFile = gpFile.substr(0, gpFile.size() - 5) + "_parser";
  }

  ParseInfo parseInfo = parseConfig(gpFile);
  generateParserCode(outputFile, parseInfo);
}
