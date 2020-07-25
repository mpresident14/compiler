#include "src/language/decl.hpp"

#include "src/language/stmt.hpp"
#include "src/language/utils.hpp"

using namespace std;

namespace language {

/********
 * Func *
 ********/

size_t Func::nextId_ = 0;

Func::Func(
    Func::Inheritance inheritance,
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
    string_view name,
    vector<pair<TypePtr, string>>&& params,
    unique_ptr<Block>&& body,
    size_t line)
    // returnType_ is a nullptr for now b/c we assign it in ClassDecl::addToCtx to prevent creating
    // a new shared_ptr for each ctor
    : Func(Func::Inheritance::NONE, nullptr, name, move(params), move(body), line) {}


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

// string ClassDecl::mangleMethod(string_view className, string_view fnName) {
//   return string(className).append("_").append(fnName);
// }


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
  // I would rather have this logic in the Ctx class, but I can't have Func and Field in Ctx because
  // of circular dependency, and splitting them into their fields would make the code too messy imo

  Ctx::ClassInfo& classInfo = ctx.insertClass(name_, id_, line_);

  // Class starts with vtable pointer if there are any virtual methods
  size_t currentOffset = vMethods_.empty() ? 0 : 8;
  size_t vMethodCnt = 0;
  if (superName_) {
    const Ctx::ClassInfo* superInfo = ctx.lookupClassRec(superQuals_, *superName_, line_);
    if (superInfo) {
      classInfo.superInfo = superInfo;
      vMethodCnt = superInfo->vTableOffsets.size();
      vTableEntries_.resize(vMethodCnt);

      // Add all fields from superclass
      for (const auto& [name, info] : superInfo->fields) {
        classInfo.fields.emplace(name, info);
        currentOffset = info.offset;
      }

      // Add all methods that from the superclass (including those we override)
      for (const auto& [name, info] : superInfo->methods) {
        if (!info.isVirtual) {
          // No error-checking required here. If there was an error, we will have already reported
          // it in the superclass
          classInfo.methods.emplace(name, info);
        } else {
          // We need to make sure that this class's vtable entries are in the same order as its
          // superclass, otherwise casting and invoking a method won't work. Therefore, we find any
          // methods that override superclass methods and insert them at the same vtable index

          // TODO: Is linear search the best option here? Instead, we could iterate thru this
          // class's vMethods, which would also allow us to utilize the hashmap and find override
          // methods that aren't overriding anything
          auto overrideIter = find_if(
              vMethods_.cbegin(),
              vMethods_.cend(),
              [& supMethName = name, &supMethInfo = info](const unique_ptr<Func>& vMeth) {
                return vMeth->inheritance_ == Func::Inheritance::OVERRIDE
                       && vMeth->name_ == supMethName
                       && equal(
                           vMeth->paramTypes_.cbegin(),
                           vMeth->paramTypes_.cend(),
                           supMethInfo.paramTypes.cbegin(),
                           supMethInfo.paramTypes.cend());
              });
          // No redefinition/type checking required here. If there was an error, we will have
          // already reported it in the superclass
          size_t funcId;
          if (overrideIter == vMethods_.cend()) {
            funcId = info.id;
            classInfo.methods.emplace(name, info);
          } else {
            funcId = (*overrideIter)->id_;
            classInfo.methods.emplace(
                name,
                Ctx::FnInfo{info.paramTypes,
                info.returnType,
                true,
                funcId,
                ctx.getFilename(),
                (*overrideIter)->line_});
          }

          size_t vTableIndex = superInfo->vTableOffsets.at(info.id);
          vTableEntries_[vTableIndex] = Ctx::mangleFn(name, funcId);
          classInfo.vTableOffsets.emplace(funcId, vTableIndex);
        }
      }
    }
  }

  for (const auto& [type, name, line] : fields_) {
    if (classInfo.fields.try_emplace(name, Ctx::FieldInfo{ type, currentOffset }).second) {
      currentOffset += type->numBytes;
    } else {
      ostringstream& err = ctx.getLogger().logError(line);
      err << "Redefinition of field '" << name << "' in class " << name_;
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
        name_,
        method->name_,
        method->paramTypes_,
        method->returnType_,
        false,
        method->id_,
        method->line_);
    method->paramNames_.push_back(lang_utils::THIS);
    method->paramTypes_.push_back(classTy);
  }

  for (const unique_ptr<Func>& method : vMethods_) {
    // We already added the overridden methods above
    // TODO: Check for methods declared override that don't override anything
    if (method->inheritance_ != Func::Inheritance::OVERRIDE) {
      method->checkTypes(ctx);
      ctx.insertMethod(
          classInfo.methods,
          name_,
          method->name_,
          method->paramTypes_,
          method->returnType_,
          true,
          method->id_,
          method->line_);
      method->paramNames_.push_back(lang_utils::THIS);
      method->paramTypes_.push_back(classTy);
      vTableEntries_.push_back(Ctx::mangleFn(method->name_, method->id_));
      classInfo.vTableOffsets.emplace(method->id_, vMethodCnt++);
    }
  }

  // cout << vTableEntries_ << endl;
  // cout << classInfo.vTableOffsets << endl;

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
  ctx.enterClass();
  for (Constructor& ctor : ctors_) {
    ctor.toImDecls(imDecls, ctx);
  }

  for (const unique_ptr<Func>& method : nonVMethods_) {
    // method->name_ = mangleMethod(name_, method->name_);
    method->toImDecls(imDecls, ctx);
  }

  for (const unique_ptr<Func>& method : vMethods_) {
    // method->name_ = mangleMethod(name_, method->name_);
    method->toImDecls(imDecls, ctx);
  }

  // Create the vtable if necessary
  if (!vMethods_.empty()) {
    imDecls.push_back(make_unique<im::VTable>(vTableName(), move(vTableEntries_)));
  }

  ctx.exitClass();
}


string ClassDecl::vTableName() { return string(name_).append("_vtable_").append(to_string(id_)); }

}  // namespace language
