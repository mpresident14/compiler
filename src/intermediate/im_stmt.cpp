#include "src/intermediate/intermediate.hpp"

#include <sstream>
#include <stdexcept>

#include <prez/print_stuff.hpp>

using namespace std;

namespace im {


/*************
 * MakeLabel *
 *************/
MakeLabel::MakeLabel(const std::string& name) : name_(move(name)) {}

void MakeLabel::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  if (instr_) {
    // Already generated the instruction
    instrs.push_back(move(instr_));
  } else {
    instrs.emplace_back(new assem::Label(name_));
  }
}

assem::Label* MakeLabel::genInstr() {
  // Only generate label once
  if (!instr_) {
    instr_ = make_unique<assem::Label>(name_);
  }
  return instr_.get();
}


/********
 * Jump *
 ********/
Jump::Jump(assem::Label* label) : label_(label) {}

void Jump::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  instrs.emplace_back(
      new assem::JumpOp("jmp " + label_->name_, {}, {}, label_));
}


/************
 * CondJump *
 ************/
CondJump::CondJump(
    ExprPtr&& e1,
    ExprPtr&& e2,
    ROp rop,
    assem::Label* ifTrue,
    assem::Label* ifFalse)
    : e1_(move(e1)),
      e2_(move(e2)),
      rop_(rop),
      ifTrue_(ifTrue),
      ifFalse_(ifFalse) {}

namespace {
  bool (*getRopFn(ROp rOp))(long, long) {
    switch (rOp) {
      case ROp::EQ:
        return [](long a, long b) { return a == b; };
      case ROp::NEQ:
        return [](long a, long b) { return a != b; };
      case ROp::LT:
        return [](long a, long b) { return a < b; };
      case ROp::GT:
        return [](long a, long b) { return a > b; };
      case ROp::LTE:
        return [](long a, long b) { return a <= b; };
      case ROp::GTE:
        return [](long a, long b) { return a >= b; };
      default:
        throw invalid_argument("Unrecognized relative operator.");
    }
  }

  string getRopAsm(ROp rOp, bool needsFlip) {
    switch (rOp) {
      case ROp::EQ:
        return needsFlip ? "jne " : "je ";
      case ROp::NEQ:
        return needsFlip ? "je " : "jne ";
      case ROp::LT:
        return needsFlip ? "jge " : "jl ";
      case ROp::GT:
        return needsFlip ? "jle " : "jg ";
      case ROp::LTE:
        return needsFlip ? "jg " : "jle ";
      case ROp::GTE:
        return needsFlip ? "jl " : "jge ";
      default:
        throw invalid_argument("Unrecognized relative operator.");
    }
  }
}  // namespace


void CondJump::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  ExprPtr eOpt1 = e1_->optimize();
  ExprPtr eOpt2 = e2_->optimize();
  string asmChunk1 = eOpt1->asmChunk(8, true, 0);
  string asmChunk2 = eOpt2->asmChunk(8, true, 0);
  vector<int> srcTemps;

  bool needsFlip = false;
  if (isConstChunk(asmChunk1)) {
    if (isConstChunk(asmChunk2)) {
      // Both constants, just evaluate at compile-time
      bool (*opFn)(long, long) = getRopFn(rop_);
      // TODO: Log warning about always true/false
      bool res = opFn(
          static_cast<const Const*>(eOpt1.get())->n_,
          static_cast<const Const*>(eOpt2.get())->n_);
      if (res) {
        instrs.emplace_back(
            new assem::JumpOp("jmp " + ifTrue_->name_, {}, {}, ifTrue_));
      } else {
        instrs.emplace_back(
            new assem::JumpOp("jmp " + ifFalse_->name_, {}, {}, ifFalse_));
      }
      return;
    } else {
      // "cmp reg, imm" is not legal, so we need to do "cmp imm, reg" and flip
      // the operator
      needsFlip = true;
      srcTemps.push_back(eOpt2->toAssemInstrs(instrs));
    }
  } else {
    if (!isConstChunk(asmChunk2)) {
      // asmChunk in the form "`<#><S|D><index>"
      asmChunk1[3] = '1';
      srcTemps.push_back(eOpt2->toAssemInstrs(instrs));
    }
    srcTemps.push_back(eOpt1->toAssemInstrs(instrs));
  }

  ostringstream cmpAsm;
  if (needsFlip) {
    cmpAsm << "cmpq " << asmChunk1 << ", " << asmChunk2;
  } else {
    cmpAsm << "cmpq " << asmChunk2 << ", " << asmChunk1;
  }
  instrs.emplace_back(new assem::Operation(cmpAsm.str(), move(srcTemps), {}));
  instrs.emplace_back(new assem::CondJumpOp(
      getRopAsm(rop_, needsFlip).append(ifTrue_->name_), {}, {}, ifTrue_));
  instrs.emplace_back(
      new assem::JumpOp("jmp " + ifFalse_->name_, {}, {}, ifFalse_));
}


/**********
 * Assign *
 **********/

char movSuffix(size_t numBytes) {
  switch (numBytes) {
    case 8:
      return 'q';
    case 4:
      return 'l';
    case 2:
      return 'w';
    case 1:
      return 'b';
    default:
      throw invalid_argument("movSuffix: " + to_string(numBytes));
  }
}

Assign::Assign(ExprPtr&& e1, ExprPtr&& e2) : e1_(move(e1)), e2_(move(e2)) {}

void Assign::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  ExprPtr eOpt1 = e1_->optimize();
  ExprPtr eOpt2 = e2_->optimize();

  ExprType lhsType = eOpt1->getType();
  if (lhsType == ExprType::TEMP) {
    // Move e2 into the Temp of e1
    eOpt2->toAssemInstrs(static_cast<Temp*>(eOpt1.get())->t_, instrs);
  } else if (lhsType == ExprType::MEM_DEREF) {
    // Move e2 into the address of e1

    // Of the form {tRhs?, tAddr, tMult?}
    vector<int> srcTemps;
    size_t tempIndex = 0;
    const MemDeref* memDeref = static_cast<MemDeref*>(eOpt1.get());
    size_t numBytes = memDeref->numBytes_;

    string asmChunk2 = eOpt2->asmChunk(numBytes, true, 0);
    if (!isConstChunk(asmChunk2)) {
      ++tempIndex;
      srcTemps.push_back(eOpt2->toAssemInstrs(instrs));
    }

    srcTemps.push_back(memDeref->baseAddr_->toAssemInstrs(instrs));
    if (memDeref->mult_) {
      srcTemps.push_back(memDeref->mult_->toAssemInstrs(instrs));
    }

    ostringstream asmOp;
    asmOp << "mov" << movSuffix(numBytes) << ' ' << asmChunk2 << ", "
          << memDeref->genAsmCode(tempIndex);
    instrs.emplace_back(
        new assem::Operation(asmOp.str(), move(srcTemps), {}, true));
  } else {
    ostringstream err;
    err << "Invalid ExprType for Assign LHS: " << lhsType;
    throw invalid_argument(err.str());
  }
}


/************
 * ExprStmt *
 ************/

ExprStmt::ExprStmt(ExprPtr&& expr) : expr_(move(expr)) {}


void ExprStmt::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  expr_->optimize()->toAssemInstrs(instrs);
}


/**************
 * ReturnStmt *
 **************/

ReturnStmt::ReturnStmt(ExprPtr&& retValue) : retValue_(move(retValue)) {}

void ReturnStmt::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  if (retValue_) {
    retValue_->optimize()->toAssemInstrs(RAX, instrs);
  }
  instrs.emplace_back(new assem::Return(retValue_ != nullptr));
}

}  // namespace im
