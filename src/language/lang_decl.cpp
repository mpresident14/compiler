#include "src/language/language.hpp"
#include "src/main/parser.hpp"

#include <fstream>

#include <boost/algorithm/string/split.hpp>

using namespace std;
namespace fs = std::filesystem;

namespace language {


/***********
 * Program *
 ***********/

Program::Program(vector<Import>&& imports, vector<DeclPtr>&& decls)
    : imports_(move(imports)), ctx_(nullptr) {
  for (DeclPtr& decl : decls) {
    if (decl->getCategory() == Decl::Category::CLASS) {
      classes_.emplace_back(static_cast<ClassDecl*>(decl.release()));
    } else {
      funcs_.emplace_back(static_cast<Func*>(decl.release()));
    }
  }
}


namespace {
  fs::path toCanonical(const fs::path& parent, fs::path imported, error_code& ec) {
    if (imported.is_relative()) {
      return fs::canonical(fs::path(parent).replace_filename(imported), ec);
    }
    return fs::canonical(imported, ec);
  }
}  // namespace


void Program::initContext(
    const fs::path& filePath,
    unordered_map<string, unique_ptr<Program>>& initializedProgs,
    const std::vector<Program>& builtIns,
    const shared_ptr<unordered_map<int, Ctx::ClassInfo*>>& classIds) {
  // Create a new context
  ctx_ = make_unique<Ctx>(filePath.c_str(), classIds);

  // Add builtin declarations to the context
  for (const Program& prog : builtIns) {
    ctx_->includeDecls(*prog.ctx_);
  }

  // Go through the imports and build the context tree so we have access
  // to all imported declarations.
  error_code ec;
  for (const Import& imported : imports_) {
    fs::path importPath = toCanonical(filePath, imported.filename, ec);
    if (ec) {
      ostream& err = ctx_->getLogger().logError(imported.line, ec.message());
      err << " \"" << imported.filename << "\"";
      continue;
    }

    auto progsIter = initializedProgs.find(importPath);
    Program* prog = nullptr;
    if (progsIter == initializedProgs.end()) {
      // We haven't initialized this program yet
      // Mark as initialized before recursing to allow circular dependencies
      try {
        // We use a nullptr as a placeholder to mark that we already tried to
        // parse the program. This way, we don't try parsing it again if it is
        // imported somewhere else
        auto iter = initializedProgs.emplace(importPath, nullptr).first;
        iter->second = make_unique<Program>(parser::parse(importPath));
        prog = iter->second.get();
        prog->initContext(importPath, initializedProgs, builtIns, classIds);
      } catch (const parser::ParseException& e) {
        cerr << e.what() << "\n(Imported at " << filePath << ", line " << imported.line << ")\n"
             << endl;
      }
    } else {
      prog = progsIter->second.get();
    }

    // If the program parsed, put the import's context in our context tree
    if (prog && !ctx_->getCtxTree().addCtx(imported.filename, prog->ctx_.get())) {
      ctx_->getLogger().logNote(imported.line, "Duplicate import '" + imported.filename + "'");
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
}  // namespace language


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
    Ctx::FnInfo::Inheritance inheritance,
    TypePtr&& returnType,
    string_view name,
    vector<pair<TypePtr, string>>&& params,
    unique_ptr<Block>&& body,
    size_t line)
    : Decl(line),
      inheritance_(inheritance),
      returnType_(move(returnType)),
      name_(name),
      body_(move(body)),
      id_(nextId_++) {
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
  imDecls.push_back(make_unique<im::Func>(Ctx::mangleFn(name_, id_), move(imStmts)));
}


void Func::checkForReturn(Ctx& ctx) {
  const StmtPtr* last = body_->lastStmt();
  if (!last || !(dynamic_cast<Return*>(last->get()))) {
    if (*returnType_ == *voidType) {
      // Add implicit return for functions with void return type if needed
      body_->stmts_.push_back(make_unique<Return>(std::optional<ExprPtr>(), 0));
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
    imStmts.push_back(
        make_unique<im::Assign>(make_unique<im::Temp>(temp), make_unique<im::Temp>(ARG_REGS[i])));
  }

  // Move extra parameters from stack into temporaries
  for (size_t i = numRegParams; i < numParams; i++) {
    TypePtr& pType = paramTypes_[i];
    ctx.checkType(*pType, line_);
    int temp = ctx.insertVar(paramNames_[i], pType, line_);
    size_t offset = 16 + 8 * (i - numRegParams);
    imStmts.push_back(make_unique<im::Assign>(
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
    // returnType_ is a nullptr for now b/c we assign it in ClassDecl::addToCtx to prevent creating
    // a new shared_ptr for each ctor
    : Func(Ctx::FnInfo::Inheritance::NONE, nullptr, name, move(params), move(body), line) {}


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
          line_),
      line_)
      .toImStmts(imStmts, ctx);

  // Create pointer to vtable if necessary
  if (vTableName_) {
    imStmts.push_back(make_unique<im::Assign>(
        make_unique<im::MemDeref>(line_, Var(ClassDecl::THIS, 0).toImExpr(ctx).imExpr, nullptr, 8),
        make_unique<im::LabelAddr>(*vTableName_)));
  }

  // Implicit return of the object we created. We insert it into the body rather than inserting it
  // manually into imStmts so that nicer errors are given if a user redefines "this"
  body_->stmts_.push_back(
      make_unique<Return>(optional<ExprPtr>{ make_unique<Var>(ClassDecl::THIS, 0) }, 0));
  body_->toImStmts(imStmts, ctx);

  ctx.removeParams(paramNames_, line_);
  ctx.removeVars({ { ClassDecl::THIS, 0 } });

  imDecls.push_back(make_unique<im::Func>(Ctx::mangleFn(name_, id_), move(imStmts)));
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
      case ClassElem::Type::METHOD: {
        unique_ptr<Func>& method = get<unique_ptr<Func>>(elem.elem);
        if (method->isVirtual()) {
          vTableEntries_.push_back(Ctx::mangleFn(mangleMethod(name_, method->name_), method->id_));
          vMethods_.push_back(move(method));
        } else {
          nonVMethods_.push_back(move(method));
        }
        break;
      }
      default:
        throw runtime_error("ClassDecl::ClassDecl");
    }
  }
}


ClassDecl::ClassDecl(
    std::string_view name,
    std::vector<std::string>&& superQuals,
    std::string_view superName,
    std::vector<ClassElem>&& classElems,
    size_t line)
    : ClassDecl(name, move(classElems), line) {
  superQuals_ = move(superQuals);
  superName_ = superName;
}


void ClassDecl::addToCtx(Ctx& ctx) {
  // I would rather have this logic in the Ctx class, but I can't have Func and Field in Ctx because
  // of circular dependency, and splitting them into their fields would make the code too messy imo

  Ctx::ClassInfo& classInfo = ctx.insertClass(name_, id_, line_);

  // Arrange fields from greatest size to least so that we don't overlap 8-byte intervals
  sort(fields_.begin(), fields_.end(), [](const Field& f1, const Field& f2) {
    return f1.type->numBytes < f2.type->numBytes;
  });

  // Class starts with vtable pointer if there are any virtual methods
  size_t currentOffset = vMethods_.empty() ? 0 : 8;
  for (const auto& [type, name, line] : fields_) {
    if (classInfo.fields.try_emplace(name, Ctx::FieldInfo{ type, currentOffset }).second) {
      currentOffset += type->numBytes;
    } else {
      ostringstream& err = ctx.getLogger().logError(line);
      err << "Redefinition of field " << name << " in class " << name_;
    }
  }

  // TODO: Add fields (here and in ctor)

  // Add all (virtual and nonvirtual) methods that weren't overridden from the superclass
  if (superName_) {
    const Ctx::ClassInfo* superInfo = ctx.lookupClassRec(superQuals_, *superName_, line_);
    if (superInfo) {
      for (const auto& [name, info] : superInfo->methods) {
        // TODO: The declFiles will not be correct here
        if (info.inheritance == Ctx::FnInfo::Inheritance::NONE) {
          ctx.insertMethod(
              classInfo.methods,
              info.inheritance,
              name,
              info.paramTypes,
              info.returnType,
              info.id,
              info.line);
        }
        // TODO: Is linear search the best option here?
        else if (
            find_if(
                vMethods_.cbegin(),
                vMethods_.cend(),
                [&supMethName = name, &supMethInfo = info](const std::unique_ptr<Func>& vMeth) {
                  // cout << "supMethname: " << supMethName;
                  // Ctx::streamParamTypes(supMethInfo.paramTypes, cout);
                  // cout << endl;
                  // cout << "vMeth->name_: " << vMeth->name_;
                  // Ctx::streamParamTypes(vMeth->paramTypes_, cout);
                  // cout << endl;
                  return vMeth->inheritance_ == Ctx::FnInfo::Inheritance::OVERRIDE &&
                         vMeth->name_ == supMethName &&
                         equal(
                             vMeth->paramTypes_.cbegin(),
                             vMeth->paramTypes_.cend(),
                             supMethInfo.paramTypes.cbegin(),
                             supMethInfo.paramTypes.cend());
                }) == vMethods_.cend()) {
          cout << "INSERT: " << name << endl;
          ctx.insertMethod(
              classInfo.methods,
              info.inheritance,
              name,
              info.paramTypes,
              info.returnType,
              info.id,
              info.line);
          vTableEntries_.push_back(Ctx::mangleFn(name, info.id));
        }
      }
    }
  }

  // We are inside the same file as the class declaration, so no qualifiers
  shared_ptr<Class> classTy = make_shared<Class>(vector<string>(), name_);

  // Add methods (AFTER checking for base class methods so that the "this" parameter doesn't mess up
  // matching)
  for (const unique_ptr<Func>& method : nonVMethods_) {
    method->checkTypes(ctx);
    ctx.insertMethod(
        classInfo.methods,
        method->inheritance_,
        method->name_,
        method->paramTypes_,
        method->returnType_,
        method->id_,
        method->line_);
    method->paramNames_.push_back(THIS);
    method->paramTypes_.push_back(classTy);
  }

  size_t vMethodCnt = 0;
  for (const unique_ptr<Func>& method : vMethods_) {
    method->checkTypes(ctx);
    ctx.insertMethod(
        classInfo.methods,
        method->inheritance_,
        method->name_,
        method->paramTypes_,
        method->returnType_,
        method->id_,
        method->line_);
    method->paramNames_.push_back(THIS);
    method->paramTypes_.push_back(classTy);
    classInfo.vTableOffsets.emplace(method->id_, vMethodCnt++);
  }


  optional<string> vtable = vMethods_.empty() ? optional<string>() : vTableName();
  // Add constructors to this context
  for (Constructor& ctor : ctors_) {
    if (ctor.name_ != name_) {
      ostringstream& err = ctx.getLogger().logError(ctor.line_);
      err << "Cannot declare a constructor for class " << ctor.name_
          << " inside declaration of class " << name_;
      continue;
    }
    ctor.objSize_ = currentOffset;
    ctor.returnType_ = classTy;
    ctor.vTableName_ = vtable;
    ctor.addToCtx(ctx);
  }
}  // namespace language


void ClassDecl::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  for (Constructor& ctor : ctors_) {
    ctor.toImDecls(imDecls, ctx);
  }

  for (const unique_ptr<Func>& method : nonVMethods_) {
    method->name_ = mangleMethod(name_, method->name_);
    method->toImDecls(imDecls, ctx);
  }

  for (const unique_ptr<Func>& method : vMethods_) {
    method->name_ = mangleMethod(name_, method->name_);
    method->toImDecls(imDecls, ctx);
  }

  // Create the vtable if necessary
  if (!vMethods_.empty()) {
    imDecls.push_back(make_unique<im::VTable>(vTableName(), move(vTableEntries_)));
  }
}


string ClassDecl::vTableName() { return string(name_).append("_vtable_").append(to_string(id_)); }

}  // namespace language
