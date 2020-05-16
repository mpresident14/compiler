#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/language/typecheck/type.hpp"

#include <vector>
#include <unordered_map>


class Context {
public:
  struct VarInfo {
    TypePtr type;
    int temp;
  };

  struct FnInfo {
    std::vector<TypePtr> paramTypes;
    TypePtr returnType;
  };

  Context() = default;
  ~Context() = default;
  Context(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(const Context&) = delete;
  Context& operator=(Context&&) = delete;

  const std::unordered_map<std::string, VarInfo>& getVarMap() const noexcept {
    return varMap_;
  }

  const std::unordered_map<std::string, FnInfo>& getFnMap() const noexcept {
    return fnMap_;
  }

  const TypePtr& getReturnTy() const noexcept {
    return returnType_;
  }

  int insertVar(const std::string& name, TypePtr&& type);
  const VarInfo& lookupVar(const std::string& name) const;

  void insertFn(const std::string& name, std::vector<TypePtr>&& paramTypes, TypePtr&& returnType);
  const FnInfo& lookupFn(const std::string& name) const;



private:
  std::unordered_map<std::string, VarInfo> varMap_;
  std::unordered_map<std::string, FnInfo> fnMap_;
  TypePtr returnType_;
};

#endif // CONTEXT_HPP
