#include "src/assembly/assembly.hpp"
#include "src/assembly/reg_alloc/flow_graph.hpp"
#include "src/assembly/temp.hpp"
#include "src/intermediate/intermediate.hpp"
#include "src/language/language.hpp"
#include "src/main/parser.hpp"
#include "src/language/typecheck/context.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <string.h>
#include <unordered_map>

using namespace std;


/* Return true if no errors */
bool compile(string_view srcFilename, string_view asmFilename) {
  Logger logger;

  // string_view::data() OK
  ifstream srcFile(srcFilename.data());
  logger.checkFile(srcFilename, srcFile);
  unordered_map<std::string, language::Program> initializedProgs;

  // Mark as initiailized before recursing to allow circular dependencies
  // TODO: Catch and log (fatal?) parse errors
  auto iter = initializedProgs.emplace(srcFile, parser::parse(srcFile)).first;

  // Convert to assembly
  vector<assem::Program> assemProgs;
  bool hasErr = false;
  // Recursively record all declarations
  iter->second.initContext(srcFilename, initializedProgs);
  for (const auto& [filename, prog] : initializedProgs) {
    assemProgs.push_back(prog.toAssemProg());
    // Print any warnings or non-fatal errors
    hasErr |= prog.getCtx().displayLogs();
  }

  // If we compiled successfully, write the assembly to the file
  if (!hasErr) {
    // string_view::data() OK
    ofstream asmFile(asmFilename.data());
    logger.checkFile(asmFilename, asmFile);
    for (assem::Program& prog : assemProgs) {
      prog.toCode(asmFile);
    }
  }
}


int main(int, char** argv) {
  char* srcFile = argv[1];
  char* asmFile = argv[2];

  try {
    return compile(srcFile, asmFile);
  } catch (const parser::ParseException& e) {
    cerr << e.what() << endl;
    return 1;
  } catch (Logger::Exception& e) {
    cerr << e.what() << endl;
    return 1;
  }
}
