#include "src/intermediate/intermediate.hpp"

#include <functional>
#include <stdexcept>

using namespace std;

namespace im {


/*********
 * Const *
 *********/

void Const::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  instrs.emplace_back(new assem::Operation(
      string("movq $").append(to_string(n_)).append(", `8D0"), {}, { temp }));
}


/*********
 * Temp *
 *********/

void Temp::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  instrs.emplace_back(new assem::Move(t_, temp));
}

int Temp::toAssemInstrs(vector<assem::InstrPtr>&) const { return t_; }


/*********
 * BinOp *
 *********/

BinOp::BinOp(ExprPtr&& expr1, ExprPtr&& expr2, BOp bop)
    : expr1_(move(expr1)), expr2_(move(expr2)), bop_(bop) {}

void BinOp::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  switch (bop_) {
    case BOp::LSHIFT:
      return handleShifts("shlq", temp, instrs);
    case BOp::RSHIFT:
      return handleShifts("shrq", temp, instrs);
    case BOp::ARSHIFT:
      return handleShifts("sarq", temp, instrs);
    case BOp::DIV:
      return handleDiv(true, temp, instrs);
    case BOp::MOD:
      return handleDiv(false, temp, instrs);
    case BOp::PLUS:
      return handleOthers("addq", temp, instrs, plus<int>());
    case BOp::MINUS:
      return handleOthers("subq", temp, instrs, minus<int>());
    case BOp::MUL:
      return handleOthers("imulq", temp, instrs, multiplies<int>());
    case BOp::AND:
      return handleOthers("andq", temp, instrs, bit_and<int>());
    case BOp::OR:
      return handleOthers("orq", temp, instrs, bit_or<int>());
    case BOp::XOR:
      return handleOthers("xorq", temp, instrs, bit_xor<int>());
    default:
      throw invalid_argument("Unrecognized binary operator.");
  }
}

void BinOp::handleShifts(
    string asmCode,
    int temp,
    vector<assem::InstrPtr>& instrs) const {
  expr1_->toAssemInstrs(temp, instrs);

  // If shift number (expr2_) is not an immediate, its value must be in %cl
  if (expr2_->getType() == ExprType::CONST) {
    asmCode.append(" $");
    asmCode.append(to_string(static_cast<Const*>(expr2_.get())->getInt()));
    asmCode.append(", `8D0");
    instrs.emplace_back(new assem::Operation(asmCode, { temp }, { temp }));
  } else {
    expr2_->toAssemInstrs(RCX, instrs);
    asmCode.append(" %cl, `8D0");
    instrs.emplace_back(new assem::Operation(asmCode, { RCX, temp }, { temp }));
  }
}

void BinOp::handleDiv(bool isDiv, int temp, vector<assem::InstrPtr>& instrs)
    const {
  expr1_->toAssemInstrs(RAX, instrs);
  int t2 = expr2_->toAssemInstrs(instrs);
  // Sign-extend %rax into %rdx
  instrs.emplace_back(new assem::Operation("cqto", { RAX }, { RDX }));
  // Divide %rax%rdx by t2, quotient in %rax, remainder in %rdx
  instrs.emplace_back(
      new assem::Operation("idivq `8S0", { t2, RAX, RDX }, { RAX, RDX }));
  if (isDiv) {
    // If division, move %rax into temp
    Temp(RAX).toAssemInstrs(temp, instrs);
  } else {
    // If mod, move %rdx into temp
    Temp(RDX).toAssemInstrs(temp, instrs);
  }
}

namespace {
  /* HalfConst will have already optimized to a shift, so we'll convert that to
   * a leaq if possible */
  bool isValidLeaq(int n) { return n <= 4; }
}  // namespace


ExprPtr BinOp::optimize() {
  ExprPtr eOpt1 = expr1_->optimize();
  ExprPtr eOpt2 = expr2_->optimize();

  // Const optimization
  if (eOpt2->getType() == ExprType::CONST) {
    int n2 = static_cast<const Const*>(eOpt2.get())->getInt();
    if (eOpt1->getType() == ExprType::CONST) {
      // const OP const
      int n1 = static_cast<const Const*>(eOpt1.get())->getInt();
      return make_unique<Const>(doOp(n1, n2));
    } else {
      // x OP const
      return HalfConst(move(eOpt1), n2, bop_, false).optimize();
    }
  } else if (eOpt1->getType() == ExprType::CONST) {
    // const OP x
    int n1 = static_cast<const Const*>(eOpt1.get())->getInt();
    return HalfConst(move(eOpt2), n1, bop_, true).optimize();
  }


  // Leaq optimization: x + y*k, where k is 1,2,4, or 8
  // Note that if k is one of these, HalfConst will already be optimized to
  // a shift, so we act accordingly
  if (bop_ == BOp::PLUS) {
    HalfConst* halfConst = dynamic_cast<HalfConst*>(eOpt1.get());
    if (halfConst && !halfConst->reversed_ && halfConst->bOp_ == BOp::LSHIFT &&
        isValidLeaq(halfConst->n_)) {
      return make_unique<Leaq>(
          move(eOpt2), move(halfConst->expr_), halfConst->n_);
    }

    halfConst = dynamic_cast<HalfConst*>(eOpt2.get());
    if (halfConst && !halfConst->reversed_ && halfConst->bOp_ == BOp::LSHIFT &&
        isValidLeaq(halfConst->n_)) {
      return make_unique<Leaq>(
          move(eOpt1), move(halfConst->expr_), halfConst->n_);
    }
  }

  return make_unique<BinOp>(move(eOpt1), move(eOpt2));
}


/************
 * MemDeref *
 ************/

namespace {
  void addInstrLetter(char bytesChar, string& str) {
    switch (bytesChar) {
      case '8':
        return;
      case '4':
        str.append("sl");
        return;
      case '2':
        str.append("sw");
        return;
      case '1':
        str.append("sb");
        return;
      default:
        throw invalid_argument("addInstrLetter: " + to_string(bytesChar));
    }
  }
}  // namespace

MemDeref::MemDeref(ExprPtr&& addr, u_char numBytes)
    : addr_(move(addr)), numBytes_(numBytes) {}

void MemDeref::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  int t = addr_->toAssemInstrs(instrs);
  string asmOp = "mov";

  char bytesChar = numBytes_ + '0';
  addInstrLetter(bytesChar, asmOp);
  asmOp.append("q (`8S0), `8D0");
  instrs.emplace_back(new assem::Operation(move(asmOp), { t }, { temp }));
}

/**************
 * DoThenEval *
 **************/

DoThenEval::DoThenEval(vector<StmtPtr>&& stmts, ExprPtr expr)
    : stmts_(move(stmts)), expr_(move(expr)) {}

void DoThenEval::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs)
    const {
  for (const StmtPtr& stmt : stmts_) {
    stmt->toAssemInstrs(instrs);
  }
  expr_->toAssemInstrs(temp, instrs);
}

/*************
 * LabelAddr *
 *************/

LabelAddr::LabelAddr(const string& name) : name_(name) {}

void LabelAddr::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  // "leaq symbol(%rip), dst" looks like symbol + %rip, but actually means
  // symbol with respect to %rip. Essentially, it calculates the address of
  // symbol
  instrs.emplace_back(new assem::Operation(
      string("leaq ").append(name_).append("(%rip), `8D0"), {}, { temp }));
}


/************
 * CallExpr *
 ************/
CallExpr::CallExpr(
    ExprPtr&& addr,
    vector<ExprPtr>&& params,
    bool hasReturnValue)
    : addr_(move(addr)),
      params_(move(params)),
      hasReturnValue_(hasReturnValue) {}


void CallExpr::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  // Move params into argument registers
  // TODO: If more than six params, need to put onto stack
  vector<int> srcTemps;
  size_t numParams = params_.size();
  if (numParams > 6) {
    throw runtime_error("TODO: More than 6 args not implemented yet");
  }
  for (size_t i = 0; i < numParams; ++i) {
    int argReg = ARG_REGS[i];
    params_[i]->toAssemInstrs(argReg, instrs);
    srcTemps.push_back(argReg);
  }

  if (addr_->getType() == ExprType::LABEL_ADDR) {
    // If we are calling a function name, just call it directly
    instrs.emplace_back(new assem::Operation(
        "callq " + static_cast<LabelAddr*>(addr_.get())->getName(),
        move(srcTemps),
        regsAsInts(CALLER_SAVE_REGS)));
  } else {
    // If we are calling an address, we need to put it in a register
    int t = addr_->toAssemInstrs(instrs);
    srcTemps.push_back(t);
    instrs.emplace_back(new assem::Operation(
        "callq *`8S0", move(srcTemps), regsAsInts(CALLER_SAVE_REGS)));
  }

  // Move result from %rax to temp if needed
  if (hasReturnValue_) {
    instrs.emplace_back(new assem::Move(RAX, temp));
  }
}

namespace {
  /* If the nonzero number has 2 or fewer set bits, returns their positions.
   * Otherwise returns an empty vector */
  vector<size_t> getSetBits(int n) {
    vector<size_t> setBits;
    size_t numShifts;
    while (n != 0) {
      if (n & 1) {
        if (setBits.size() == 2) {
          return {};
        }
        setBits.push_back(numShifts);
      }
      n >>= 1;
      ++numShifts;
    }
    return setBits;
  }
}  // namespace

/*************
 * HalfConst *
 *************/

ExprPtr HalfConst::optimize() {
  ExprPtr eOpt = expr_->optimize();

  // Const optimization
  if (n_ == 0) {
    switch (bOp_) {
      case BOp::PLUS:
      case BOp::MINUS:
      case BOp::OR:
      case BOp::LSHIFT:
      case BOp::RSHIFT:
      case BOp::ARSHIFT:
        return eOpt;
      case BOp::MUL:
      case BOp::AND:
        return make_unique<Const>(0);
      default:
        return make_unique<HalfConst>(move(eOpt), n_, bOp_);
    }
  }

  if (n_ == 1) {
    switch (bOp_) {
      case BOp::PLUS:
        return make_unique<IncDec>(move(eOpt), true);
      case BOp::MINUS:
        return make_unique<IncDec>(move(eOpt), false);
      case BOp::MUL:
        return eOpt;
      case BOp::MOD:
        return make_unique<Const>(0);
      default:
        return make_unique<HalfConst>(move(eOpt), n_, bOp_);
    }
  }

  // Multiplication as shifts, e.g. 8x = x << 3 or 36x = (x << 5) + (x << 2)
  if (bOp_ == BOp::MUL) {
    vector<size_t> setBits = getSetBits(n_);
    if (setBits.size() == 1) {
      return make_unique<BinOp>(move(eOpt), setBits[0], BOp::LSHIFT);
    } else if (setBits.size() == 2) {
      vector<StmtPtr> stmts;
      int t = newTemp();
      stmts.emplace_back(make_unique<Assign>(new Temp(t), move(eOpt)));
      return make_unique<DoThenEval>(
          move(stmts),
          make_unique<BinOp>(
              make_unique<BinOp>(new Temp(t), setBits[0], BOp::LSHIFT),
              make_unique<BinOp>(new Temp(t), setBits[1], BOp::LSHIFT),
              BOp::PLUS));
    }
  }

  return make_unique<HalfConst>(move(eOpt), n_, bOp_);
}

/********
 * Leaq *
 ********/

ExprPtr Leaq::optimize() {
  return make_unique<Leaq>(e1_->optimize(), e2_->optimize(), n_);
}

/**********
 * IncDec *
 **********/
ExprPtr IncDec::optimize() {
  return make_unique<IncDec>(expr_->optimize(), inc_);
}


/********
 * Expr *
 ********/
int Expr::toAssemInstrs(vector<assem::InstrPtr>& instrs) const {
  int temp = newTemp();
  toAssemInstrs(temp, instrs);
  return temp;
}

}  // namespace im
