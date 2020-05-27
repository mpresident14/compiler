#include "src/assembly/assembly.hpp"
#include "src/assembly/reg_alloc/flow_graph.hpp"
#include "src/assembly/reg_alloc/interference_graph.hpp"
#include "src/assembly/temp.hpp"

#include <fstream>
#include <stdexcept>

using namespace std;

namespace assem {

/***********
 * Program *
 ***********/
Program::Program(std::vector<DeclPtr>&& decls) : decls_(move(decls)) {}


void Program::toCode(const string& fileName) {
  ofstream asmFile(fileName);
  if (!asmFile.is_open()) {
    throw invalid_argument("Could not open file " + fileName);
  }

  asmFile << ".text\n.globl runprez\n.align 16\n";
  for (DeclPtr& decl : decls_) {
    decl->toCode(asmFile);
  }
}


/********
 * Func *
 ********/
Function::Function(const std::string& name, std::vector<InstrPtr>&& instrs)
    : name_(name), instrs_(move(instrs)) {}


void Function::toCode(std::ostream& out) {
  regAlloc();
  out << name_ << ":\n";
  for (const InstrPtr& instr : instrs_) {
    instr->toCode(out, varToStackOffset_);
  }
}


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
  for (auto iter = instrs_.begin(); iter != instrs_.end(); ++iter) {
    InstrPtr& instr = *iter;
    InstrType type = instr->getType();
    if (type == InstrType::JUMP_OP &&
        static_cast<JumpOp*>(instr.get())->getJump() == next(iter)->get()) {
      // Skip jumps immediately followed by the label to which they jump
      // (Nothing to do)
    } else if (type == InstrType::RETURN) {
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


}  // namespace assem
