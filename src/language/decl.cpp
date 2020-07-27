#include "src/language/decl.hpp"

#include "src/language/stmt.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/utils.hpp"

#include <prez/print_stuff.hpp>

using namespace std;

namespace language {

/********
 * Func *
 ********/

size_t Func::nextId_ = 0;

Func::Func(
    int modifiers,
    TypePtr&& returnType,
    string_view name,
    vector<pair<TypePtr, string>>&& params,
    unique_ptr<Block>&& body,
    size_t line)
    : Decl(line),
      modifiers_(modifiers),
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
  ctx.insertFn(*this);
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

  if (ctx.insideClass()) {
    // Prevent undefined variable "this" warning
    // TODO: Log note that this method can be static methods (after we implement static methods)
    ctx.removeThis();
  }
  ctx.removeParams(paramNames_, line_);

  imDecls.push_back(make_unique<im::Func>(Ctx::mangleFn(name_, id_), move(imStmts)));
}


void Func::checkForReturn(Ctx& ctx) {
  const StmtPtr* last = body_->lastStmt();
  if (!last || !(dynamic_cast<Return*>(last->get()))) {
    if (*returnType_ == *Type::VOID_TYPE) {
      // Add implicit return for functions with void return type if needed
      body_->stmts_.push_back(make_unique<Return>(optional<ExprPtr>(), 0));
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
    string_view name, vector<pair<TypePtr, string>>&& params, unique_ptr<Block>&& body, size_t line)
    // returnType_ is a nullptr for now b/c we assign it in ClassDecl::addToCtx to prevent creating
    // a new shared_ptr for each ctor
    : Func(Func::Modifier::NONE, nullptr, name, move(params), move(body), line) {}


void Constructor::toImDecls(vector<im::DeclPtr>& imDecls, Ctx& ctx) {
  ctx.setCurrentRetType(returnType_);
  vector<im::StmtPtr> imStmts = paramsToImStmts(ctx);

  // Initialize "this"
  vector<im::ExprPtr> mallocBytes;
  mallocBytes.push_back(make_unique<im::Const>(objSize_));
  VarDecl(
      TypePtr(returnType_),
      lang_utils::THIS,
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
        make_unique<im::MemDeref>(0, Var(lang_utils::THIS, 0).toImExpr(ctx).imExpr, nullptr, 8),
        make_unique<im::LabelAddr>(*vTableName_)));
  }

  // Implicit return of the object we created. We insert it into the body rather than inserting it
  // manually into imStmts so that nicer errors are given if a user redefines "this"
  body_->stmts_.push_back(
      make_unique<Return>(optional<ExprPtr>{ make_unique<Var>(lang_utils::THIS, 0) }, 0));
  body_->toImStmts(imStmts, ctx);

  ctx.removeThis();
  ctx.removeParams(paramNames_, line_);

  imDecls.push_back(make_unique<im::Func>(Ctx::mangleFn(name_, id_), move(imStmts)));
}


/*********
 * Class *
 *********/

int ClassDecl::nextId_ = 0;

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
        switch (method->modifiers_) {
          case Func::Modifier::NONE:
            nonVMethods_.push_back(move(method));
            break;
          case Func::Modifier::VIRTUAL:
            vMethods_.push_back(move(method));
            break;
          case Func::Modifier::OVERRIDE:
            overrideMethods_.push_back(move(method));
            break;
          case Func::Modifier::STATIC:
            staticMethods_.push_back(move(method));
            break;
          default:
            throw runtime_error("ClassDecl::ClassDecl (Modifier");
        }
        break;
      }
      default:
        throw runtime_error("ClassDecl::ClassDecl (ClassElem");
    }
  }
}


ClassDecl::ClassDecl(
    string_view name,
    vector<string>&& superQuals,
    string_view superName,
    vector<ClassElem>&& classElems,
    size_t line)
    : ClassDecl(name, move(classElems), line) {
  superQuals_ = move(superQuals);
  superName_ = superName;
}


void ClassDecl::addToCtx(Ctx& ctx) {
  // We are inside the same file as the class declaration, so no qualifiers
  shared_ptr<Class> classTy = make_shared<Class>(vector<string>(), name_);
  Ctx::ClassInfo& classInfo = ctx.insertClass(name_, id_, line_);

  // Class starts with vtable pointer if there are any virtual methods
  size_t currentOffset = hasVirtualFns() ? 8 : 0;
  size_t vMethodCnt = 0;
  const Ctx::ClassInfo* superInfo = nullptr;
  if (superName_) {
    if ((superInfo = ctx.lookupClassRec(superQuals_, *superName_, line_))) {
      classInfo.superInfo = superInfo;
      vMethodCnt = superInfo->vTableOffsets.size();
      vTableEntries_.resize(vMethodCnt);

      // Add all fields from superclass
      for (const auto& [name, info] : superInfo->fields) {
        classInfo.fields.emplace(name, info);
        currentOffset += info.type->numBytes;
      }
    }
  }

  // Add fields declared in this class
  for (const auto& [type, name, line] : fields_) {
    if (classInfo.fields.emplace(name, Ctx::FieldInfo{ type, currentOffset }).second) {
      currentOffset += type->numBytes;
    } else {
      ostringstream& err = ctx.getLogger().logError(line);
      err << "Redefinition of field '" << name << "' in class " << name_;
    }
  }

  // Add all methods declared in superclass first. This way, we will only log redefinition errors
  // for new functions declared in this class

  // Add virtual methods overriding a base class method
  unordered_set<const Ctx::FnInfo*> supsOverridden;
  for (const unique_ptr<Func>& method : overrideMethods_) {
    method->checkTypes(ctx);

    // No superclass
    if (!superInfo) {
      ostream& err = ctx.getLogger().logError(line_);
      err << "Method " << name_ << "::" << method->name_;
      Ctx::streamParamTypes(method->paramTypes_, err);
      err << " is declared override, but class " << name_ << " has no superclass";
      continue;
    }

    auto supMethRange = superInfo->methods.equal_range(method->name_);
    auto supMethIter = find_if(
        supMethRange.first,
        supMethRange.second,
        [&method](const pair<const string, const Ctx::FnInfo>& p) {
          const Ctx::FnInfo& supMethInfo = p.second;
          return Func::isVirtual(supMethInfo.modifiers)
                 && equal(
                     supMethInfo.paramTypes.cbegin(),
                     supMethInfo.paramTypes.cend(),
                     method->paramTypes_.cbegin(),
                     method->paramTypes_.cend());
        });

    // No identical virtual method in superclass
    if (supMethIter == supMethRange.second) {
      ostream& err = ctx.getLogger().logError(line_);
      err << "Method " << name_ << "::" << method->name_;
      Ctx::streamParamTypes(method->paramTypes_, err);
      err << " is declared override, but does not override a virtual method from the superclass";
      continue;
    }

    const Ctx::FnInfo& supMethInfo = supMethIter->second;
    supsOverridden.insert(&supMethInfo);
    // We need to make sure that this class's vtable entries are in the same order as its
    // superclass, otherwise casting and invoking a method won't work. Therefore, we find any
    // methods that override superclass methods and insert them at the same vtable index
    size_t vTableIndex = superInfo->vTableOffsets.at(supMethInfo.id);
    vTableEntries_[vTableIndex] = Ctx::mangleFn(method->name_, method->id_);
    classInfo.vTableOffsets.emplace(method->id_, vTableIndex);

    ctx.insertMethod(classInfo.methods, name_, *method);
    method->paramNames_.push_back(lang_utils::THIS);
    method->paramTypes_.push_back(classTy);
  }

  // Add nonstatic superclass methods that were not overridden. No error-checking required here. If
  // there was an error, we will have already reported it in the superclass
  if (superInfo) {
    for (const auto& [name, supMethInfo] : superInfo->methods) {
      if (!(Func::isStatic(supMethInfo.modifiers)) && !supsOverridden.contains(&supMethInfo)) {
        classInfo.methods.emplace(name, supMethInfo);
        if (Func::isVirtual(supMethInfo.modifiers)) {
          size_t vTableIndex = superInfo->vTableOffsets.at(supMethInfo.id);
          vTableEntries_[vTableIndex] = Ctx::mangleFn(name, supMethInfo.id);
          classInfo.vTableOffsets.emplace(supMethInfo.id, vTableIndex);
        }
      }
    }
  }

  // Add nonvirtual methods declared in this class
  for (const unique_ptr<Func>& method : vMethods_) {
    method->checkTypes(ctx);

    vTableEntries_.push_back(Ctx::mangleFn(method->name_, method->id_));
    classInfo.vTableOffsets.emplace(method->id_, vMethodCnt++);

    ctx.insertMethod(classInfo.methods, name_, *method);
    method->paramNames_.push_back(lang_utils::THIS);
    method->paramTypes_.push_back(classTy);
  }

  // Add nonvirtual methods declared in this class
  for (const unique_ptr<Func>& method : nonVMethods_) {
    method->checkTypes(ctx);
    ctx.insertMethod(classInfo.methods, name_, *method);
    method->paramNames_.push_back(lang_utils::THIS);
    method->paramTypes_.push_back(classTy);
  }

  // Add static methods declared in this class
  for (const unique_ptr<Func>& method : staticMethods_) {
    method->checkTypes(ctx);
    ctx.insertMethod(classInfo.methods, name_, *method);
  }

  optional<string> vtable = hasVirtualFns() ? vTableName() : optional<string>();
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
  ctx.enterClass(id_);
  for (Constructor& ctor : ctors_) {
    ctor.toImDecls(imDecls, ctx);
  }
  for (const unique_ptr<Func>& method : nonVMethods_) {
    method->toImDecls(imDecls, ctx);
  }
  for (const unique_ptr<Func>& method : vMethods_) {
    method->toImDecls(imDecls, ctx);
  }
  for (const unique_ptr<Func>& method : overrideMethods_) {
    method->toImDecls(imDecls, ctx);
  }
  for (const unique_ptr<Func>& method : staticMethods_) {
    method->toImDecls(imDecls, ctx);
  }

  // Create the vtable if necessary
  if (hasVirtualFns()) {
    imDecls.push_back(make_unique<im::VTable>(vTableName(), move(vTableEntries_)));
  }

  ctx.exitClass();
}

bool ClassDecl::hasVirtualFns() const noexcept {
  return !(vMethods_.empty() && overrideMethods_.empty());
}
string ClassDecl::vTableName() { return string(name_).append("_vtable_").append(to_string(id_)); }

}  // namespace language
