#include "src/language/language.hpp"

using namespace std;


namespace language {


string newLabel() {
  static int i = 0;
  return "L" + to_string(i++);
}


optional<string> mangleFnName(string_view fnName, string_view filename) {
  // TODO: Remove runPrez and printInt when applicable
  if (fnName.front() == '_' || fnName == "runprez" || fnName == "printInt") {
    return {};
  }

  string newName = string("_").append(fnName);
  filename = filename.substr(0, filename.size() - sizeof(".prez") + 1);
  newName.append(filename).push_back('_');
  replace(newName.begin(), newName.end(), '/', '_');
  return newName;
}


tuple<string, vector<im::ExprPtr>, TypePtr> argsToImExprs(
    const vector<string>& qualifiers,
    const string& fnName,
    const vector<ExprPtr>& params, size_t line, Ctx& ctx) {
  // Ensure function was declared
  const Ctx::FnInfo& fnInfo = ctx.lookupFnRec(qualifiers, fnName, line);
  // Ensure parameter types match and translate them to intermediate exprs
  const vector<TypePtr>& paramTypes = fnInfo.paramTypes;
  size_t expectedNumParams = paramTypes.size();
  size_t numParams = params.size();
  if (numParams != paramTypes.size()) {
    auto& errStream = ctx.getLogger().logError(line);
    errStream << "Wrong number of arguments for function: \"" << fnName
              << "\". Expected " << expectedNumParams << ", got " << numParams
              << ". Originally declared at " << fnInfo.declFile << ", line " << fnInfo.line;
  }
  vector<im::ExprPtr> argsCode;
  argsCode.reserve(numParams);
  // NOTE: We use the minimum of the two so that we don't segfault if they aren't equal.
  // This allows us to continue compiling and report other errors
  size_t smaller = min(numParams, expectedNumParams);
  for (size_t i = 0; i < smaller; ++i) {
    argsCode.push_back(params[i]->toImExprAssert(*paramTypes[i], ctx));
  }

  if (optional<string> newFnName = mangleFnName(fnName, fnInfo.declFile)) {
    return { *newFnName, move(argsCode), fnInfo.returnType };
  }

  return { fnName, move(argsCode), fnInfo.returnType };
}

}  // namespace language
