#include "src/language/language.hpp"

using namespace std;


namespace language {


/***********
 * Program *
 ***********/

Program::Program(vector<DeclPtr>&& decls) : decls_(move(decls)) {}


im::Program Program::toImProg() const {
  // First go through all the declarations and add them to the context so that
  // we don't have to support forward declarations.
  for (const DeclPtr& decl : decls_) {
    decl->addToContext();
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
    TypePtr&& returnType,
    string_view name,
    vector<pair<TypePtr, string>>&& params,
    unique_ptr<Block>&& body,
    size_t line)
    : Decl(line), returnType_(move(returnType)), name_(name), body_(move(body)) {
  paramTypes_.reserve(params.size());
  paramNames_.reserve(params.size());
  for (const auto& [type, name] : params) {
    paramTypes_.push_back(move(type));
    paramNames_.push_back(move(name));
  }
}


void Func::toImDecls(vector<im::DeclPtr>& imDecls) {
  checkForReturn();
  ctx::setCurrentFn(name_);
  vector<im::StmtPtr> imStmts;

  // Insert all the parameters as variables
  // Move all parameters from argument registers into temporaries
  //   (b/c if we call a function w/i this function, they will be overwritten)
  size_t numParams = paramTypes_.size();
  for (size_t i = 0; i < numParams; ++i) {
    int temp = ctx::insertVar(paramNames_[i], paramTypes_[i], line_);
    imStmts.emplace_back(new im::Assign(
      make_unique<im::Temp>(temp),
      make_unique<im::Temp>(ARG_REGS[i])));
  }

  // Typecheck and compile the function
  body_->toImStmts(imStmts);
  // Remove all parameters
  ctx::removeParams(paramNames_, line_);
  imDecls.emplace_back(new im::Func(name_, move(imStmts)));
}

void Func::addToContext() const {
  ctx::insertFn(name_, paramTypes_, returnType_, line_);
}


void Func::checkForReturn() {
  vector<StmtPtr>& stmts = body_->stmts_;
  if (stmts.empty() || !(dynamic_cast<Return*>(stmts.back().get()))) {
    if (returnType_ == voidType) {
      // Add implicit return for functions with void return type if needed
      body_->stmts_.emplace_back(new Return({}, 0));
    } else {
      ctx::getLogger().logError(line_, string("Non-void function ").append(name_).append(" should return a value"));
    }
  }
}


}  // namespace language
