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


namespace lg = language;


void compile(std::ifstream& srcFile, std::ofstream& outAsmFile) {
  lg::Program prog = parser::parse(srcFile);
  prog.toImProg().toAssemProg().toCode(outAsmFile);
}


int main(int, char** argv) {
  std::string srcFile(argv[1]);

  std::ifstream in(srcFile);
  if (!in.is_open()) {
    std::cerr << "Unable to open " << srcFile << ": " << strerror(errno) << std::endl;
  }

  std::ofstream out(argv[2]);
  if (!out.is_open()) {
    std::cerr << "Unable to open " << argv[2] << ": " << strerror(errno) << std::endl;
  }

  Context& ctx = Context::getContext();
  ctx.addLogger(srcFile);

  try {
    compile(in, out);
    if (ctx.displayLogs()) {
      return 1;
    }
    return 0;
  } catch (const parser::ParseException& e) {
    // Error from the parser
    ctx.currentLogger().logError(0, e.what());
    ctx.displayLogs();
    return 1;
  } catch (Logger::Exception& e) {
    // Fatal errors from the translator (already logged)
    ctx.displayLogs();
    return 1;
  }
}
