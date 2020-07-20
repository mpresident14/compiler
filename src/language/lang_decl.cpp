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

// TODO: Imports should be relative to the file in which they were declared, not to the working
// directory where the compiler exe was called
// TODO: When we use an absolute path as the main file, it will compile the main file twice if a
// relative import is used in another file.
// Ex: mpresident@mpresident-XPS-15-9550:~/cs/compiler/test/translator/success$
// ~/cs/compiler/bin/main
// ~/cs/compiler/test/translator/success/import_test.prez ~/cs/compiler/log/import_test.s
Program::Program(vector<Import>&& imports, vector<DeclPtr>&& decls)
    : imports_(move(imports)), ctx_(nullptr) {
  imports_.push_back({ importDir + "/string.prez", 0 });
  for (DeclPtr& decl : decls) {
    if (decl->getCategory() == Decl::Category::CLASS) {
      classes_.emplace_back(static_cast<ClassDecl*>(decl.release()));
    } else {
      funcs_.emplace_back(static_cast<Func*>(decl.release()));
    }
  }
}


void Program::initContext(
    string_view filename,
    unordered_map<string, unique_ptr<Program>>& initializedProgs,
    const shared_ptr<unordered_map<int, Ctx::ClassInfo*>>& classIds) {
  // Create a new context
  ctx_ = make_unique<Ctx>(filename, classIds);

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
        prog->initContext(importName, initializedProgs, classIds);
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

  // Add this program's declarations to its own context. We add class names before functions/methods
  // so that parameters and return types that are classes will already be in the context.
  for (const unique_ptr<ClassDecl>& cls : classes_) {
    cls->addToCtx(*ctx_);
  }
  for (const unique_ptr<Func>& fn : funcs_) {
    fn->addToCtx(*ctx_);
  }
}


assem::Program Program::toAssemProg() const { return toImProg().toAssemProg(); }

im::Program Program::toImProg() const {
  vector<im::DeclPtr> imDecls;
  for (const unique_ptr<ClassDecl>& cls : classes_) {
    cls->toImDecls(imDecls, *ctx_);
  }
  for (const unique_ptr<Func>& fn : funcs_) {
    fn->toImDecls(imDecls, *ctx_);
  }
  return im::Program(move(imDecls));
}


/********
 * Func *
 ********/

size_t Func::nextId_ = 0;

Func::Func(
    TypePtr&& returnType,
    string_view name,
    vector<pair<TypePtr, string>>&& params,
    unique_ptr<Block>&& body,
    size_t line)
    : Decl(line), returnType_(move(returnType)), name_(name), body_(move(body)), id_(nextId_++) {
  paramTypes_.reserve(params.size());
  paramNames_.reserve(params.size());
  for (const auto& [type, name] : params) {
    paramTypes_.push_back(move(type));
    paramNames_.push_back(move(name));
  }
}


void Func::addToCtx(Ctx& ctx) {
  checkTypes(ctx);
  ctx.insertFn(name_, paramTypes_, returnType_, id_, line_);
}

void Func::checkTypes(Ctx& ctx) const {
  ctx.checkType(*returnType_, line_);
  for (const TypePtr& type : paramTypes_) {
    ctx.checkType(*type, line_);
  }
}

void Func::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  ctx.checkType(*returnType_, line_);
  checkForReturn(ctx);
  ctx.setCurrentRetType(returnType_);
  vector<im::StmtPtr> imStmts = paramsToImStmts(ctx);
  body_->toImStmts(imStmts, ctx);
  ctx.removeParams(paramNames_, line_);
  imDecls.emplace_back(
      new im::Func(ctx.mangleFn(name_, id_), move(imStmts)));
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


vector<im::StmtPtr> Func::paramsToImStmts(Ctx& ctx) {
  vector<im::StmtPtr> imStmts;
  // Insert all the parameters as variables
  // Move parameters from argument registers into temporaries
  //   (b/c if we call a function w/i this function, they will be overwritten)
  size_t numParams = paramTypes_.size();
  size_t numRegParams = min(numParams, (size_t)6);
  for (size_t i = 0; i < numRegParams; ++i) {
    TypePtr& pType = paramTypes_[i];
    ctx.checkType(*pType, line_);
    int temp = ctx.insertVar(paramNames_[i], pType, line_);
    imStmts.emplace_back(
        new im::Assign(make_unique<im::Temp>(temp), make_unique<im::Temp>(ARG_REGS[i])));
  }

  // Move extra parameters from stack into temporaries
  for (size_t i = numRegParams; i < numParams; i++) {
    TypePtr& pType = paramTypes_[i];
    ctx.checkType(*pType, line_);
    int temp = ctx.insertVar(paramNames_[i], pType, line_);
    size_t offset = 16 + 8 * (i - numRegParams);
    imStmts.emplace_back(new im::Assign(
        make_unique<im::Temp>(temp),
        make_unique<im::MemDeref>(offset, make_unique<im::Temp>(RBP), nullptr, 8)));
  }

  return imStmts;
}


/***************
 * Constructor *
 ***************/

Constructor::Constructor(
    string_view name,
    vector<pair<TypePtr, string>>&& params,
    unique_ptr<Block>&& body,
    size_t line)
    : Func(nullptr, name, move(params), move(body), line) {}


void Constructor::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  ctx.setCurrentRetType(returnType_);
  vector<im::StmtPtr> imStmts = paramsToImStmts(ctx);

  // Initialize "this"
  vector<im::ExprPtr> mallocBytes;
  mallocBytes.push_back(make_unique<im::Const>(objSize_));
  VarDecl(
      TypePtr(returnType_),
      ClassDecl::THIS,
      make_unique<ImWrapper>(
          make_unique<im::CallExpr>(
              make_unique<im::LabelAddr>("__malloc"), move(mallocBytes), true),
          returnType_,
          false,
          0),
      0)
      .toImStmts(imStmts, ctx);
  body_->toImStmts(imStmts, ctx);

  // Implicit return of the object we created
  Return({ make_unique<Var>(ClassDecl::THIS, 0) }, 0).toImStmts(imStmts, ctx);

  ctx.removeParams(paramNames_, line_);
  ctx.removeVars({ { ClassDecl::THIS, 0 } });

  imDecls.emplace_back(
      new im::Func(ctx.mangleFn(name_, id_), move(imStmts)));
}


/*********
 * Class *
 *********/

const string ClassDecl::THIS = "this";
int ClassDecl::nextId_ = 0;

string ClassDecl::mangleMethod(string_view className, string_view fnName) {
  return string(className).append("_").append(fnName);
}

ClassDecl::ClassDecl(string_view name, vector<ClassElem>&& classElems, size_t line)
    : Decl(line), name_(name), id_(nextId_++) {
  for (ClassElem& elem : classElems) {
    switch (elem.type) {
      case ClassElem::Type::FIELD:
        fields_.push_back(move(get<Field>(elem.elem)));
        break;
      case ClassElem::Type::CTOR:
        ctors_.push_back(move(get<Constructor>(elem.elem)));
        break;
      case ClassElem::Type::METHOD:
        methods_.push_back(move(get<unique_ptr<Func>>(elem.elem)));
        break;
      default:
        throw runtime_error("ClassDecl::ClassDecl");
    }
  }
}


void ClassDecl::addToCtx(Ctx& ctx) {
  // I would rather have this logic in the Ctx class, but I can't have Func and Field in Ctx because
  // of circular dependency, and splitting them into their fields would make the code too messy imo

  Ctx::ClassInfo& classInfo = ctx.insertClass(name_, id_, line_);

  // Arrange fields from greatest size to least so that we don't overlap 8-byte intervals
  sort(fields_.begin(), fields_.end(), [](const Field& f1, const Field& f2) {
    return f1.type->numBytes < f2.type->numBytes;
  });

  // Class starts with vtable pointer
  size_t currentOffset = 8;
  size_t objSize = 8;
  for (const auto& [type, name, line] : fields_) {
    if (classInfo.fields.try_emplace(name, Ctx::FieldInfo{ type, currentOffset }).second) {
      objSize += type->numBytes;
      currentOffset += type->numBytes;
    } else {
      ostringstream& err = ctx.getLogger().logError(line);
      err << "Redefinition of field " << name << " in class " << name_;
    }
  }

  // We are inside the same file as the class declaration, so no qualifiers
  shared_ptr<Class> classTy = make_shared<Class>(vector<string>(), name_);

  // Add constructors to this context
  for (Constructor& ctor : ctors_) {
    if (ctor.name_ != name_) {
      ostringstream& err = ctx.getLogger().logError(ctor.line_);
      err << "Cannot declare a constructor for class " << ctor.name_
          << " inside declaration of class " << name_;
      continue;
    }
    ctor.objSize_ = objSize;
    ctor.returnType_ = classTy;
    ctor.addToCtx(ctx);
  }

  // Add methods
  for (unique_ptr<Func>& method : methods_) {
    method->checkTypes(ctx);
    ctx.insertFn(classInfo.methods, method->name_, method->paramTypes_, method->returnType_, method->id_, method->line_);
    // Add "this" parameter as the last argument AFTER inserting it into the context
    method->paramNames_.push_back(THIS);
    method->paramTypes_.push_back(classTy);
  }
}


void ClassDecl::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  for (Constructor& ctor : ctors_) {
    ctor.toImDecls(imDecls, ctx);
  }
  for (const unique_ptr<Func>& method : methods_) {
    method->name_ = mangleMethod(name_, method->name_), method->toImDecls(imDecls, ctx);
  }
}

}  // namespace language
