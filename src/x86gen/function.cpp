#include "src/x86gen/flow_graph.hpp"
#include "src/x86gen/instruction.hpp"
#include "src/x86gen/interference_graph.hpp"
#include "src/x86gen/temp.hpp"

#include <fstream>
#include <stdexcept>

using namespace std;

namespace x86_64 {


Function::Function(const std::string& name, std::vector<InstrPtr>&& instrs)
    : name_(name), instrs_(move(instrs)) {}

void Function::regAlloc() {
  FlowGraph fgraph(instrs_);
  fgraph.computeLiveness();
  InterferenceGraph igraph(fgraph);
  auto colorPair = igraph.color();
  const unordered_map<int, MachineReg>& coloring = colorPair.first;
  const vector<int>& spilled = colorPair.second;

  for (int tempId : spilled) {
    if (!varToStackOffset_.contains(tempId)) {
      varToStackOffset_.emplace(tempId, 8 * varToStackOffset_.size());
    }
  }

  // Allocate space on the stack for spilled variables
  size_t stackSpace = varToStackOffset_.size() * 8;
  vector<InstrPtr> newInstrs;
  newInstrs.push_back(make_unique<Operation>(
      "subq $" + to_string(stackSpace) + ", %rsp",
      vector<int>{},
      vector<int>{}));

  // Update the instructions and add them
  // TODO: Lots of virtual function calls here, either use a switch
  // or group them together
  for (InstrPtr& instr : instrs_) {
    if (instr->getType() == InstrType::RETURN) {
      // Deallocate space on the stack for spilled variables
      newInstrs.push_back(make_unique<Operation>(
          "addq $" + to_string(stackSpace) + ", %rsp",
          vector<int>{},
          vector<int>{}));
      newInstrs.push_back(move(instr));
    } else {
      instr->assignRegs(coloring);
      if (instr->spillTemps(newInstrs)) {
        newInstrs.push_back(move(instr));
      }
    }
  }

  instrs_ = move(newInstrs);
}


void Function::toX86_64(std::ostream& out) {
  regAlloc();
  out << name_ << ":\n";
  for (const InstrPtr& instr : instrs_) {
    instr->toCode(out, varToStackOffset_);
  }
}


void Program::toX86_64(const std::string& fileName) {
  ofstream asmFile(fileName);
  if (!asmFile.is_open()) {
    throw invalid_argument("Could not open file " + fileName);
  }

  asmFile << ".text\n.globl runprez\n.align 16\n";
  for (DeclPtr& decl : decls) {
    decl->toX86_64(asmFile);
  }
}


}
