#include "src/intermediate/intermediate.hpp"

#include <stdexcept>
#include <memory>

using namespace std;

namespace im {


/***********
 * Program *
 ***********/
Program::Program(std::vector<DeclPtr>&& decls)
    : decls_(move(decls)) {}

assem::Program Program::toAssemProg() const {
  std::vector<assem::DeclPtr> assemDecls;
  for (const DeclPtr& decl : decls_) {
    assemDecls.push_back(decl->toAssemDecl());
  }
  return assem::Program(move(assemDecls));
}


/********
 * Func *
 ********/

Func::Func(const std::string& name, std::vector<StmtPtr>&& stmts)
    : name_(name), stmts_(move(stmts)) {}


assem::DeclPtr Func::toAssemDecl() const {
  std::vector<assem::InstrPtr> assemInstrs;
  for (const StmtPtr& stmt : stmts_) {
    stmt->toAssemInstrs(assemInstrs);
  }
  return make_unique<assem::Function>(name_, move(assemInstrs));
}









}
