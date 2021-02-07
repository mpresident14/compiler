#include "src/intermediate/intermediate.hpp"

#include <memory>
#include <stdexcept>

using namespace std;

namespace im {


/***********
 * Program *
 ***********/
SrcFile::SrcFile(vector<DeclPtr>&& decls) : decls_(move(decls)) {}

assem::SrcFile SrcFile::toAssemProg() const {
  vector<assem::DeclPtr> assemDecls;
  for (const DeclPtr& decl : decls_) {
    assemDecls.push_back(decl->toAssemDecl());
  }
  return assem::SrcFile(move(assemDecls));
}


/********
 * Func *
 ********/

Func::Func(const string& name, vector<StmtPtr>&& stmts) : name_(name), stmts_(move(stmts)) {}


assem::DeclPtr Func::toAssemDecl() {
  vector<assem::InstrPtr> assemInstrs;
  for (const StmtPtr& stmt : stmts_) {
    stmt->toAssemInstrs(assemInstrs);
  }
  return make_unique<assem::Function>(name_, move(assemInstrs));
}


/********
 * VTable *
 ********/

VTable::VTable(string_view name, vector<std::string>&& fnNames)
    : label_(name), fnNames_(move(fnNames)) {}

assem::DeclPtr VTable::toAssemDecl() { return make_unique<assem::VTable>(label_, move(fnNames_)); }


}  // namespace im
