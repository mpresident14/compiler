#include "src/language/language.hpp"
#include "src/main/parser.hpp"

#include <fstream>

using namespace std;


namespace language {


/***********
 * Program *
 ***********/

Program::Program(vector<string>&& imports, vector<DeclPtr>&& decls)
    : imports_(move(imports)), decls_(move(decls)), ctx_(nullptr) {}


void Program::initContext(
    string_view filename,
    std::unordered_map<std::string, Program>& initializedProgs) {
  // Create a new context
  ctx_ = make_shared<Ctx>(filename);

  // Go through the imports and build the context tree so we have access
  // to all imported declarations.
  for (const string& import : imports_) {
    auto progsIter = initializedProgs.find(import);
    Program* prog;
    if (progsIter == initializedProgs.end()) {
      ifstream importFile(import);
      ctx_->getLogger().checkFile(import, importFile);
      // Mark as initialized before recursing to allow circular dependencies
      prog = &initializedProgs.emplace(import, parser::parse(importFile))
                  .first->second;
    } else {
      prog = &progsIter->second;
    }

    prog->initContext(import, initializedProgs);
    ctx_->getCtxTree().addCtx(import, prog->ctx_);
  }

  // Add this program's declarations to its own context
  for (const DeclPtr& decl : decls_) {
    decl->addToContext(*ctx_);
  }
}


assem::Program Program::toAssemProg() const { return toImProg().toAssemProg(); }

im::Program Program::toImProg() const {
  vector<im::DeclPtr> imDecls;
  for (const DeclPtr& decl : decls_) {
    decl->toImDecls(imDecls, *ctx_);
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
    : Decl(line),
      returnType_(move(returnType)),
      name_(name),
      body_(move(body)) {
  paramTypes_.reserve(params.size());
  paramNames_.reserve(params.size());
  for (const auto& [type, name] : params) {
    paramTypes_.push_back(move(type));
    paramNames_.push_back(move(name));
  }
}


void Func::addToContext(Ctx& ctx) const {
  ctx.insertFn(name_, paramTypes_, returnType_, line_);
}


void Func::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  checkForReturn(ctx);
  ctx.setCurrentFn(name_);
  vector<im::StmtPtr> imStmts;

  // Insert all the parameters as variables
  // Move all parameters from argument registers into temporaries
  //   (b/c if we call a function w/i this function, they will be overwritten)
  size_t numParams = paramTypes_.size();
  for (size_t i = 0; i < numParams; ++i) {
    int temp = ctx.insertVar(paramNames_[i], paramTypes_[i], line_);
    imStmts.emplace_back(new im::Assign(
        make_unique<im::Temp>(temp), make_unique<im::Temp>(ARG_REGS[i])));
  }

  // Typecheck and compile the function
  body_->toImStmts(imStmts, ctx);
  // Remove all parameters
  ctx.removeParams(paramNames_, line_);

  if (optional<string> newFnName = mangleFnName(name_, ctx.getFilename())) {
    imDecls.emplace_back(new im::Func(move(*newFnName), move(imStmts)));
  } else {
    imDecls.emplace_back(new im::Func(name_, move(imStmts)));
  }
}

void Func::checkForReturn(Ctx& ctx) {
  vector<StmtPtr>& stmts = body_->stmts_;
  if (stmts.empty() || !(dynamic_cast<Return*>(stmts.back().get()))) {
    if (returnType_ == voidType) {
      // Add implicit return for functions with void return type if needed
      body_->stmts_.emplace_back(new Return({}, 0));
    } else {
      ctx.getLogger().logError(
          line_,
          string("Non-void function ")
              .append(name_)
              .append(" should return a value"));
    }
  }
}


}  // namespace language
