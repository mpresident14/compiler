#include "src/intermediate/intermediate.hpp"

using namespace std;

namespace im {

// TODO: Can I just use ints for labels other than actual functions???
// String copying is a lot, especially b/c of SSO

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

void CondJump::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  int t1 = newTemp();
  int t2 = newTemp();
  e1_->toAssemInstrs(t1, instrs);
  e2_->toAssemInstrs(t2, instrs);
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
  // switch (rop_) {
  //   case ROp::EQ:
  //     op = "jne ";
  //     break;
  //   case ROp::NEQ:
  //     op = "je ";
  //     break;
  //   case ROp::LT:
  //     op = "jge ";
  //     break;
  //   case ROp::GT:
  //     op = "jle ";
  //     break;
  //   case ROp::LTE:
  //     op = "jg ";
  //     break;
  //   case ROp::GTE:
  //     op = "jl ";
  //     break;
  //   default:
  //     throw invalid_argument("Unrecognized relative operator.");
  // }

  instrs.emplace_back(new assem::Operation("cmpq `S1, `S0", { t1, t2 }, {}));
  instrs.emplace_back(
      new assem::CondJumpOp(op.append(ifTrue_->getName()), {}, {}, ifTrue_));
  instrs.emplace_back(
      new assem::JumpOp("jmp " + ifFalse_->getName(), {}, {}, ifFalse_));
  // instrs.emplace_back(
  //     new assem::CondJumpOp(op.append(ifFalse_->getName()), {}, {}, { ifFalse_ }));
  // instrs.emplace_back(
  //     new assem::JumpOp("jmp " + ifTrue_->getName(), {}, {}, { ifTrue_ }));
}


/**********
 * Assign *
 **********/

Assign::Assign(ExprPtr&& e1, ExprPtr&& e2) : e1_(move(e1)), e2_(move(e2)) {}

void Assign::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  ExprType lhsType = e1_->getType();
  if (lhsType == ExprType::TEMP) {
    // Move e2 into the Temp of e1
    e2_->toAssemInstrs(static_cast<Temp*>(e1_.get())->getTemp(), instrs);
  } else if (lhsType == ExprType::MEM_DEREF) {
    // Move e2 into the address of e1
    int t1 = newTemp();
    int t2 = newTemp();
    e2_->toAssemInstrs(t2, instrs);
    static_cast<MemDeref*>(e1_.get())->getAddr()->toAssemInstrs(t1, instrs);
    instrs.emplace_back(new assem::Operation("movq `S1, (`S0)", { t1, t2 }, {}));
  } else {
    throw invalid_argument("Invalid ExprType for Assign LHS.");
  }
}


/************
 * CallStmt *
 ************/

CallStmt::CallStmt(ExprPtr&& addr, std::vector<ExprPtr>&& params)
    : addr_(move(addr)), params_(move(params)) {}

void CallStmt::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  CallExpr(move(addr_), move(params_), false).toAssemInstrs(0, instrs);
}


/**************
 * ReturnStmt *
 **************/

ReturnStmt::ReturnStmt(ExprPtr&& retValue) : retValue_(move(retValue)) {}

void ReturnStmt::toAssemInstrs(std::vector<assem::InstrPtr>& instrs) {
  if (retValue_) {
    retValue_->toAssemInstrs(RAX, instrs);
  }
  instrs.emplace_back(new assem::Return(retValue_ != nullptr));
}

}
