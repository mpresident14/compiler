#include "src/assembly/assembly.hpp"
#include "src/assembly/reg_alloc/flow_graph.hpp"
#include "src/assembly/temp.hpp"
#include "src/intermediate/intermediate.hpp"
#include "src/language/language.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/main/parser.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;


/* Return true if no errors */
bool compile(string_view srcFilename, string_view asmFilename) {
  unordered_map<string, language::Program> initializedProgs;
  Logger logger;
  bool hasErr = false;
  try {
    // string_view::data() OK
    ifstream srcFile(srcFilename.data());
    logger.checkFile(srcFilename, srcFile);

    // Mark as initiailized before recursing to allow circular dependencies
    // TODO: Catch and log (fatal?) parse errors
    auto iter =
        initializedProgs.emplace(srcFilename, parser::parse(srcFile)).first;


    // Recursively record all declarations
    iter->second.initContext(
        srcFilename,
        initializedProgs,
        make_shared<unordered_map<string, std::string>>(),
        make_shared<unordered_map<string, std::string>>());

    // Convert to assembly
    vector<assem::Program> assemProgs;
    for (const auto& [filename, prog] : initializedProgs) {
      assemProgs.push_back(prog.toAssemProg());
      // Print any warnings or non-fatal errors
      hasErr |= prog.getCtx().hasErrors();
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
  } catch (const Logger::Exception&) {
    hasErr = true;
  } catch (const parser::ParseException& e) {
    hasErr = true;
    cerr << e.what() << endl;
  }

  logger.streamLog();
  for (const auto& [filename, prog] : initializedProgs) {
    prog.getCtx().displayLogs();
  }

  return hasErr;
}


int main(int, char** argv) {
  char* srcFile = argv[1];
  char* asmFile = argv[2];

  return compile(srcFile, asmFile);
}
