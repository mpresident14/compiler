#include "src/assembly/function.hpp"

#include "src/assembly/temp.hpp"
#include "src/assembly/instruction.hpp"
#include "src/assembly/flow_graph.hpp"
#include "src/assembly/interference_graph.hpp"

using namespace std;

Function::Function(
    std::string&& name,
    std::vector<InstrPtr>&& instrs)
    : name_(move(name)), instrs_(move(instrs)) {}

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
      vector<int>{},
      optional<vector<Instruction*>>{}));

  // Update the instructions and add them
  for (InstrPtr& instr : instrs_) {
    instr->assignRegs(coloring);
    if (instr->spillTemps(newInstrs)) {
      newInstrs.push_back(move(instr));
    }
  }

  // Deallocate space on the stack for spilled variables
  newInstrs.push_back(make_unique<Operation>(
      "addq $" + to_string(stackSpace) + ", %rsp",
      vector<int>{},
      vector<int>{},
      optional<vector<Instruction*>>{}));
  newInstrs.push_back(make_unique<Operation>(
      "retq", vector<int>{}, vector<int>{}, optional<vector<Instruction*>>{}));

  instrs_ = move(newInstrs);
}

void Function::toCode(std::ostream& out) {
  regAlloc();
  out << name_ << ":\n";
  for (const InstrPtr& instr : instrs_) {
    instr->toCode(out, varToStackOffset_);
  }
}
