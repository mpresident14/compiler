#include "src/intermediate/intermediate.hpp"

#include <functional>
#include <stdexcept>

using namespace std;

namespace im {


/*********
 * Const *
 *********/

ExprPtr Const::clone() const {
  return make_unique<Const>(n_);
}

void Const::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  instrs.emplace_back(new assem::Operation(
      string("movq $").append(to_string(n_)).append(", `8D0"), {}, { temp }));
}

ExprPtr Const::optimize() { return make_unique<Const>(n_); }


/*********
 * Temp *
 *********/

ExprPtr Temp::clone() const {
  return make_unique<Temp>(t_);
}

void Temp::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  instrs.emplace_back(new assem::Move(t_, temp));
}

int Temp::toAssemInstrs(vector<assem::InstrPtr>&) const { return t_; }

ExprPtr Temp::optimize() { return make_unique<Temp>(t_); }

/*********
 * BinOp *
 *********/

ExprPtr BinOp::clone() const {
  return make_unique<BinOp>(expr1_->clone(), expr2_->clone(), bOp_);
}

BinOp::BinOp(ExprPtr&& expr1, ExprPtr&& expr2, BOp bOp)
    : expr1_(move(expr1)), expr2_(move(expr2)), bOp_(bOp) {}

void BinOp::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  switch (bOp_) {
    case BOp::LSHIFT:
      return handleShifts("shlq", temp, instrs);
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
  asmCode.append(" `8S1, `8D0");


  if (t1 == temp) {
    instrs.emplace_back(
        new assem::Operation(move(asmCode), { t1, t2 }, { t1 }));
  } else if (t2 == temp) {
    if (asmCode == "subq") {
      // Only subtraction is non-commutative
      instrs.emplace_back(new assem::Operation("negq `8D0", { t2 }, { t2 }));
      instrs.emplace_back(
          new assem::Operation("addq `8S1, `8D0", { t2, t1 }, { t2 }));
    } else {
      instrs.emplace_back(
          new assem::Operation(move(asmCode), { t2, t1 }, { t2 }));
    }
  } else {
    instrs.emplace_back(new assem::Move(t1, temp));
    instrs.emplace_back(
        new assem::Operation(move(asmCode), { temp, t2 }, { temp }));
  }
}


namespace {
  /* HalfConst will have already optimized to a shift, so we'll convert that to
   * a leaq if possible */
  bool isValidLeaq(HalfConst* halfConst) {
    return halfConst && !halfConst->reversed_ &&
           halfConst->bOp_ == BOp::LSHIFT && halfConst->n_ <= 4;
  }
}  // namespace


ExprPtr BinOp::optimize() {
  ExprPtr eOpt1 = expr1_->optimize();
  ExprPtr eOpt2 = expr2_->optimize();

  if (ExprPtr optConst = optimizeConst(eOpt1, eOpt2)) {
    return optConst;
  }

  // Leaq optimization: x + y*k, where k is 1,2,4, or 8
  // Note that if k is one of these, HalfConst will already be optimized to
  // a shift, so we act accordingly
  if (bOp_ == BOp::PLUS) {
    HalfConst* halfConst = dynamic_cast<HalfConst*>(eOpt1.get());
    if (isValidLeaq(halfConst)) {
      return make_unique<Leaq>(
          move(eOpt2), move(halfConst->expr_), 1 << halfConst->n_);
    }

    halfConst = dynamic_cast<HalfConst*>(eOpt2.get());
    if (isValidLeaq(halfConst)) {
      return make_unique<Leaq>(
          move(eOpt1), move(halfConst->expr_), 1 << halfConst->n_);
    }
  }

  return make_unique<BinOp>(move(eOpt1), move(eOpt2), bOp_);
}


ExprPtr BinOp::optimizeConst(ExprPtr& eOpt1, ExprPtr& eOpt2) {
  // Denotes the operations that can be optimized by turning them into a
  // HalfConst
  bool tryHalfConst;
  long (*op)(long, long);

  switch (bOp_) {
    case BOp::LSHIFT:
      tryHalfConst = false;
      op = [](long a, long b) { return a << b; };
      break;
    case BOp::ARSHIFT:
      tryHalfConst = false;
      op = [](long a, long b) { return a >> b; };
      break;
    case BOp::DIV:
      tryHalfConst = false;
      op = [](long a, long b) { return a / b; };
      break;
    case BOp::MOD:
      tryHalfConst = false;
      op = [](long a, long b) { return a % b; };
      break;
    case BOp::PLUS:
      tryHalfConst = false;
      op = [](long a, long b) { return a + b; };
      break;
    case BOp::MINUS:
      tryHalfConst = false;
      op = [](long a, long b) { return a - b; };
      break;
    case BOp::MUL:
      tryHalfConst = false;
      op = [](long a, long b) { return a * b; };
      break;
    case BOp::AND:
      tryHalfConst = false;
      op = [](long a, long b) { return a & b; };
      break;
    case BOp::OR:
      tryHalfConst = false;
      op = [](long a, long b) { return a | b; };
      break;
    case BOp::XOR:
      tryHalfConst = false;
      op = [](long a, long b) { return a ^ b; };
      break;
    default:
      throw invalid_argument("BinOp::optimizeConst");
  }

  // Const optimization
  if (eOpt2->getType() == ExprType::CONST) {
    long n2 = static_cast<const Const*>(eOpt2.get())->getInt();
    if (eOpt1->getType() == ExprType::CONST) {
      // const OP const
      long n1 = static_cast<const Const*>(eOpt1.get())->getInt();
      return make_unique<Const>(op(n1, n2));
    } else if (tryHalfConst) {
      // x OP const
      return HalfConst(move(eOpt1), n2, bOp_, false).optimize();
    }
  } else if (tryHalfConst && eOpt1->getType() == ExprType::CONST) {
    // const OP x
    long n1 = static_cast<const Const*>(eOpt1.get())->getInt();
    return HalfConst(move(eOpt2), n1, bOp_, true).optimize();
  }

  return nullptr;
}


/************
 * MemDeref *
 ************/

ExprPtr MemDeref::clone() const {
  return make_unique<MemDeref>(addr_->clone(), numBytes_);
}

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

ExprPtr DoThenEval::clone() const {
  vector<StmtPtr> stmtsClone;
  for (const StmtPtr& stmt : stmts_) {
    stmtsClone.push_back(stmt->clone());
  }

  return make_unique<DoThenEval>(move(stmtsClone), expr_->clone());
}

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

ExprPtr LabelAddr::clone() const {
  return make_unique<LabelAddr>(name_);
}

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


ExprPtr CallExpr::clone() const {
  vector<ExprPtr> paramsClone;
  for (const ExprPtr& param : params_) {
    paramsClone.push_back(param->clone());
  }

  return make_unique<CallExpr>(addr_->clone(), move(paramsClone), hasReturnValue_);
}

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

ExprPtr HalfConst::clone() const {
  return make_unique<HalfConst>(expr_->clone(), n_, bOp_, reversed_);
}


HalfConst::HalfConst(ExprPtr&& expr, long n, BOp bOp, bool reversed)
    : expr_(move(expr)), n_(n), bOp_(bOp), reversed_(reversed) {}

void HalfConst::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  string asmCode;
  switch (bOp_) {
    case BOp::LSHIFT:
      // Guaranteed to be of the form x << const, not const << x
      // Otherwise handled by BinOp (see BinOp::optimize)
      asmCode = "shlq";
      break;
    case BOp::PLUS:
      asmCode = "addq";
      break;
    case BOp::MINUS:
      if (reversed_) {
        // const - x: Subtraction is non-commutative
        expr_->toAssemInstrs(temp, instrs);
        instrs.emplace_back(
            new assem::Operation("negq `8D0", { temp }, { temp }));
        instrs.emplace_back(new assem::Operation(
            string("addq $").append(to_string(n_)).append(", `8D0"),
            { temp },
            { temp }));
        return;
      }
      asmCode = "subq";
      break;
    case BOp::MUL:
      if (n_ == -1) {
        // x * - 1 = -n
        expr_->toAssemInstrs(temp, instrs);
        instrs.emplace_back(
            new assem::Operation("negq `8D0", { temp }, { temp }));
        return;
      }
      asmCode = "imulq";
      break;
    case BOp::AND:
      asmCode = "andq";
      break;
    case BOp::OR:
      asmCode = "orq";
      break;
    case BOp::XOR:
      asmCode = "xorq";
      break;
    default:
      throw invalid_argument("HalfConst::toAssemInstrs");
  }

  expr_->toAssemInstrs(temp, instrs);
  instrs.emplace_back(new assem::Operation(
      asmCode.append(" $").append(to_string(n_)).append(", `8D0"),
      { temp },
      { temp }));
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
      case BOp::OR:
        return eOpt;
      case BOp::MINUS:
        return reversed_
                   ? make_unique<HalfConst>(move(eOpt), n_, bOp_, reversed_)
                   : move(eOpt);
      case BOp::MUL:
      case BOp::AND:
        return make_unique<Const>(0);
      case BOp::XOR:
        return make_unique<HalfConst>(move(eOpt), n_, bOp_, reversed_);
      default:
        throw invalid_argument("HalfConst::optimize(n == 0)");
    }
  }

  if (n_ == 1) {
    switch (bOp_) {
      case BOp::PLUS:
        return make_unique<IncDec>(move(eOpt), true);
      case BOp::MINUS:
        return reversed_ ? static_cast<ExprPtr>(make_unique<HalfConst>(
                               move(eOpt), n_, bOp_, reversed_))
                         : make_unique<IncDec>(move(eOpt), false);
      case BOp::MUL:
        return eOpt;
      case BOp::AND:
      case BOp::OR:
      case BOp::XOR:
        return make_unique<HalfConst>(move(eOpt), n_, bOp_, reversed_);
      default:
        throw invalid_argument("HalfConst::optimize(n == 1)");
    }
  }

  // Multiplication as shifts, e.g. 8x = x << 3 or 36x = (x << 5) + (x << 2)
  if (bOp_ == BOp::MUL) {
    vector<size_t> setBits = getSetBits(n_);
    if (setBits.size() == 1) {
      return make_unique<HalfConst>(
          move(eOpt), setBits[0], BOp::LSHIFT, reversed_);
    } else if (setBits.size() == 2) {
      vector<StmtPtr> stmts;
      int t = newTemp();
      stmts.emplace_back(make_unique<Assign>(make_unique<Temp>(t), move(eOpt)));
      return make_unique<DoThenEval>(
          move(stmts),
          make_unique<BinOp>(
              make_unique<HalfConst>(
                  make_unique<Temp>(t), setBits[0], BOp::LSHIFT, reversed_),
              make_unique<HalfConst>(
                  make_unique<Temp>(t), setBits[1], BOp::LSHIFT, reversed_),
              BOp::PLUS));
    }
  }

  return make_unique<HalfConst>(move(eOpt), n_, bOp_, reversed_);
}


/********
 * Leaq *
 ********/

Leaq::Leaq(ExprPtr&& e1, ExprPtr&& e2, u_char n)
    : e1_(move(e1)), e2_(move(e2)), n_(n) {}

ExprPtr Leaq::clone() const {
  return make_unique<Leaq>(e1_->clone(), e2_->clone(), n_);
}

void Leaq::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  int t1 = e1_->toAssemInstrs(instrs);
  int t2 = e2_->toAssemInstrs(instrs);
  string asmCode =
      string("leaq (`8S0, `8S1, ").append(to_string(n_)).append("), `8D0");
  instrs.emplace_back(
      new assem::Operation(move(asmCode), { t1, t2 }, { temp }));
}

ExprPtr Leaq::optimize() {
  return make_unique<Leaq>(e1_->optimize(), e2_->optimize(), n_);
}


/**********
 * IncDec *
 **********/

ExprPtr IncDec::clone() const {
  return make_unique<IncDec>(expr_->clone(), inc_);
}

IncDec::IncDec(ExprPtr&& expr, bool inc) : expr_(move(expr)), inc_(inc) {}

ExprPtr IncDec::optimize() {
  return make_unique<IncDec>(expr_->optimize(), inc_);
}


void IncDec::toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
    const {
  expr_->toAssemInstrs(temp, instrs);
  if (inc_) {
    instrs.emplace_back(new assem::Operation("incq `8D0", { temp }, { temp }));
  } else {
    instrs.emplace_back(new assem::Operation("decq `8D0", { temp }, { temp }));
  }
}

/********
 * Expr *
 ********/
int Expr::toAssemInstrs(vector<assem::InstrPtr>& instrs) const {
  int temp = newTemp();
  toAssemInstrs(temp, instrs);
  return temp;
}

std::ostream& operator<<(std::ostream& out, ExprType exprType) {
  switch (exprType) {
    case ExprType::BINOP: return out << "BINOP";
    case ExprType::CONST: return out << "BINOP";
    case ExprType::TEMP: return out << "BINOP";
    case ExprType::MEM_DEREF: return out << "BINOP";
    case ExprType::DO_THEN_EVAL: return out << "BINOP";
    case ExprType::LABEL_ADDR: return out << "BINOP";
    case ExprType::CALL: return out << "BINOP";
    case ExprType::HALF_CONST: return out << "BINOP";
    case ExprType::LEAQ: return out << "BINOP";
    case ExprType::INC_DEC: return out << "BINOP";
    default: throw invalid_argument("im::ExprType: operator<<");
  }
}

}  // namespace im
