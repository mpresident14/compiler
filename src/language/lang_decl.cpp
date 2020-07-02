#include "src/language/language.hpp"
#include "src/main/parser.hpp"

#include <fstream>

#include <boost/algorithm/string/split.hpp>

using namespace std;


namespace language {


/***********
 * Program *
 ***********/

string Program::importDir = "";
vector<string> Program::importPathParts = {};

void Program::setImportPath(string_view importPath) {
  importDir = importPath;
  boost::split(importPathParts, importPath, [](char c) { return c == '/'; });
}


Program::Program(vector<Import>&& imports, vector<DeclPtr>&& decls)
    : imports_(move(imports)), decls_(move(decls)), ctx_(nullptr) {
  imports_.push_back({ importDir + "/to_string.prez", 0 });
}


void Program::initContext(
    string_view filename,
    unordered_map<string, unique_ptr<Program>>& initializedProgs,
    shared_ptr<unordered_map<string, string>> fileIds,
    shared_ptr<unordered_map<string, string>> typeIds) {
  // Create a new context
  ctx_ = make_unique<Ctx>(filename, fileIds, typeIds);
  ctx_->addFileId(filename);

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
        cerr << e.what() << "\n(Imported at " << filename << ", line " << imported.line << ")\n"
             << endl;
      }
    } else {
      prog = progsIter->second.get();
    }

    // If the program parsed, put the import's context in our context tree
    if (prog && !ctx_->getCtxTree().addCtx(importName, prog->ctx_.get())) {
      ctx_->getLogger().logNote(imported.line, "Duplicate import '" + importName + "'");
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
    : Decl(line), returnType_(move(returnType)), name_(name), body_(move(body)) {
  paramTypes_.reserve(params.size());
  paramNames_.reserve(params.size());
  for (const auto& [type, name] : params) {
    paramTypes_.push_back(move(type));
    paramNames_.push_back(move(name));
  }
}


void Func::addToContext(Ctx& ctx) { ctx.insertFn(name_, paramTypes_, returnType_, line_); }


void Func::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  checkForReturn(ctx);
  ctx.setCurrentRetType(returnType_);
  vector<im::StmtPtr> imStmts;

  // Insert all the parameters as variables
  // Move parameters from argument registers into temporaries
  //   (b/c if we call a function w/i this function, they will be overwritten)
  size_t numParams = paramTypes_.size();
  size_t numRegParams = min(numParams, (size_t)6);
  for (size_t i = 0; i < numRegParams; ++i) {
    int temp = ctx.insertVar(paramNames_[i], paramTypes_[i], line_);
    imStmts.emplace_back(
        new im::Assign(make_unique<im::Temp>(temp), make_unique<im::Temp>(ARG_REGS[i])));
  }

  // Move extra parameters from stack into temporaries
  for (size_t i = numRegParams; i < numParams; i++) {
    int temp = ctx.insertVar(paramNames_[i], paramTypes_[i], line_);
    size_t offset = 16 + 8 * (i - numRegParams);
    imStmts.emplace_back(new im::Assign(
        make_unique<im::Temp>(temp),
        make_unique<im::MemDeref>(offset, make_unique<im::Temp>(RBP), nullptr, 8)));
  }

  // Typecheck and compile the function
  body_->toImStmts(imStmts, ctx);
  // Remove all parameters
  ctx.removeParams(paramNames_, line_);

  imDecls.emplace_back(
      new im::Func(ctx.mangleFn(name_, ctx.getFilename(), paramTypes_), move(imStmts)));
}

void Func::checkForReturn(Ctx& ctx) {
  const StmtPtr* last = body_->lastStmt();
  if (!last || !(dynamic_cast<Return*>(last->get()))) {
    if (*returnType_ == *voidType) {
      // Add implicit return for functions with void return type if needed
      body_->stmts_.emplace_back(new Return({}, 0));
    } else {
      ostringstream& error = ctx.getLogger().logError(line_);
      error << "Some paths through non-void function '" << *returnType_ << ' ' << name_;
      Ctx::streamParamTypes(paramTypes_, error);
      error << "' do not return a value";
    }
  }
}


/*********
 * Class *
 *********/

const std::string ClassDecl::THIS = "this";

string ClassDecl::mangleMethod(std::string_view className, std::string_view fnName) {
  return string("_").append(className).append("_").append(fnName);
}

ClassDecl::ClassDecl(
    string_view name,
    vector<unique_ptr<Func>>&& methods,
    vector<Field>&& fields,
    size_t line)
    : Decl(line), name_(name), methods_(move(methods)), fields_(move(fields)) {}


void ClassDecl::addToContext(Ctx& ctx) {
  // I would rather have this logic in the Ctx class, but I can't have Func and Field in Ctx because
  // of circular dependency, and splitting them into their fields would make the code too messy imo
  if (ctx.lookupClass(name_)) {
    ostringstream& err = ctx.getLogger().logError(line_);
    err << "Redefinition of class " << name_;
    return;
  }

  //! TODO(BUG): This will screw up when we try to import a class with the same name from multiple files
  //! E.g. "void f(file1::Myclass obj);"" and "void f(file2::Myclass obj);"
  //! We should put it in ClassInfo instead and retrieve it
  ctx.addTypeId(name_);

  for (unique_ptr<Func>& method : methods_) {
    method->name_ = mangleMethod(name_, method->name_);
    // Add "this" parameter as the last argument
    method->paramNames_.push_back(THIS);
    method->paramTypes_.push_back(make_shared<Class>(name_));
  }

  // Add methods to context
  unordered_multimap<string, Ctx::FnInfo> methodMap;
  for (unique_ptr<Func>& method : methods_) {
    // TODO: Specifying class name in error would be nice
    ctx.insertFn(methodMap, method->name_, method->paramTypes_, method->returnType_, method->line_);
  }

  // Arrange fields from greatest size to least so that we don't overlap 8-byte intervals
  sort(fields_.begin(), fields_.end(), [](const Field& f1, const Field& f2) {
    return f1.type->numBytes < f2.type->numBytes;
  });
  unordered_map<string, size_t> fieldOffsets;
  // Class starts with vtable pointer
  size_t currentOffset = 8;
  for (const auto& [type, name, line] : fields_) {
    if (fieldOffsets.try_emplace(name, currentOffset).second) {
      ostringstream& err = ctx.getLogger().logError(line);
      err << "Redefinition of field " << name << " in class " << name_;
    }
  }

  ctx.insertClass(name_, move(methodMap), move(fieldOffsets));
}


void ClassDecl::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  for (const unique_ptr<Func>& method : methods_) {
    method->toImDecls(imDecls, ctx);
  }
}

}  // namespace language
