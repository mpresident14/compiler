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

ExprPtr Const::optimize() { return make_unique<Const>(n_); }


/*********
 * Temp *
 *********/

void Temp::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  instrs.emplace_back(new assem::Move(t_, temp));
}

int Temp::toAssemInstrs(vector<assem::InstrPtr>&) const { return t_; }

ExprPtr Temp::optimize() { return make_unique<Temp>(t_); }

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
      return handleOthers("addq", temp, instrs);
    case BOp::MINUS:
      return handleOthers("subq", temp, instrs);
    case BOp::MUL:
      return handleOthers("imulq", temp, instrs);
    case BOp::AND:
      return handleOthers("andq", temp, instrs);
    case BOp::OR:
      return handleOthers("orq", temp, instrs);
    case BOp::XOR:
      return handleOthers("xorq", temp, instrs);
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


void BinOp::handleOthers(
    std::string asmCode,
    int temp,
    std::vector<assem::InstrPtr>& instrs) const {
  int t1 = expr1_->toAssemInstrs(instrs);
  int t2 = expr2_->toAssemInstrs(instrs);
  if (t1 == temp) {
    instrs.emplace_back(new assem::Operation(
        asmCode.append(" `8S1, `8D0"), { t1, t2 }, { t1 }));
  } else if (t2 == temp) {
    if (asmCode == "subq") {
      // Only subtraction is non-commutative
      instrs.emplace_back(new assem::Operation("negq `8D0", { t2 }, { t2 }));
      instrs.emplace_back(
          new assem::Operation("addq `8S0, `8D0", { t1, t2 }, { t2 }));
    } else {
      instrs.emplace_back(new assem::Operation(
          asmCode.append(" `8S0, `8D0"), { t1, t2 }, { t2 }));
    }
  } else {
    instrs.emplace_back(new assem::Move(t1, temp));
    instrs.emplace_back(new assem::Operation(
        asmCode.append(" `8S1, `8D0"), { temp, t2 }, { temp }));
  }
}

namespace {
  /* HalfConst will have already optimized to a shift, so we'll convert that to
   * a leaq if possible */
  bool isValidLeaq(int n) { return n <= 4; }

  /* These are the operations that would benefit from being turned into a HalfConst */
  bool shouldTryHalfConst(BOp bOp) {
    switch (bOp) {
      case BOp::LSHIFT:
      case BOp::RSHIFT:
      case BOp::ARSHIFT:
      case BOp::DIV:
      case BOp::MOD:
        return false;
      case BOp::PLUS:
      case BOp::MINUS:
      case BOp::MUL:
      case BOp::AND:
      case BOp::OR:
      case BOp::XOR:
        return true;
      default:
        throw invalid_argument("Unrecognized binary operator.");
    }
  }

}  // namespace


ExprPtr BinOp::optimize() {
  ExprPtr eOpt1 = expr1_->optimize();
  ExprPtr eOpt2 = expr2_->optimize();
  bool tryHalfConst = shouldTryHalfConst(bop_);

  // Const optimization
  if (eOpt2->getType() == ExprType::CONST) {
    int n2 = static_cast<const Const*>(eOpt2.get())->getInt();
    if (eOpt1->getType() == ExprType::CONST) {
      // const OP const
      int n1 = static_cast<const Const*>(eOpt1.get())->getInt();
      return make_unique<Const>(doOp(n1, n2));
    } else if (tryHalfConst) {
      // x OP const
      return HalfConst(move(eOpt1), n2, bop_, false).optimize();
    }
  } else if (tryHalfConst && eOpt1->getType() == ExprType::CONST) {
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

  return make_unique<BinOp>(move(eOpt1), move(eOpt2), bop_);
}


/************
 * MemDeref *
 ************/

namespace {
  void addInstrLetter(u_char numBytes, string& str) {
    switch (numBytes) {
      case 8:
        return;
      case 4:
        str.append("sl");
        return;
      case 2:
        str.append("sw");
        return;
      case 1:
        str.append("sb");
        return;
      default:
        throw invalid_argument("addInstrLetter: " + to_string(numBytes));
    }
  }
}  // namespace

MemDeref::MemDeref(ExprPtr&& addr, u_char numBytes)
    : addr_(move(addr)), numBytes_(numBytes) {}

void MemDeref::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  int t = addr_->toAssemInstrs(instrs);
  string asmOp = "mov";

  addInstrLetter(numBytes_, asmOp);
  asmOp.append("q (`8S0), `8D0");
  instrs.emplace_back(new assem::Operation(move(asmOp), { t }, { temp }));
}

ExprPtr MemDeref::optimize() {
  return make_unique<MemDeref>(addr_->optimize(), numBytes_);
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

ExprPtr DoThenEval::optimize() {
  return make_unique<DoThenEval>(move(stmts_), expr_->optimize());
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

ExprPtr LabelAddr::optimize() { return make_unique<LabelAddr>(move(name_)); }

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

ExprPtr CallExpr::optimize() {
  vector<ExprPtr> optParams;
  optParams.reserve(params_.size());
  for (const ExprPtr& param : params_) {
    optParams.push_back(param->optimize());
  }
  return make_unique<CallExpr>(
      addr_->optimize(), move(optParams), hasReturnValue_);
}


/*************
 * HalfConst *
 *************/

HalfConst::HalfConst(ExprPtr&& expr, int n, BOp bOp, bool reversed)
    : expr_(move(expr)), n_(n), bOp_(bOp), reversed_(reversed) {}

void HalfConst::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  string asmCode;
  switch (bOp_) {
    case BOp::LSHIFT:
      // Guaranteed to be of the form x << const, not const << x
      // Otherwise handled by BinOp (see BinOp::optimize)
      asmCode = "shlq";
    case BOp::PLUS:
      asmCode = "addq";
    case BOp::MINUS:
      asmCode = "subq";
    case BOp::MUL:
      asmCode = "imulq";
    case BOp::AND:
      asmCode = "andq";
    case BOp::OR:
      asmCode = "orq";
    case BOp::XOR:
      asmCode = "xorq";
    default:
      throw invalid_argument("HalfConst::toAssemInstrs");
  }

  if (reversed_ && bOp_ == BOp::MINUS) {
    int t = expr_->toAssemInstrs(instrs);
    instrs.emplace_back(new assem::Operation(
          string("movq $").append(to_string(n_)).append(", `8D0"), {}, {temp}));
    instrs.emplace_back(new assem::Operation(
        asmCode.append(" `8S0, `8D0"), {t, temp}, {temp}));

  } else {
    expr_->toAssemInstrs(temp, instrs);
    instrs.emplace_back(new assem::Operation(
        asmCode.append(" $").append(to_string(n_)).append(", `8D0"), {temp}, {temp}));
  }
}

namespace {
  /* If the nonzero number has 2 or fewer set bits, returns their positions.
   * Otherwise returns an empty vector */
  vector<size_t> getSetBits(int n) {
    vector<size_t> setBits;
    size_t numShifts = 0;
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
        return make_unique<HalfConst>(move(eOpt), n_, bOp_, reversed_);
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
        return make_unique<HalfConst>(move(eOpt), n_, bOp_, reversed_);
    }
  }

  // Multiplication as shifts, e.g. 8x = x << 3 or 36x = (x << 5) + (x << 2)
  if (bOp_ == BOp::MUL) {
    vector<size_t> setBits = getSetBits(n_);
    if (setBits.size() == 1) {
      return make_unique<BinOp>(
          move(eOpt), make_unique<Const>(setBits[0]), BOp::LSHIFT);
    } else if (setBits.size() == 2) {
      vector<StmtPtr> stmts;
      int t = newTemp();
      stmts.emplace_back(make_unique<Assign>(make_unique<Temp>(t), move(eOpt)));
      return make_unique<DoThenEval>(
          move(stmts),
          make_unique<BinOp>(
              make_unique<BinOp>(
                  make_unique<Temp>(t),
                  make_unique<Const>(setBits[0]),
                  BOp::LSHIFT),
              make_unique<BinOp>(
                  make_unique<Temp>(t),
                  make_unique<Const>(setBits[1]),
                  BOp::LSHIFT),
              BOp::PLUS));
    }
  }

  return make_unique<HalfConst>(move(eOpt), n_, bOp_, reversed_);
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
