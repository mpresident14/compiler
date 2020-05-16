#include "src/intermediate/intermediate.hpp"

using namespace std;
using namespace im;

// TODO: Can I just use ints for labels other than actual functions???
// String copying is a lot, especially b/c of SSO

/*********
 * Block *
 *********/
Block::Block(std::vector<StmtPtr>&& stmts) : stmts_(move(stmts)) {}

void Block::toInstrs(std::vector<InstrPtr>& instrs) {
  for (const StmtPtr& stmt : stmts_) {
    stmt->toInstrs(instrs);
  }
}

/*************
 * MakeLabel *
 *************/
MakeLabel::MakeLabel(const std::string& name) : name_(move(name)) {}

void MakeLabel::toInstrs(std::vector<InstrPtr>& instrs) {
  if (instr_) {
    // Already generated the instruction
    instrs.push_back(move(instr_));
  } else {
    instrs.emplace_back(new Label(name_));
  }
}

Label* MakeLabel::genInstr() {
  // Only generate label once
  if (!instr_) {
    instr_ = make_unique<Label>(name_);
  }
  return instr_.get();
}


/********
 * Jump *
 ********/
Jump::Jump(Label* label) : label_(label) {}

void Jump::toInstrs(std::vector<InstrPtr>& instrs) {
  instrs.emplace_back(
      new JumpOp("jmp " + label_->getName(), {}, {}, { label_ }));
}


/************
 * CondJump *
 ************/
CondJump::CondJump(ExprPtr&& e1, ExprPtr&& e2, ROp rop, Label* ifTrue,  Label* ifFalse)
    : e1_(move(e1)), e2_(move(e2)), rop_(rop), ifTrue_(ifTrue), ifFalse_(ifFalse) {}

void CondJump::toInstrs(std::vector<InstrPtr>& instrs) {
  int t1 = newTemp();
  int t2 = newTemp();
  e1_->toInstrs(t1, instrs);
  e2_->toInstrs(t2, instrs);
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

  instrs.emplace_back(new Operation("cmpq `S1, `S0", { t1, t2 }, {}));
  instrs.emplace_back(
      new CondJumpOp(op.append(ifTrue_->getName()), {}, {}, { ifTrue_ }));
  instrs.emplace_back(
      new JumpOp("jmp " + ifFalse_->getName(), {}, {}, { ifFalse_ }));
}


/**********
 * Assign *
 **********/

Assign::Assign(ExprPtr&& e1, ExprPtr&& e2) : e1_(move(e1)), e2_(move(e2)) {}

void Assign::toInstrs(std::vector<InstrPtr>& instrs) {
  int t2 = newTemp();
  e2_->toInstrs(t2, instrs);
  ExprType lhsType = e1_->getType();
  if (lhsType == ExprType::TEMP) {
    // Move e2 into the Temp of e1
    e2_->toInstrs(static_cast<Temp*>(e1_.get())->getTemp(), instrs);
  } else if (lhsType == ExprType::MEM_DEREF) {
    // Move e2 into the address of e1
    int t1 = newTemp();
    static_cast<MemDeref*>(e1_.get())->getAddr()->toInstrs(t1, instrs);
    instrs.emplace_back(new Operation("movq `S1, (`S0)", { t1, t2 }, {}));
  } else {
    throw invalid_argument("Invalid ExprType for Assign LHS.");
  }
}


/************
 * CallStmt *
 ************/

CallStmt::CallStmt(ExprPtr&& addr, std::vector<ExprPtr>&& params)
    : addr_(move(addr)), params_(move(params)) {}

void CallStmt::toInstrs(std::vector<InstrPtr>& instrs) {
  CallExpr(move(addr_), move(params_), false).toInstrs(0, instrs);
}


/**************
 * ReturnStmt *
 **************/

ReturnStmt::ReturnStmt(ExprPtr&& retValue) : retValue_(move(retValue)) {}

void ReturnStmt::toInstrs(std::vector<InstrPtr>& instrs) {
  if (retValue_) {
    retValue_->toInstrs(RAX, instrs);
  }
  instrs.emplace_back(new Return(retValue_ != nullptr));
}
