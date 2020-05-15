#include "src/x86gen/flow_graph.hpp"
#include "src/x86gen/function.hpp"
#include "src/x86gen/instruction.hpp"
#include "src/x86gen/temp.hpp"
#include "src/intermediate/intermediate.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace intermediate;

int main() {
  vector<InstrPtr> instrs;

  ExprPtr expr1(new Const(1));
  ExprPtr expr2(new Const(3));
  ExprPtr binop(new BinOp(move(expr1), move(expr2), Bop::DIV));
  binop->toInstrs(R15, instrs);

  Function fn("runprez", move(instrs));

  ofstream asmFile("prez.s");
  asmFile << ".text\n.globl runprez\n.align 16\n";
  fn.toCode(asmFile);
}
