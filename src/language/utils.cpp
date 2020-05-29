#include "src/language/language.hpp"

using namespace std;

// TODO(everywhere): Provide more informative errors and with line #s

namespace language {


string newLabel() {
  static int i = 0;
  return "L" + to_string(i++);
}

pair<vector<im::ExprPtr>, Type> argsToImExprs(
    const string& fnName,
    const vector<ExprPtr>& params, size_t line) {
  // Ensure function was declared
  const ctx::FnInfo& fnInfo = ctx::lookupFn(fnName, line);
  // Ensure parameter types match and translate them to intermediate exprs
  const vector<Type>& paramTypes = fnInfo.paramTypes;
  size_t expectedNumParams = paramTypes.size();
  size_t numParams = params.size();
  if (numParams != paramTypes.size()) {
    auto& errStream = ctx::getLogger().logError(line);
    errStream << "Wrong number of arguments for function: \"" << fnName
              << "\". Expected " << expectedNumParams << ", got " << numParams
              << ". Originally declared at " << fnInfo.declFile << ", line " << fnInfo.line;
  }
  std::vector<im::ExprPtr> argsCode;
  argsCode.reserve(numParams);
  // NOTE: We use the minimum of the two so that we don't segfault if they aren't equal.
  // This allows us to continue compiling and report other errors
  size_t smaller = min(numParams, expectedNumParams);
  for (size_t i = 0; i < smaller; ++i) {
    argsCode.push_back(params[i]->toImExprAssert(paramTypes[i]));
  }
  return { move(argsCode), fnInfo.returnType };
}

}  // namespace language
