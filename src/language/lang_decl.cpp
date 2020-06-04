#include "src/language/language.hpp"
#include "src/main/parser.hpp"

#include <fstream>

using namespace std;


namespace {

size_t newFileId() {
  static size_t i = 0;
  return i++;
}

}  // namespace

namespace language {


/***********
 * Program *
 ***********/

Program::Program(vector<Import>&& imports, vector<DeclPtr>&& decls)
    : imports_(move(imports)), decls_(move(decls)), ctx_(nullptr) {}


void Program::initContext(
    string_view filename,
    unordered_map<string, unique_ptr<Program>>& initializedProgs,
    shared_ptr<unordered_map<string, string>> fileIds,
    shared_ptr<unordered_map<string, string>> typeIds) {
  // Create a new context
  ctx_ = make_unique<Ctx>(filename, fileIds, typeIds);
  ctx_->addFileId(newFileId(), filename);

  // Go through the imports and build the context tree so we have access
  // to all imported declarations.
  for (const Import& imported : imports_) {
    const string& importName = imported.filename;
    auto progsIter = initializedProgs.find(importName);
    Program* prog = nullptr;
    if (progsIter == initializedProgs.end()) {
      // We haven't initialized this program yet
      // Mark as initialized before recursing to allow circular dependencies
      try {
        // We use a nullptr as a placeholder to mark that we already tried to
        // parse the program. This way, we don't try parsing it again if it is
        // imported somewhere else
        auto iter = initializedProgs.emplace(importName, nullptr).first;
        iter->second = make_unique<Program>(parser::parse(importName));
        prog = iter->second.get();
        prog->initContext(importName, initializedProgs, fileIds, typeIds);
      } catch (const runtime_error& e) {
        // Catch "can't open file" errors
        ctx_->getLogger().logError(imported.line, e.what());
      } catch (const parser::ParseException& e) {
        cerr << e.what() << "\n(Imported at " << filename << ", line "
             << imported.line << ")\n"
             << endl;
      }
    } else {
      prog = progsIter->second.get();
    }

    // If the program parsed, put the import's context in our context tree
    // if (!prog || (prog->has_value() && !ctx_)) {
    //   cout << "WAAAAHHHH!!!!" << endl;
    // }
    if (prog && !ctx_->getCtxTree().addCtx(importName, prog->ctx_.get())) {
      ctx_->getLogger().logNote(
          imported.line, "Duplicate import '" + importName + "'");
    }
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


void Func::addToContext(Ctx& ctx) {
  ctx.insertFn(name_, paramTypes_, returnType_, line_);
}


void Func::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  checkForReturn(ctx);
  ctx.setCurrentRetType(returnType_);
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

  imDecls.emplace_back(new im::Func(
      ctx.mangleFn(name_, ctx.getFilename(), paramTypes_), move(imStmts)));
}

void Func::checkForReturn(Ctx&) {
  vector<StmtPtr>& stmts = body_->stmts_;
  if (stmts.empty() || !(dynamic_cast<Return*>(stmts.back().get()))) {
    if (returnType_ == voidType) {
      // Add implicit return for functions with void return type if needed
      body_->stmts_.emplace_back(new Return({}, 0));
    }
    // TODO: The code below fails to address return values in, for example,
    // and if-else block. This check needs to be done at the instruction level
    // } else {
    //   ctx.getLogger().logError(
    //       line_,
    //       string("Non-void function ")
    //           .append(name_)
    //           .append(" should return a value"));
    // }
  }
}


}  // namespace language
