#include "src/assembly/reg_alloc/flow_graph.hpp"
#include "src/assembly/assembly.hpp"
#include "src/assembly/temp.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace assem;

void writeTest0() {
  InstrPtr moveImm0 =
      make_unique<Operation>("movq $1, `D0", vector<int>{}, vector<int>{ RAX });
  InstrPtr moveImm1 =
      make_unique<Operation>("movq $5, `D0", vector<int>{}, vector<int>{ -1 });
  InstrPtr leaq = make_unique<Operation>(
      "leaq (`S0, `S1, 2), `D0", vector<int>{ RAX, -1 }, vector<int>{ RAX });
  InstrPtr move0 = make_unique<Move>(RAX, -1);
  InstrPtr move1 = make_unique<Move>(-1, -2);
  InstrPtr move2 = make_unique<Move>(-2, RSI);

  InstrPtr cmp = make_unique<Operation>(
      "cmpq $11, `S0", vector<int>{ RSI }, vector<int>{});
  InstrPtr eqLabel = make_unique<Label>("EQ");
  InstrPtr doneLabel = make_unique<Label>("FIN");
  InstrPtr moveEq = make_unique<Move>(RSI, RDI);
  InstrPtr moveNeq = make_unique<Operation>(
      "movq $-1, `D0", vector<int>{}, vector<int>{ RDI });
  InstrPtr je = make_unique<CondJumpOp>(
      "je EQ",
      vector<int>{},
      vector<int>{},
      vector<Instruction*>{ eqLabel.get() });
  InstrPtr jDone = make_unique<JumpOp>(
      "jmp FIN",
      vector<int>{},
      vector<int>{},
      vector<Instruction*>{ doneLabel.get() });
  InstrPtr callPrint = make_unique<Operation>(
      "callq printInt",
      regsAsInts(CALLER_SAVE_REGS),
      regsAsInts(CALLEE_SAVE_REGS));

  vector<InstrPtr> instrs;
  instrs.push_back(move(moveImm0));
  instrs.push_back(move(moveImm1));
  instrs.push_back(move(leaq));
  instrs.push_back(move(move0));
  instrs.push_back(move(move1));
  instrs.push_back(move(move2));
  instrs.push_back(move(cmp));
  instrs.push_back(move(je));
  instrs.push_back(move(moveNeq));
  instrs.push_back(move(jDone));
  instrs.push_back(move(eqLabel));
  instrs.push_back(move(moveEq));
  instrs.push_back(move(doneLabel));
  instrs.push_back(move(callPrint));
  instrs.push_back(make_unique<Return>(false));

  vector<DeclPtr> decls;
  decls.emplace_back(new Function("runprez", move(instrs)));
  Program program("test0", move(decls));
  program.toCode();
}

int main() {
  writeTest0();

  return 0;
}
