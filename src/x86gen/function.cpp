#include "src/x86gen/function.hpp"

#include "src/x86gen/flow_graph.hpp"
#include "src/x86gen/instruction.hpp"
#include "src/x86gen/interference_graph.hpp"
#include "src/x86gen/temp.hpp"

using namespace std;

Function::Function(std::string &&name, std::vector<InstrPtr> &&instrs)
    : name_(move(name)), instrs_(move(instrs)) {}

void Function::regAlloc() {
  FlowGraph fgraph(instrs_);
  fgraph.computeLiveness();
  InterferenceGraph igraph(fgraph);
  auto colorPair = igraph.color();
  const unordered_map<int, MachineReg> &coloring = colorPair.first;
  const vector<int> &spilled = colorPair.second;

  for (int tempId : spilled) {
    if (!varToStackOffset_.contains(tempId)) {
      varToStackOffset_.emplace(tempId, 8 * varToStackOffset_.size());
    }
  }

  // Allocate space on the stack for spilled variables
  size_t stackSpace = varToStackOffset_.size() * 8;
  vector<InstrPtr> newInstrs;
  newInstrs.push_back(make_unique<Operation>(
      "subq $" + to_string(stackSpace) + ", %rsp", vector<int>{}, vector<int>{}));

  // Update the instructions and add them
  // TODO: Lots of virtual function calls here, either use a switch
  // or group them together
  for (InstrPtr &instr : instrs_) {
    if (instr->getType() == InstrType::RETURN) {
      // Deallocate space on the stack for spilled variables
      newInstrs.push_back(make_unique<Operation>(
          "addq $" + to_string(stackSpace) + ", %rsp", vector<int>{},
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

void Function::toCode(std::ostream &out) {
  regAlloc();
  out << name_ << ":\n";
  for (const InstrPtr &instr : instrs_) {
    instr->toCode(out, varToStackOffset_);
  }
}

Program::Program(string &&name, vector<Function> &&fns)
    : name_(name), fns_(move(fns)) {}

void Program::toCode(ostream &out) {
  out << ".text\n.globl runprez\n.align 16\n";
  for (Function &fn : fns_) {
    fn.toCode(out);
  }
}
