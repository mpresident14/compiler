#include "src/intermediate/intermediate.hpp"

using namespace std;
using namespace intermediate;

// TODO: Can I just use ints for labels other than actual functions???


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
  instr_ = make_unique<Label>(name_);
  return instr_.get();
}


/********
 * Jump *
 ********/
Jump::Jump(Label* label) : label_(label) {}

void Jump::toInstrs(std::vector<InstrPtr>& instrs) {
  instrs.emplace_back(new JumpOp("jmp " + label_->getName(), {}, {}, {label_}, false));
}


/************
 * CondJump *
 ************/
CondJump::CondJump(ExprPtr e1, ExprPtr e2, Rop rop, Label* ifTrue)
    : e1_(move(e1)), e2_(move(e2)), rop_(rop), ifTrue_(ifTrue) {}

void CondJump::toInstrs(std::vector<InstrPtr>& instrs) {
  int t1 = newTemp();
  int t2 = newTemp();
  e1_->toInstrs(t1, instrs);
  e2_->toInstrs(t2, instrs);
  string op;
  switch (rop_) {
    case Rop::EQ:
      op = "je ";
      break;
    case Rop::NEQ:
      op = "jne ";
      break;
    case Rop::LESS:
      op = "jl ";
      break;
    case Rop::GREATER:
      op = "jg ";
      break;
    case Rop::LESSEQ:
      op = "jle ";
      break;
    case Rop::GREATEREQ:
      op = "jge ";
      break;
    default: throw invalid_argument("Unrecognized relative operator.");
  }

  instrs.emplace_back(new Operation("cmpq `S1, `S0", {t1, t2}, {}));
  instrs.emplace_back(new JumpOp(op.append(ifTrue_->getName()), {}, {}, {ifTrue_}, true));
}