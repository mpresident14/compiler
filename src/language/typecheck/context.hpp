#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/assembly/temp.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/misc/logger.hpp"

#include <utility>
#include <vector>

namespace ctx {

struct VarInfo {
  TypePtr type;
  int temp;
  size_t line;
  bool used;
};

struct FnInfo {
  std::vector<TypePtr> paramTypes;
  TypePtr returnType;
  std::string declFile;
  size_t line;
};

void setCurrentFile(const std::string& fileName);
Logger& addLogger(const std::string& srcFileName);
Logger& getLogger();
void setCurrentFn(std::string_view fnName);
const std::string& getCurrentFn() noexcept;
int insertVar(std::string_view name, TypePtr type, size_t line);
const VarInfo& lookupVar(const std::string& name, size_t line);
void removeVars(const std::vector<std::pair<std::string, size_t>>& vars);
void removeParams(const std::vector<std::string>& params, size_t line);
void insertFn(
    std::string_view name,
    const std::vector<TypePtr>& paramTypes,
    TypePtr returnType,
    size_t line);
const FnInfo& lookupFn(const std::string& name, size_t line);

/* Returns true if there was an error */
bool displayLogs();

}  // namespace ctx


#endif  // CONTEXT_HPP
