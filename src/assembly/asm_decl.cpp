#include "src/assembly/assembly.hpp"
#include "src/assembly/reg_alloc/flow_graph.hpp"
#include "src/assembly/reg_alloc/interference_graph.hpp"
#include "src/assembly/temp.hpp"
#include "src/language/typecheck/context.hpp"
#include "src/language/typecheck/type.hpp"

#include <stdexcept>

#include <prez/print_stuff.hpp>

using namespace std;

namespace assem {

/***********
 * Program *
 ***********/
Program::Program(vector<DeclPtr>&& decls) : decls_(move(decls)) {}


void Program::toCode(ostream& asmFile) {
  asmFile << ".text\n.globl runprez\n.align 16\n";
  for (DeclPtr& decl : decls_) {
    decl->toCode(asmFile);
  }
}

void Program::toCodeWithTemps(ostream& logFile) {
  logFile << ".text\n.globl runprez\n.align 16\n";
  for (DeclPtr& decl : decls_) {
    decl->toCodeWithTemps(logFile);
  }
}

/********
 * Func *
 ********/
Function::Function(const string& name, vector<InstrPtr>&& instrs)
    : name_(name), instrs_(move(instrs)) {}


void Function::toCode(ostream& out) {
  FlowGraph fgraph(instrs_);
  fgraph.computeLiveness();
  InterferenceGraph igraph(fgraph);
  auto colorPair = igraph.color();

  cout << igraph << endl;
  cout << colorPair.first << endl;

  bitset<NUM_AVAIL_REGS> writtenRegs = regAlloc(colorPair.first, colorPair.second);
  auto savesAndRestores = preserveRegs(writtenRegs);
  const vector<InstrPtr>& saves = savesAndRestores.first;
  const vector<InstrPtr>& restores = savesAndRestores.second;

  out << name_ << ":\n";

  // Push all registers that need to be saved at the beginning of the function
  for (const InstrPtr& save : saves) {
    save->toCode(out, varToStackOffset_);
  }
  for (const InstrPtr& instr : instrs_) {
    // Need to pop before every return
    if (instr->getType() == InstrType::RETURN) {
      for (const InstrPtr& restore : restores) {
        restore->toCode(out, varToStackOffset_);
      }
    }
    instr->toCode(out, varToStackOffset_);
  }
}


bitset<NUM_AVAIL_REGS> Function::regAlloc(
    const unordered_map<int, MachineReg>& coloring,
    const vector<int>& spilled) {
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
  bitset<NUM_AVAIL_REGS> writtenRegs;
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
      instr->assignRegs(coloring, writtenRegs);
      if (instr->spillTemps(newInstrs)) {
        newInstrs.push_back(move(instr));
      }
    }
  }

  instrs_ = move(newInstrs);
  return writtenRegs;
}


std::pair<std::vector<InstrPtr>, std::vector<InstrPtr>> Function::preserveRegs(
    const std::bitset<NUM_AVAIL_REGS>& writtenRegs) {
  vector<MachineReg> regsToPreserve;
  vector<InstrPtr> pushInstrs;
  for (MachineReg reg : CALLEE_SAVE_REGS) {
    if (writtenRegs[reg]) {
      regsToPreserve.push_back(reg);
      // We'll add the pops to the back in this function, but just return the
      // pushes to avoid inserting at the beginning of the vector
      pushInstrs.emplace_back(new Operation("pushq `S0", { reg }, {}));
    }
  }

  // Have to pop in reverse order
  vector<InstrPtr> popInstrs;
  for (auto iter = regsToPreserve.crbegin(); iter != regsToPreserve.crend();
       ++iter) {
    popInstrs.emplace_back(new Operation("popq `D0", {}, { *iter }));
  }

  return { move(pushInstrs), move(popInstrs) };
}


void Function::toCodeWithTemps(ostream& out) {
  for (const InstrPtr& instr : instrs_) {
    cout << *instr << endl;
    instr->toCodeWithTemps(out);
  }
}


}  // namespace assem
