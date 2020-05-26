#include "src/language/language.hpp"

using namespace std;

namespace {
  Context& ctx = Context::getContext();
}

namespace language {

/***********
 * Program *
 ***********/

Program::Program(vector<DeclPtr>&& decls)
    : decls_(move(decls)) {}


im::Program Program::toImProg() const {
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
  return im::Program(move(imDecls));
}


/********
 * Func *
 ********/

Func::Func(
    Type returnType,
    const string& name,
    vector<pair<Type, string>>&& params,
    unique_ptr<Block>&& body,
    size_t line)
    : Decl(line), returnType_(returnType), name_(name), body_(move(body)) {
  paramTypes_.reserve(params.size());
  paramNames_.reserve(params.size());
  for (const auto& param : params) {
    paramTypes_.push_back(param.first);
    paramNames_.push_back(param.second);
  }
}


void Func::toImDecls(vector<im::DeclPtr>& imDecls) const {
  ctx.setCurrentFn(name_);
  ctx.setReturnTy(returnType_);
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
  imDecls.emplace_back(new im::Func(name_, move(imStmts)));
}

void Func::addToContext(Context& ctx) const {
  ctx.insertFn(name_, vector<Type>(paramTypes_), returnType_);
}


}  // namespace language
