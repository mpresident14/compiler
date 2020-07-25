#include "src/language/typecheck/context.hpp"

#include "src/assembly/temp.hpp"
#include "src/language/utils.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace std;

void Ctx::streamParamTypes(const vector<TypePtr>& paramTypes, ostream& err) {
  err << '(';
  if (!paramTypes.empty()) {
    for (auto iter = paramTypes.cbegin(); iter != prev(paramTypes.cend()); ++iter) {
      err << **iter << ", ";
    }
    err << *paramTypes.back();
  }
  err << ')';
}


Ctx::Ctx(string_view filename, const shared_ptr<unordered_map<int, ClassInfo*>>& classIds)
    : filename_(filename), logger(filename), classIds_(classIds) {}

const std::string& Ctx::getFilename() const noexcept { return filename_; }
Logger& Ctx::getLogger() noexcept { return logger; }
Ctx::CtxTree& Ctx::getCtxTree() noexcept { return ctxTree_; };
const Type& Ctx::getCurrentRetType() const noexcept { return *currentRetType_; }
void Ctx::setCurrentRetType(const TypePtr& type) noexcept { currentRetType_ = type; }

void Ctx::includeDecls(Ctx& ctx) {
  for (const auto& p : ctx.fnMap_) {
    fnMap_.insert(p);
  }

  for (const auto& p : ctx.classMap_) {
    classMap_.insert(p);
  }
}


int Ctx::insertVar(const string& name, const TypePtr& type, size_t line) {
  int temp = newTemp();
  auto insertResult = varMap_.try_emplace(name, type, temp, line);
  if (!insertResult.second) {
    auto& errStream = logger.logError(line);
    errStream << "Redefinition of variable '" << name << "'. Originally declared on line "
              << insertResult.first->second.line;
  }
  return temp;
}


const Ctx::VarInfo* Ctx::lookupVar(const string& name, size_t line) {
  auto iter = varMap_.find(name);
  if (iter == varMap_.end()) {
    logger.logError(line, string("Undefined variable '").append(name).append("'"));
    return nullptr;
  }
  VarInfo& varInfo = iter->second;
  varInfo.used = true;
  return &varInfo;
}


void Ctx::removeVars(const vector<pair<string, size_t>>& vars) {
  for (const auto& [var, line] : vars) {
    removeVar(var, line);
  }
}

void Ctx::removeParams(const vector<string>& params, size_t line) {
  for (const string& param : params) {
    removeVar(param, line);
  }
}

void Ctx::removeVar(const string& var, size_t line) {
  // If we had a variable redefinition error, we may have already removed this
  // variable
  auto iter = varMap_.find(var);
  if (iter != varMap_.end() && !iter->second.used) {
    logger.logWarning(line, string("Unused variable '").append(var).append("'"));
  }
  varMap_.erase(var);
}

void Ctx::removeThis() { varMap_.erase(lang_utils::THIS); }

Ctx::ClassInfo& Ctx::insertClass(const string& className, int id, size_t line) {
  // TODO: Use try_emplace here (ClassInfo will need a ctor)
  auto p = classMap_.try_emplace(className, filename_, id);
  if (p.second) {
    classIds_->emplace(id, &p.first->second);
  } else {
    ostringstream& err = logger.logError(line);
    err << "Redefinition of class " << className;
  }

  return p.first->second;
}


void Ctx::enterClass() { insideClass_ = true; }
void Ctx::exitClass() { insideClass_ = false; }

bool Ctx::isBaseOf(const Class& classTy, const Type& base) const {
  if (base.typeName != Type::Category::CLASS) {
    return false;
  }

  int baseId = static_cast<const Class&>(base).id;
  const ClassInfo* classInfo = lookupClass(classTy.id);

  // cout << "baseId: " << baseId << endl;

  while (classInfo) {
  // cout << "classInfo->id: " << classInfo->id << endl;
    if (classInfo->id == baseId) {
      return true;
    }
    classInfo = classInfo->superInfo;
  }

  return false;
}

Ctx::ClsLookupRes Ctx::lookupClass(const string& name) const {
  auto iter = classMap_.find(name);
  if (iter == classMap_.end()) {
    return { LookupStatus::UNDEFINED, nullptr, filename_ };
  }
  return { LookupStatus::FOUND, &iter->second, filename_ };
}

const Ctx::ClassInfo* Ctx::lookupClass(int id) const {
  auto iter = classIds_->find(id);
  if (iter == classIds_->end()) {
    return nullptr;
  }
  return iter->second;
}

const Ctx::ClassInfo* Ctx::lookupClassRec(
    const vector<string>& qualifiers, const string& name, size_t line) const {
  LookupRes lookupRes =
      qualifiers.empty() ? lookupClass(name) : ctxTree_.lookupClass(qualifiers, name);

  return handleClassLookupRes(lookupRes, qualifiers, name, line);
}


pair<const Ctx::ClassInfo*, const Ctx::FnInfo*> Ctx::lookupMethod(
    int id,
    const string& className,
    const string& methodName,
    const vector<TypePtr>& paramTypes,
    size_t line) const {
  const ClassInfo* classInfo = lookupClass(id);
  if (!classInfo) {
    // If we hit this case, we already logged an undefined class error somewhere else, so don't
    // duplicate it
    return { nullptr, nullptr };
  }

  LookupRes methodLookup = lookupFn(classInfo->methods, methodName, paramTypes);
  methodLookup.searchedPath = classInfo->declFile;
  return {
    classInfo,
    handleFnLookupRes(
        methodLookup, {}, string(className).append("::").append(methodName), paramTypes, line)
  };
}


const Ctx::ClassInfo* Ctx::handleClassLookupRes(
    const ClsLookupRes& lookupRes,
    const vector<string>& qualifiers,
    const string& name,
    size_t line) const {
  switch (lookupRes.status) {
    case LookupStatus::FOUND:
      return get<const ClassInfo*>(lookupRes.result);
    case LookupStatus::UNDEFINED:
      undefinedClass(qualifiers, name, line, lookupRes.searchedPath);
      return nullptr;
    case LookupStatus::BAD_QUALS:
      undefClassBadQuals(qualifiers, name, line);
      return nullptr;
    case LookupStatus::AMBIG_QUALS:
      undefClassAmbigQuals(
          qualifiers,
          name,
          line,
          get<vector<const string*>>(lookupRes.result),
          lookupRes.searchedPath);
      return nullptr;
    default:
      throw invalid_argument("Ctx::lookupClassRec");
  }
}

void Ctx::insertFn(
    const string& name,
    const vector<TypePtr>& paramTypes,
    const TypePtr& returnType,
    size_t id,
    size_t line) {
  insertMethod(fnMap_, "", name, paramTypes, returnType, false, id, line);
}

void Ctx::insertMethod(
    unordered_multimap<string, Ctx::FnInfo>& funcMap,
    string_view className,
    const string& name,
    const vector<TypePtr>& paramTypes,
    const TypePtr& returnType,
    bool isVirtual,
    size_t id,
    size_t line) {
  auto iterPair = funcMap.equal_range(name);
  for (auto iter = iterPair.first; iter != iterPair.second; ++iter) {
    const FnInfo& fnInfo = iter->second;
    const vector<TypePtr>& fnParamTypes = fnInfo.paramTypes;
    if (equal(paramTypes.cbegin(), paramTypes.cend(), fnParamTypes.cbegin(), fnParamTypes.cend())) {
      ostream& errStream = logger.logError(line);
      errStream << "Redefinition of function '"
                << (className.empty() ? name : string(className).append("::").append(name));
      Ctx::streamParamTypes(paramTypes, errStream);
      errStream << "'. Originally declared at " << fnInfo.declFile << ", line " << fnInfo.line;
      return;
    }
  }
  funcMap.emplace(name, FnInfo{ paramTypes, returnType, isVirtual, id, filename_, line });
}


Ctx::FnLookupRes Ctx::lookupFn(const string& name, const vector<TypePtr>& paramTypes) const {
  return lookupFn(fnMap_, name, paramTypes);
}

Ctx::FnLookupRes Ctx::lookupFn(
    const unordered_multimap<string, FnInfo>& funcMap,
    const string& name,
    const vector<TypePtr>& paramTypes) const {
  vector<const FnInfo*> wideMatches;
  vector<const FnInfo*> narrowMatches;
  auto iterPair = funcMap.equal_range(name);

  for (auto iter = iterPair.first; iter != iterPair.second; ++iter) {
    const vector<TypePtr>& fnParamTypes = iter->second.paramTypes;
    bool isNarrowing = false;
    if (equal(paramTypes.cbegin(), paramTypes.cend(), fnParamTypes.cbegin(), fnParamTypes.cend())) {
      // Exact match
      return { LookupStatus::FOUND, vector<const FnInfo*>{ &iter->second }, filename_ };
    } else if (equal(
                   paramTypes.cbegin(),
                   paramTypes.cend(),
                   fnParamTypes.cbegin(),
                   fnParamTypes.cend(),
                   [&isNarrowing, this](const TypePtr& from, const TypePtr& to) {
                     bool b;
                     bool res = from->isConvertibleTo(*to, &b, *this);
                     isNarrowing |= b;
                     return res;
                   })) {
      // Implicit conversion match
      if (isNarrowing && wideMatches.empty()) {
        // No need to keep track of narrowing matches if we already found a
        // widening match
        narrowMatches.push_back(&iter->second);
      } else if (!isNarrowing) {
        wideMatches.push_back(&iter->second);
      }
    }
  }

  // Only try narrowing matches if there are no widening matches
  const vector<const FnInfo*>* matchVec = wideMatches.empty() ? &narrowMatches : &wideMatches;
  if (matchVec->size() > 1) {
    return { LookupStatus::AMBIG_OVERLOAD, move(*matchVec), filename_ };
  } else if (matchVec->size() == 1) {
    return { matchVec == &narrowMatches ? LookupStatus::NARROWING : LookupStatus::FOUND,
             { move(*matchVec) },
             filename_ };
  } else {
    // No matches
    vector<const FnInfo*> candidates;
    transform(iterPair.first, iterPair.second, back_inserter(candidates), [](const auto& p) {
      return &p.second;
    });
    return { LookupStatus::UNDEFINED, move(candidates), filename_ };
  }
}


// TODO: Allow "from <file> import <function/class>" ???
const Ctx::FnInfo* Ctx::lookupFnRec(
    const vector<string>& qualifiers,
    const string& name,
    const vector<TypePtr>& paramTypes,
    size_t line) const {
  LookupRes lookupRes = qualifiers.empty() ? lookupFn(name, paramTypes)
                                           : ctxTree_.lookupFn(qualifiers, name, paramTypes);
  return handleFnLookupRes(lookupRes, qualifiers, name, paramTypes, line);
}

const Ctx::FnInfo* Ctx::handleFnLookupRes(
    const FnLookupRes& lookupRes,
    const vector<string>& qualifiers,
    const string& name,
    const vector<TypePtr>& paramTypes,
    size_t line) const {
  switch (lookupRes.status) {
    case LookupStatus::NARROWING: {
      const FnInfo* fnInfo = get<vector<const FnInfo*>>(lookupRes.result).front();
      // This function duplicates some work we already did, but it would take
      // more effort to keep track of the narrowing conversions as we search for
      // a feasible overload
      warnNarrow(paramTypes, fnInfo->paramTypes, line);
      return fnInfo;
    }
    case LookupStatus::FOUND:
      return get<vector<const FnInfo*>>(lookupRes.result).front();
    case LookupStatus::AMBIG_OVERLOAD:
      ambigOverload(
          qualifiers,
          name,
          paramTypes,
          line,
          get<vector<const FnInfo*>>(lookupRes.result),
          lookupRes.searchedPath);
      return nullptr;
    case LookupStatus::UNDEFINED:
      undefinedFn(
          qualifiers,
          name,
          paramTypes,
          line,
          get<vector<const FnInfo*>>(lookupRes.result),
          lookupRes.searchedPath);
      return nullptr;
    case LookupStatus::BAD_QUALS:
      undefFnBadQuals(qualifiers, name, paramTypes, line);
      return nullptr;
    case LookupStatus::AMBIG_QUALS:
      undefFnAmbigQuals(
          qualifiers,
          name,
          paramTypes,
          line,
          get<vector<const string*>>(lookupRes.result),
          lookupRes.searchedPath);
      return nullptr;
    default:
      throw invalid_argument("Ctx::lookupFnRec");
  }
}


void Ctx::undefinedFn(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line,
    const vector<const FnInfo*>& candidates,
    string_view searchedFile) const {
  ostream& err = logger.logError(line);
  err << "Undefined function '" << lang_utils::qualifiedName(qualifiers, fnName);
  streamParamTypes(paramTypes, err);

  if (candidates.empty()) {
    err << "'\nNo candidate functions in " << searchedFile;
  } else {
    err << "'\nCandidate functions in " << searchedFile << ":";
    for (const FnInfo* fnInfo : candidates) {
      err << "\n\tLine " << fnInfo->line << ": " << *fnInfo->returnType << ' ' << fnName;
      streamParamTypes(fnInfo->paramTypes, err);
    }
  }
}

void Ctx::undefFnBadQuals(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line) const {
  ostream& err = logger.logError(line);
  err << "Undefined function '" << lang_utils::qualifiedName(qualifiers, fnName);
  streamParamTypes(paramTypes, err);
  err << "'. No imported file matches qualifiers.";
}

void Ctx::undefFnAmbigQuals(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line,
    const vector<const string*> candidates,
    string_view searchedPath) const {
  ostream& err = logger.logError(line);
  err << "Ambiguous qualifier for function '" << lang_utils::qualifiedName(qualifiers, fnName);
  streamParamTypes(paramTypes, err);
  err << "'. Found";
  for (const string* cand : candidates) {
    err << "\n\t" << *cand << "::" << searchedPath;
  }
}

void Ctx::ambigOverload(
    const vector<string>& qualifiers,
    string_view fnName,
    const vector<TypePtr>& paramTypes,
    size_t line,
    const vector<const Ctx::FnInfo*>& candidates,
    string_view searchedFile) const {
  ostream& err = logger.logError(line);
  err << "Call of overloaded function '" << lang_utils::qualifiedName(qualifiers, fnName);
  streamParamTypes(paramTypes, err);
  err << "' is ambiguous. Candidate functions in " << searchedFile << ":";
  for (const FnInfo* fnInfo : candidates) {
    err << "\n\tLine " << fnInfo->line << ": " << *fnInfo->returnType << ' ' << fnName;
    streamParamTypes(fnInfo->paramTypes, err);
  }
}

void Ctx::warnNarrow(
    const vector<TypePtr>& fromTypes, const vector<TypePtr>& toTypes, size_t line) const {
  size_t len = fromTypes.size();
  for (size_t i = 0; i < len; ++i) {
    const Type& from = *fromTypes[i];
    const Type& to = *toTypes[i];
    if (from.numBytes > to.numBytes) {
      ostringstream& warn = logger.logWarning(line);
      warn << "Narrowing conversion from " << from << " to " << to;
    }
  }
}


void Ctx::undefinedClass(
    const vector<string>& qualifiers,
    string_view className,
    size_t line,
    string_view searchedFile) const {
  ostream& err = logger.logError(line);
  err << "Class '" << lang_utils::qualifiedName(qualifiers, className) << "' is not defined in "
      << searchedFile;
}

void Ctx::undefClassBadQuals(
    const vector<string>& qualifiers, string_view className, size_t line) const {
  ostream& err = logger.logError(line);
  err << "Undefined class '" << lang_utils::qualifiedName(qualifiers, className)
      << "'. No imported file matches qualifiers.";
}

void Ctx::undefClassAmbigQuals(
    const vector<string>& qualifiers,
    string_view className,
    size_t line,
    const vector<const string*> candidates,
    string_view searchedPath) const {
  ostream& err = logger.logError(line);
  err << "Ambiguous qualifier for function '" << lang_utils::qualifiedName(qualifiers, className) << "'. Found";
  for (const string* cand : candidates) {
    err << "\n\t" << *cand << "::" << searchedPath;
  }
}

void Ctx::checkType(Type& type, size_t line) const {
  // Built-in types are always valid
  if (type.typeName != Type::Category::CLASS) {
    return;
  }

  // Already checked this one
  Class& classTy = static_cast<Class&>(type);
  if (classTy.id != Class::ID_EMPTY) {
    return;
  }

  const ClassInfo* classInfo = lookupClassRec(classTy.qualifiers, classTy.className, line);
  if (!classInfo) {
    // Prevent doing the check again
    classTy.id = Class::ID_UNKNOWN;
  } else {
    classTy.id = classInfo->id;
  }
}


string Ctx::mangleFn(string_view fnName, size_t id) {
  if (fnName.starts_with("__") || fnName == "main") {
    return string(fnName);
  }

  return string(fnName).append(1, '_').append(to_string(id));
}

void Ctx::displayLogs() const { logger.streamLog(); }

bool Ctx::hasErrors() const noexcept { return logger.hasErrors(); }
