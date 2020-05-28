#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "src/assembly/temp.hpp"
#include "src/language/typecheck/type.hpp"
#include "src/misc/logger.hpp"

#include <unordered_map>
#include <vector>

namespace ctx {

struct VarInfo {
  Type type;
  int temp;
};

struct FnInfo {
  std::vector<Type> paramTypes;
  Type returnType;
};

void setCurrentFile(std::string_view fileName);
Logger& addLogger(std::string_view srcFileName);
Logger& getLogger();
void setCurrentFn(std::string_view fnName);
const std::string& getCurrentFn() noexcept;
int insertVar(std::string_view name, const Type& type, size_t line);
void insertParam(std::string_view name, const Type& type, MachineReg reg, size_t line);
const VarInfo& lookupVar(std::string_view name, size_t line);
void removeVar(std::string_view name);
void insertFn(
    std::string_view name,
    std::vector<Type>&& paramTypes,
    const Type& returnType, size_t line);
const FnInfo& lookupFn(std::string_view name, size_t line);

/* Returns true if there was an error */
bool displayLogs();

}


#endif  // CONTEXT_HPP
