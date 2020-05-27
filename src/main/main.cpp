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


void compile(const std::string& srcFile, const std::string& outAsmFile) {
  // TODO: Open with APPEND when we implement import
  std::ifstream in(srcFile);
  if (!in.is_open()) {
    throw std::invalid_argument(std::string("Unable to open ").append(srcFile).append(": ").append(strerror(errno)));
  }
  std::ofstream out(outAsmFile);
  if (!out.is_open()) {
    throw std::invalid_argument(std::string("Unable to open ").append(outAsmFile).append(": ").append(strerror(errno)));
  }

  Context& ctx = Context::getContext();
  ctx.addLogger(srcFile);
  try {
    lg::Program prog = parser::parse(in);
    prog.toImProg().toAssemProg().toCode(out);
  } catch (const parser::ParseException& e) {
    ctx.currentLogger().logError(0, e.what());
  } catch (const std::exception& e) {
    // Any exception from the translating code will already be logged, so do nothing
  }
}


int main(int, char** argv) {
  try {
    compile(argv[1], argv[2]);
    Context::getContext().displayLog();
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
