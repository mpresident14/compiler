#include "src/assembly/assembly.hpp"
#include "src/assembly/reg_alloc/flow_graph.hpp"
#include "src/assembly/reg_alloc/interference_graph.hpp"
#include "src/assembly/temp.hpp"

#include <stdexcept>

#include <prez/print_stuff.hpp>

using namespace std;

namespace assem {

/***********
 * Program *
 ***********/
SrcFile::SrcFile(vector<DeclPtr>&& decls) : decls_(move(decls)) {}


void SrcFile::toCode(ostream& asmFile) {
  asmFile << ".text\n.align 16\n";
  for (DeclPtr& decl : decls_) {
    decl->toCode(asmFile);
  }
}

/********
 * Func *
 ********/
Function::Function(string_view name, vector<InstrPtr>&& instrs)
    : name_(name), instrs_(move(instrs)) {}


void Function::toCode(ostream& out) {
  out << name_ << ":\n";
  // Save the frame pointer
  out << "\tpushq %rbp\n"
         "\tmovq %rsp, %rbp\n";

  FlowGraph fgraph(instrs_);
  fgraph.computeLiveness();
  InterferenceGraph igraph(fgraph);
  auto colorPair = igraph.color();

#ifdef DEBUG
  if (name_ == "_main0") {
    cout << fgraph << endl;
    cout << igraph << endl;
  }
  for (const InstrPtr& instr : instrs_) {
    instr->toCode(out, {});
  }
  for (const auto& [temp, reg] : colorPair.first) {
    if (!isRegister(temp)) {
      out << "# %t" << -temp << " = " << regToString(reg, 8) << '\n';
    }
  }
  out << "\n# SPILLED\n" << endl;
  for (const auto& temp : colorPair.second) {
    out << "# %t" << -temp << '\n';
  }
  return;
#endif

  bitset<NUM_AVAIL_REGS> writtenRegs = regAlloc(colorPair.first, colorPair.second);
  auto savesAndRestores = preserveRegs(writtenRegs);

  // Push all registers that need to be saved at the beginning of the function
  for (const InstrPtr& save : savesAndRestores.first) {
    save->toCode(out, varToStackOffset_);
  }

  for (const InstrPtr& instr : instrs_) {
    // Need to pop before every return
    if (dynamic_cast<const Return*>(instr.get())) {
      for (const InstrPtr& restore : savesAndRestores.second) {
        restore->toCode(out, varToStackOffset_);
      }
      // Restore the frame pointer
      out << "\tmovq %rbp, %rsp\n"
             "\tpopq %rbp\n";
    }
    instr->toCode(out, varToStackOffset_);
  }
}


bitset<NUM_AVAIL_REGS> Function::regAlloc(
    const unordered_map<int, MachineReg>& coloring,
    const vector<int>& spilled) {
  // Assign each spilled temporary a spot on the stack
  for (int tempId : spilled) {
    if (!varToStackOffset_.contains(tempId)) {
      varToStackOffset_.emplace(tempId, 8 * varToStackOffset_.size());
    }
  }

  // Allocate space on the stack for spilled variables
  size_t stackSpace = varToStackOffset_.size() * 8;
  vector<InstrPtr> newInstrs;

  bool needStackSpace;
  if ((needStackSpace = stackSpace != 0)) {
    newInstrs.emplace_back(new Operation("subq $" + to_string(stackSpace) + ", %rsp", {}, {}));
  }

  // Update the instructions and add them
  bitset<NUM_AVAIL_REGS> writtenRegs;
  for (auto iter = instrs_.begin(); iter != instrs_.end(); ++iter) {
    InstrPtr& instr = *iter;
    // Skip jumps immediately followed by the label to which they are jumping
    const JumpOp* jumpOp = dynamic_cast<const JumpOp*>(instr.get());
    if (jumpOp && jumpOp->jump_ == next(iter)->get()) {
      continue;
    }

    instr->assignRegs(coloring, writtenRegs);
    if (instr->spillTemps(newInstrs)) {
      newInstrs.push_back(move(instr));
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
      pushInstrs.emplace_back(new Operation("pushq `8S0", { reg }, {}));
    }
  }

  // Have to pop in reverse order
  vector<InstrPtr> popInstrs;
  for (auto iter = regsToPreserve.crbegin(); iter != regsToPreserve.crend(); ++iter) {
    popInstrs.emplace_back(new Operation("popq `8D0", {}, { *iter }));
  }

  return { move(pushInstrs), move(popInstrs) };
}


/********
 * VTable *
 ********/

VTable::VTable(string_view name, vector<std::string>&& fnNames)
    : label_(name), fnNames_(move(fnNames)) {}


void VTable::toCode(std::ostream& out) {
  out << label_ << ":\n";
  for (const string& str : fnNames_) {
    out << "\t.quad " << str << '\n';
  }
}

}  // namespace assem
