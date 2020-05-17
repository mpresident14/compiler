#include "src/language/language.hpp"

using namespace std;

namespace language {


Context& ctx = Context::getContext();

vector<im::StmtPtr> Program::toImStmts() const {
  // First go through all the declarations and add them to the context so that
  // we don't have to support forward declarations.
  for (const DeclPtr& decl : decls) {
    decl->addToContext(ctx);
  }
  // Then typecheck and compile
  vector<im::StmtPtr> imStmts;
  for (const DeclPtr& decl : decls) {
    decl->toImStmts(imStmts);
  }
  return imStmts;
}

void Func::toImStmts(vector<im::StmtPtr>& imStmts) const {
  // Insert all the parameters as variables
  size_t numParams = paramTypes_.size();
  for (size_t i = 0; i < numParams; ++i) {
    ctx.insertParam(paramNames_[i], paramTypes_[i], ARG_REGS[i]);
  }
  // Typecheck and compile the function
  body_->toImStmts(imStmts);
  // Remove all parameters
  for (const string& param : paramNames_) {
    ctx.removeVar(param);
  }
}

void Func::addToContext(Context& ctx) const {
  ctx.insertFn(name_, vector<Type>(paramTypes_), returnType_);
}


}
