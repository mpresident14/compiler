#include "src/x86gen/temp.hpp"
#include "src/x86gen/instruction.hpp"
#include "src/x86gen/flow_graph.hpp"
#include "src/x86gen/function.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

void writeTest0() {
  InstrPtr moveImm0 = make_unique<Operation>(
      "movq $1, <0",
      vector<int>{},
      vector<int>{ RAX },
      optional<vector<Instruction*>>{});
  InstrPtr moveImm1 = make_unique<Operation>(
      "movq $5, <0",
      vector<int>{},
      vector<int>{ -1 },
      optional<vector<Instruction*>>{});
  InstrPtr leaq = make_unique<Operation>(
      "leaq (>0, >1, 2), <0",
      vector<int>{RAX, -1},
      vector<int>{ RAX },
      optional<vector<Instruction*>>{});
  InstrPtr move0 = make_unique<Move>("movq >, <", RAX, -1);
  InstrPtr move1 = make_unique<Move>("movq >, <", -1, -2);
  InstrPtr move2 = make_unique<Move>("movq >, <", -2, RCX);

  InstrPtr cmp = make_unique<Operation>(
      "cmpq $11, <0",
      vector<int>{},
      vector<int>{ RCX },
      optional<vector<Instruction*>>{});
  InstrPtr eqLabel = make_unique<Label>("EQ");
  InstrPtr doneLabel = make_unique<Label>("FIN");
  InstrPtr moveEq = make_unique<Move>("movq >, <", RCX, RDI);
  InstrPtr moveNeq = make_unique<Operation>(
      "movq $-1, <0",
      vector<int>{},
      vector<int>{ RDI },
      optional<vector<Instruction*>>{});
  InstrPtr je = make_unique<Operation>(
      "je EQ",
      vector<int>{},
      vector<int>{},
      optional<vector<Instruction*>>{{eqLabel.get(), doneLabel.get()}});
  InstrPtr jDone = make_unique<Operation>(
      "jmp FIN",
      vector<int>{},
      vector<int>{},
      optional<vector<Instruction*>>{{doneLabel.get()}});
  InstrPtr callPrint = make_unique<Operation>(
      "callq printInt",
      vector<int>(callerSaveRegs),
      vector<int>(calleeSaveRegs),
      optional<vector<Instruction*>>{});

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

  vector<Function> fns;
  fns.emplace_back("runprez", move(instrs));
  Program program("test0", move(fns));

  ofstream asmFile("test0.s");
  if (!asmFile.is_open()) {
    cout << "WAAH" << endl;
  }

  program.toCode(asmFile);
}


int main() {
  writeTest0();

  return 0;
}
