#include "src/intermediate/intermediate.hpp"

#include <stdexcept>
#include <memory>

using namespace std;

namespace im {


assem::DeclPtr Func::toAssemDecl() const {
  std::vector<assem::InstrPtr> assemInstrs;
  for (const StmtPtr& stmt : stmts) {
    stmt->toAssemInstrs(assemInstrs);
  }
  return make_unique<assem::Decl>(move(assemInstrs));
}


assem::Program Program::toInstrProg() const {
  std::vector<assem::DeclPtr> assemDecls;
  for (const DeclPtr& decl : decls) {
    assemDecls.push_back(decl->toAssemDecl());
  }
  return assem::Program{ name, move(assemDecls) };
}


}
