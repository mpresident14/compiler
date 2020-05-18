#include "src/language/language.hpp"

using namespace std;

namespace language {

Context& ctx = Context::getContext();

string newLabel() {
  static int i = 0;
  return "L" + to_string(i++);
}

pair<vector<im::ExprPtr>, Type> argsToImExprs(const string& fnName, const vector<ExprPtr>& params) {
  // Ensure function was declared
  const Context::FnInfo& fnInfo = ctx.lookupFn(fnName);
  // Ensure parameter types match and translate them to intermediate exprs
  const vector<Type>& paramTypes = fnInfo.paramTypes;
  size_t numParams = params.size();
  if (numParams != paramTypes.size()) {
    typeError("Wrong number of arguments for function: " + fnName);
  }
  std::vector<im::ExprPtr> argsCode;
  argsCode.reserve(numParams);
  for (size_t i = 0; i < numParams; ++i) {
    argsCode.push_back(params[i]->toImExprAssert(paramTypes[i]));
  }
  return { move(argsCode), fnInfo.returnType };
}

}
