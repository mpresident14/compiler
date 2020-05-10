#include "temp.hpp"
#include "instruction.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

int main() {
  unique_ptr<Instruction> moveImm = make_unique<Operation>(
      "movq $3, D0",
      vector<int>{},
      vector<int>{ RAX },
      optional<vector<string>>{});
  unique_ptr<Instruction> move0 = make_unique<Move>("movq S, D", RAX, 0);
  unique_ptr<Instruction> move1 = make_unique<Move>("movq S, D", 0, 1);
  unique_ptr<Instruction> move2 = make_unique<Move>("movq S, D", 1, RCX);
  unique_ptr<Instruction> move3 = make_unique<Move>("movq S, D", RCX, RDI);
  unique_ptr<Instruction> callPrint = make_unique<Operation>(
      "callq printInt",
      vector<int>{},
      vector<int>(callerSaveRegs),
      optional<vector<string>>{});

  vector<unique_ptr<Instruction>> instrs;
  instrs.push_back(move(moveImm));
  instrs.push_back(move(move0));
  instrs.push_back(move(move1));
  instrs.push_back(move(move2));
  instrs.push_back(move(move3));
  instrs.push_back(move(callPrint));

  Function fn("runprez", move(instrs));

  ofstream asmFile("prez.s");
  asmFile << ".text\n.globl runprez\n.align 16\n";
  fn.toCode(asmFile);
}
