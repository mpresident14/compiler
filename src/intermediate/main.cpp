#include "src/intermediate/intermediate.hpp"
#include "src/language/language.hpp"
#include "src/x86gen/flow_graph.hpp"
#include "src/x86gen/function.hpp"
#include "src/x86gen/instruction.hpp"
#include "src/x86gen/temp.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace im;
namespace lg = language;

// int main() {
//   vector<InstrPtr> instrs;

//   ExprPtr expr1(new Const(1));
//   ExprPtr expr2(new Const(3));
//   ExprPtr binop(new BinOp(move(expr1), move(expr2), Bop::DIV));
//   binop->toInstrs(R15, instrs);

//   Function fn("runprez", move(instrs));

//   ofstream asmFile("prez.s");
//   asmFile << ".text\n.globl runprez\n.align 16\n";
//   fn.toCode(asmFile);
// }


// int main() {
//   vector<InstrPtr> instrs;

//   ExprPtr expr1(new Const(1));
//   ExprPtr expr2(new Const(3));
//   ExprPtr binop(new BinOp(move(expr1), move(expr2), Bop::XOR));
//   ExprPtr fnLabel(new LabelAddr("f"));
//   vector<ExprPtr> params;
//   params.emplace_back(move(binop));
//   ExprPtr call(new CallExpr(move(fnLabel), move(params), true));
//   call->toInstrs(R15, instrs);

//   Function fn("runprez", move(instrs));

//   ofstream asmFile("prez.s");
//   asmFile << ".text\n.globl runprez\n.align 16\n";
//   fn.toCode(asmFile);
// }


int main() {
  // lg::If ifStmt(
  //     lg::BinaryOp(lg::ConstInt(1), lg::ConstInt(2), lg::BOp::LT),
  //     make_unique<lg::Block>(vector<lg::StmtPtr{make_unique<})
  // )
  vector<im::StmtPtr> imStmts;
  lg::VarDecl varDecl(intType, "n", make_unique<lg::ConstInt>(1));
  varDecl.toImStmts(imStmts);
}
