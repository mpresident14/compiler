#include "src/language/typecheck/context.hpp"
#include "src/x86gen/temp.hpp"

#include <stdexcept>

using namespace std;

int Context::insertVar(const std::string& name, TypePtr&& type) {
  int temp = newTemp();
  varMap_.emplace(name, VarInfo{ move(type), temp });
  return temp;
}

const Context::VarInfo& Context::lookupVar(const std::string& name) const {
  auto iter = varMap_.find(name);
  if (iter == varMap_.end()) {
    throw invalid_argument("Undefined variable \"" + name + "\"");
  }
  return iter->second;
}

void Context::insertFn(const std::string& name, std::vector<TypePtr>&& paramTypes, TypePtr&& returnType) {
  fnMap_.emplace(name, FnInfo{ move(paramTypes), move(returnType) });
}

const Context::FnInfo& Context::lookupFn(const std::string& name) const {
  auto iter = fnMap_.find(name);
  if (iter == fnMap_.end()) {
    throw invalid_argument("Undefined function \"" + name + "\"");
  }
  return iter->second;
}
