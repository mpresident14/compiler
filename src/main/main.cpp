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

using namespace std;


void compile(ifstream& srcFile, ofstream& outAsmFile) {
  language::Program prog = parser::parse(srcFile);
  prog.toImProg().toAssemProg().toCode(outAsmFile);
}


int main(int, char** argv) {
  char* srcFile = argv[1];
  char* asmFile = argv[2];

  ifstream in(srcFile);
  ofstream out(asmFile);

  try {
    Logger& logger = ctx::addLogger(srcFile);
    logger.checkFile(srcFile, in);
    logger.checkFile(asmFile, out);
    compile(in, out);
    if (ctx::displayLogs()) {
      return 1;
    }
    return 0;
  } catch (const parser::ParseException& e) {
    // Error from the parser
    ctx::getLogger().logError(0, e.what());
    ctx::displayLogs();
    return 1;
  } catch (Logger::Exception& e) {
    // Fatal errors from the translator (already logged)
    ctx::displayLogs();
    return 1;
  }
}
