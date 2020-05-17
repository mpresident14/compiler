#include "src/language/language.hpp"

using namespace std;

namespace language {


Context& ctx = Context::getContext();


im::Program Program::toImStmts() const {
  // First go through all the declarations and add them to the context so that
  // we don't have to support forward declarations.
  for (const DeclPtr& decl : decls_) {
    decl->addToContext(ctx);
  }
  // Then typecheck and compile
  vector<im::DeclPtr> imDecls;
  for (const DeclPtr& decl : decls_) {
    decl->toImDecls(imDecls);
  }
  return im::Program(name_, move(imDecls));
}


void Func::toImDecls(std::vector<im::DeclPtr>& imDecls) const {
  // Insert all the parameters as variables
  size_t numParams = paramTypes_.size();
  for (size_t i = 0; i < numParams; ++i) {
    ctx.insertParam(paramNames_[i], paramTypes_[i], ARG_REGS[i]);
  }
  // Typecheck and compile the function
  vector<im::StmtPtr> imStmts;
  body_->toImStmts(imStmts);
  // Remove all parameters
  for (const string& param : paramNames_) {
    ctx.removeVar(param);
  }
  imDecls.emplace_back(new im::Func(move(imStmts)));
}

void Func::addToContext(Context& ctx) const {
  ctx.insertFn(name_, vector<Type>(paramTypes_), returnType_);
}


}
