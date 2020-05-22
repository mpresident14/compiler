#include "src/parser/config_parse.hpp"
#include "src/parser/generate.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv) {
  const char* errMsg =
      "Usage: parsegen -g <grammar_file> [-f <output_file_base>] [-l <log_file>]";

  string pgenFile;
  string parserFilePath;
  string logFile;
  char c;
  while ((c = getopt(argc, argv, "g:f:l:")) != -1) {
    switch (c) {
      case 'g':
        pgenFile = optarg;
        break;
      case 'f':
        parserFilePath = optarg;
        break;
      case 'l':
        logFile = optarg;
        break;
      default:
        cerr << errMsg << endl;
        return 1;
    }
  }

  if (pgenFile.empty()) {
    cerr << errMsg << endl;
    return 1;
  }

  if (!pgenFile.ends_with(".pgen")) {
    cerr << "Grammar file must have a .pgen extension" << endl;
    return 1;
  }

  if (parserFilePath.empty()) {
    parserFilePath = pgenFile.substr(0, pgenFile.size() - 5) + "_parser";
  }

  try {
    ParseInfo parseInfo = parseConfig(pgenFile);
    generateParserCode(parseInfo, { parserFilePath, logFile });
  } catch (exception& e) {
    cerr << pgenFile << ":\n"<< e.what() << endl;
  }
}
