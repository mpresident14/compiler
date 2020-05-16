#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/language/typecheck/type.hpp"

#include <vector>
#include <unordered_map>


class Context {
public:
  Context() = delete;
  ~Context() = default;
  Context(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(const Context&) = delete;
  Context& operator=(Context&&) = delete;

  struct FnInfo {
    std::vector<TypePtr> paramTypes;
    TypePtr returnType;
  };

private:
  std::unordered_map<std::string, FnInfo> fnMap_;

};

#endif // CONTEXT_HPP
