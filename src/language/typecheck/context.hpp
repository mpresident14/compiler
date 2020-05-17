#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/language/typecheck/type.hpp"

#include <vector>
#include <unordered_map>


class Context {
public:
  struct VarInfo {
    Type type;
    int temp;
  };

  struct FnInfo {
    std::vector<Type> paramTypes;
    Type returnType;
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

  const Type& getReturnTy() const noexcept {
    return returnType_;
  }

  int insertVar(const std::string& name, const Type& type);
  const VarInfo& lookupVar(const std::string& name) const;

  void removeVar(const std::string& name);

  void insertFn(const std::string& name, std::vector<Type>&& paramTypes, const Type& returnType);
  const FnInfo& lookupFn(const std::string& name) const;



private:
  std::unordered_map<std::string, VarInfo> varMap_;
  std::unordered_map<std::string, FnInfo> fnMap_;
  Type returnType_;
};

#endif // CONTEXT_HPP
