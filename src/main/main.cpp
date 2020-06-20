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
#include <optional>
#include <stdexcept>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// TODO: Generate more informative parser errors by analyzing stack and
// remaining tokens. Can also update the parser code itself (e.g., "expected
// <token>" or "no reduction" error)

/* Return true if no errors */
bool compile(const string& srcFilename, const string& asmFilename) {
  unordered_map<string, unique_ptr<language::Program>> initializedProgs;
  Logger logger;
  bool hasErr = false;
  try {
    // Mark as initiailized before recursing to allow circular dependencies
    auto iter =
        initializedProgs
            .emplace(
                srcFilename,
                make_unique<language::Program>(parser::parse(srcFilename)))
            .first;

    // Recursively record all declarations
    iter->second->initContext(
        srcFilename,
        initializedProgs,
        make_shared<unordered_map<string, string>>(),
        make_shared<unordered_map<string, string>>());

    // Convert to assembly
    vector<assem::Program> assemProgs;
    for (const auto& [filename, prog] : initializedProgs) {
      if (prog) {
        assemProgs.push_back(prog->toAssemProg());
        // Print any warnings or non-fatal errors
        hasErr |= prog->ctx_->hasErrors();
      }
    }

    // If we compiled successfully, write the assembly to the file
    if (!hasErr) {
      ofstream asmFile(asmFilename);
      logger.checkFile(asmFilename, asmFile);
      for (assem::Program& prog : assemProgs) {
        prog.toCode(asmFile);
      }
    }
  } catch (const Logger::Exception&) {
    hasErr = true;
  } /* catch (const runtime_error& e) {
    logger.logError(0, e.what());
  } */ catch (const parser::ParseException& e) {
    hasErr = true;
    cerr << e.what() << endl;
  }

  logger.streamLog();
  for (const auto& [filename, prog] : initializedProgs) {
    if (prog) {
      prog->ctx_->displayLogs();
    }
  }

  return hasErr;
}


int main(int, char** argv) {
  #ifndef __INTELLISENSE__
  language::Program::setImportPath(IMPORT_DIR);
  #endif
  char* srcFile = argv[1];
  char* asmFile = argv[2];

  return compile(srcFile, asmFile);
}
