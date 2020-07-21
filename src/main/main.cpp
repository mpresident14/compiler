#include "src/assembly/assembly.hpp"
#include "src/assembly/reg_alloc/flow_graph.hpp"
#include "src/assembly/temp.hpp"
#include "src/intermediate/intermediate.hpp"
#include "src/language/language.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/main/parser.hpp"

#include <string.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

// TODO: Generate more informative parser errors by analyzing stack and
// remaining tokens. Can also update the parser code itself (e.g., "expected
// <token>" or "no reduction" error)

/* Return true if no errors */
bool compile(const string& srcFilename, const string& asmFilename, vector<string> builtInFiles) {
  unordered_map<string, unique_ptr<language::Program>> initializedProgs;
  Logger logger;
  bool hasErr = false;
  try {
    // Parse builtins first so that their function IDs will match the precompiled assembly
    vector<language::Program> builtIns;
    auto classIds = make_shared<unordered_map<int, Ctx::ClassInfo*>>();
    for (string builtInFile : builtInFiles) {
      fs::path builtInPath = fs::canonical(builtInFile);
      language::Program prog = parser::parse(builtInPath);
      prog.initContext(builtInPath, initializedProgs, {}, classIds);
      builtIns.push_back(move(prog));
    }

    // Mark as initiailized before recursing to allow circular dependencies
    error_code ec;
    fs::path srcPath = fs::canonical(srcFilename, ec);
    if (ec) {
      logger.logFatal(0, ec.message().append(" \"").append(srcFilename).append(1, '"'));
    }

    auto iter = initializedProgs
                    .emplace(srcPath, make_unique<language::Program>(parser::parse(srcFilename)))
                    .first;
    // Recursively record all declarations
    iter->second->initContext(srcPath, initializedProgs, builtIns, classIds);


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
  } catch (const parser::ParseException& e) {
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


int main(int argc, char** argv) {
  char* srcFile = argv[1];
  char* asmFile = argv[2];

  // For built-in files
  if (argc == 4) {
    return compile(srcFile, asmFile, {});
  }

#ifndef __INTELLISENSE__
  return compile(srcFile, asmFile, { IMPORT_DIR "/string.prez" });
#endif
}
