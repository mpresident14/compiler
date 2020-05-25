#include "src/intermediate/intermediate.hpp"
#include "src/language/language.hpp"
#include "src/assembly/reg_alloc/flow_graph.hpp"
#include "src/assembly/assembly.hpp"
#include "src/assembly/temp.hpp"
#include "src/main/parser.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// using namespace std;
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

void compile(const lg::Program& prog, const std::string& fileName) {
  prog.toImProg().toAssemProg().toCode(fileName);
}


// int main() {
//   vector<lg::StmtPtr> stmts;
//   stmts.emplace_back(new lg::VarDecl(intType, "n", make_unique<lg::ConstInt>(1)));
//   stmts.emplace_back(new lg::VarDecl(intType, "m", make_unique<lg::ConstInt>(2)));

//   vector<lg::DeclPtr> decls;
//   decls.emplace_back(new lg::Func(
//     voidType,
//     "f",
//     std::vector<std::pair<std::string, Type>>(),
//     unique_ptr<lg::Block>(new lg::Block(move(stmts)))));

//   lg::Program prog(move(decls));
//   compile(prog, "prez.s");
// }

int main(int, char** argv) {
  std::ifstream in(argv[1]);
  if (!in.is_open()) {
    throw std::invalid_argument("File isn't open you dope");
  }
  language::Program prog = parser::parse(in);
  compile(prog, argv[2]);
}
