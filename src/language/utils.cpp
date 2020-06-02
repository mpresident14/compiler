#include "src/language/language.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/typecheck/type.hpp"

using namespace std;


namespace language {


string newLabel() {
  static int i = 0;
  return "L" + to_string(i++);
}


tuple<string, vector<im::ExprPtr>, TypePtr> argsToImExprs(
    const vector<string>& qualifiers,
    const string& fnName,
    const vector<ExprPtr>& params,
    size_t line,
    Ctx& ctx) {

  vector<im::ExprPtr> paramImExprs;
  vector<TypePtr> paramTypes;
  size_t numParams = params.size();
  // Get types and convert params
  for (size_t i = 0; i < numParams; ++i) {
    ExprInfo exprInfo = params[i]->toImExpr(ctx);
    paramImExprs.push_back(move(exprInfo.imExpr));
    paramTypes.push_back(move(exprInfo.type));
  }

  const Ctx::FnInfo* fnInfo = ctx.lookupFnRec(qualifiers, fnName, paramTypes, line);
  // TODO: Continue on failed lookup
  // if (!fnInfo) {
  //   ctx.undefinedFn(qualifiers, fnName, paramTypes, line);
  // }

  return { ctx.mangleFn(fnName, fnInfo->declFile, paramTypes), move(paramImExprs), fnInfo->returnType };
}

}  // namespace language
