// #define DEBUG

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

void debugCompile(ifstream& srcFile, ofstream& logFile) {
  language::Program prog = parser::parse(srcFile);
  prog.toImProg().toAssemProg().toCodeWithTemps(logFile);
}

void compile(ifstream& srcFile, ofstream& asmFile) {
  language::Program prog = parser::parse(srcFile);
  // TODO: We don't want to write to the assembly file if there are errors.
  // When we import other files, maybe accumulate assemPrograms in a vector
  // and pass to main function to write to the file if everything checks out.
  prog.toImProg().toAssemProg().toCode(asmFile);
}


int main(int, char** argv) {
  char* srcFile = argv[1];
  char* asmFile = argv[2];
  char* logFile = argv[3];

  ifstream in(srcFile);
  ofstream out(asmFile);
  ofstream log(logFile);


  #ifdef DEBUG
    debugCompile(in, log);
  #else
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
  #endif
}
