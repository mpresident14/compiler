#include "flow_graph.hpp"
#include "function.hpp"
#include "instruction.hpp"
#include "temp.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

int main() {
  InstrPtr moveImm0 =
      make_unique<Operation>("movq $1, <0", vector<int>{}, vector<int>{RAX},
                             optional<vector<Instruction *>>{});
  InstrPtr moveImm1 =
      make_unique<Operation>("movq $5, <0", vector<int>{}, vector<int>{-1},
                             optional<vector<Instruction *>>{});
  InstrPtr leaq = make_unique<Operation>("leaq (>0, >1, 2), <0",
                                         vector<int>{RAX, -1}, vector<int>{RAX},
                                         optional<vector<Instruction *>>{});
  InstrPtr move0 = make_unique<Move>("movq >, <", RAX, -1);
  InstrPtr move1 = make_unique<Move>("movq >, <", -1, -2);
  InstrPtr move2 = make_unique<Move>("movq >, <", -2, RCX);

  InstrPtr cmp =
      make_unique<Operation>("cmpq $11, <0", vector<int>{}, vector<int>{RCX},
                             optional<vector<Instruction *>>{});
  InstrPtr eqLabel = make_unique<Label>("EQ");
  InstrPtr doneLabel = make_unique<Label>("FIN");
  InstrPtr moveEq = make_unique<Move>("movq >, <", RCX, RDI);
  InstrPtr moveNeq =
      make_unique<Operation>("movq $-1, <0", vector<int>{}, vector<int>{RDI},
                             optional<vector<Instruction *>>{});
  InstrPtr je = make_unique<Operation>(
      "je EQ", vector<int>{}, vector<int>{},
      optional<vector<Instruction *>>{{eqLabel.get(), doneLabel.get()}});
  InstrPtr jDone = make_unique<Operation>(
      "jmp FIN", vector<int>{}, vector<int>{},
      optional<vector<Instruction *>>{{doneLabel.get()}});

  InstrPtr callPrint = make_unique<Operation>(
      "callq printInt", vector<int>(callerSaveRegs),
      vector<int>(calleeSaveRegs), optional<vector<Instruction *>>{});

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

  Function fn("runprez", move(instrs));

  ofstream asmFile("prez.s");
  asmFile << ".text\n.globl runprez\n.align 16\n";
  fn.toCode(asmFile);
}
