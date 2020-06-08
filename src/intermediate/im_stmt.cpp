#include "src/intermediate/intermediate.hpp"

#include <sstream>
#include <stdexcept>

using namespace std;

namespace im {

// TODO: Can I just use ints for labels other than actual functions???
// String copying is a lot, especially b/c of SSO

/*************
 * MakeLabel *
 *************/
MakeLabel::MakeLabel(const std::string& name) : name_(move(name)) {}

StmtPtr MakeLabel::clone() const {
  return make_unique<MakeLabel>(name_);
}

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

StmtPtr Jump::clone() const {
  return make_unique<Jump>(label_);
}

void Jump::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  instrs.emplace_back(
      new assem::JumpOp("jmp " + label_->getName(), {}, {}, label_));
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

StmtPtr CondJump::clone() const {
  return make_unique<CondJump>(e1_->clone(), e2_->clone(), rop_, ifTrue_, ifFalse_);
}

void CondJump::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  int t1 = newTemp();
  int t2 = newTemp();
  e1_->optimize()->toAssemInstrs(t1, instrs);
  e2_->optimize()->toAssemInstrs(t2, instrs);
  string op;
  switch (rop_) {
    case ROp::EQ:
      op = "je ";
      break;
    case ROp::NEQ:
      op = "jne ";
      break;
    case ROp::LT:
      op = "jl ";
      break;
    case ROp::GT:
      op = "jg ";
      break;
    case ROp::LTE:
      op = "jle ";
      break;
    case ROp::GTE:
      op = "jge ";
      break;
    default:
      throw invalid_argument("Unrecognized relative operator.");
  }

  instrs.emplace_back(new assem::Operation("cmpq `8S1, `8S0", { t1, t2 }, {}));
  instrs.emplace_back(
      new assem::CondJumpOp(op.append(ifTrue_->getName()), {}, {}, ifTrue_));
  instrs.emplace_back(
      new assem::JumpOp("jmp " + ifFalse_->getName(), {}, {}, ifFalse_));
}


/**********
 * Assign *
 **********/

namespace {
  char toInstrLetter(char bytesChar) {
    switch (bytesChar) {
      case '8':
        return 'q';
      case '4':
        return 'l';
      case '2':
        return 'w';
      case '1':
        return 'b';
      default:
        throw invalid_argument("toInstrLetter: " + to_string(bytesChar));
    }
  }
}  // namespace

Assign::Assign(ExprPtr&& e1, ExprPtr&& e2) : e1_(move(e1)), e2_(move(e2)) {}

StmtPtr Assign::clone() const {
  return make_unique<Assign>(e1_->clone(), e2_->clone());
}

void Assign::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  ExprPtr eOpt1 = e1_->optimize();
  ExprPtr eOpt2 = e2_->optimize();

  ExprType lhsType = eOpt1->getType();
  if (lhsType == ExprType::TEMP) {
    // Move e2 into the Temp of e1
    eOpt2->toAssemInstrs(static_cast<Temp*>(eOpt1.get())->getTemp(), instrs);
  } else if (lhsType == ExprType::MEM_DEREF) {
    // Move e2 into the address of e1

    // Of the form {tRhs, tAddr, tMult?}
    vector<int> srcTemps;
    srcTemps.push_back(eOpt2->toAssemInstrs(instrs));
    const MemDeref* memDeref = static_cast<MemDeref*>(eOpt1.get());
    srcTemps.push_back(memDeref->getAddr()->toAssemInstrs(instrs));
    if (const ExprPtr& mult = memDeref->getMult()) {
      srcTemps.push_back(mult->toAssemInstrs(instrs));
    }

    char bytesChar = memDeref->getNumBytes() + '0';
    string asmOp = "mov";
    asmOp.push_back(toInstrLetter(bytesChar));
    asmOp.append(" `");
    asmOp.push_back(bytesChar);
    asmOp.append("S0, ");
    asmOp.append(memDeref->genAsmCode(1));
    instrs.emplace_back(new assem::Operation(move(asmOp), move(srcTemps), {}, assem::MemRefs::DSTS));
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

StmtPtr ExprStmt::clone() const {
  return make_unique<ExprStmt>(expr_->clone());
}


void ExprStmt::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  // NOTE: This will create a wasted move to a new temp, but it will be removed
  // in flowgraph.cpp when we find undefined variables
  expr_->optimize()->toAssemInstrs(instrs);
}


/**************
 * ReturnStmt *
 **************/

ReturnStmt::ReturnStmt(ExprPtr&& retValue) : retValue_(move(retValue)) {}

StmtPtr ReturnStmt::clone() const {
  if (retValue_) {
    return make_unique<ReturnStmt>(retValue_->clone());
  }
  return make_unique<ReturnStmt>(nullptr);
}

void ReturnStmt::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  if (retValue_) {
    retValue_->optimize()->toAssemInstrs(RAX, instrs);
  }
  instrs.emplace_back(new assem::Return(retValue_ != nullptr));
}

}  // namespace im
