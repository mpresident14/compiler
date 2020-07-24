#include "src/intermediate/intermediate.hpp"

#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace im {


/*********
 * Const *
 *********/

void Const::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  instrs.emplace_back(
      new assem::Operation(string("movq $").append(to_string(n_)).append(", `8D0"), {}, { temp }));
}


namespace {
  long truncate(long n, size_t numBytes) {
    switch (numBytes) {
      case 8:
        return n;
      case 4:
        return (int)n;
      case 2:
        return (short)n;
      case 1:
        return (char)n;
      default:
        throw invalid_argument("truncate: " + to_string(numBytes));
    }
  }
}  // namespace

string Const::asmChunk(size_t numBytes, bool, size_t) const {
  return "$" + to_string(truncate(n_, numBytes));
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

void BinOp::handleShifts(string asmCode, int temp, vector<assem::InstrPtr>& instrs) const {
  expr1_->toAssemInstrs(temp, instrs);

  // If shift number (expr2_) is not an immediate, its value must be in %cl
  if (expr2_->getCategory() == Expr::Category::CONST) {
    asmCode.append(expr2_->asmChunk(8, 0, 0)).append(", `8D0");
    instrs.emplace_back(new assem::Operation(asmCode, { temp }, { temp }));
  } else {
    expr2_->toAssemInstrs(RCX, instrs);
    asmCode.append(" %cl, `8D0");
    instrs.emplace_back(new assem::Operation(asmCode, { RCX, temp }, { temp }));
  }
}

void BinOp::handleDiv(bool isDiv, int temp, vector<assem::InstrPtr>& instrs) const {
  expr1_->toAssemInstrs(RAX, instrs);
  int t2 = expr2_->toAssemInstrs(instrs);
  // Sign-extend %rax into %rdx
  instrs.emplace_back(new assem::Operation("cqto", { RAX }, { RDX }));
  // Divide %rax%rdx by t2, quotient in %rax, remainder in %rdx
  instrs.emplace_back(new assem::Operation("idivq `8S0", { t2, RAX, RDX }, { RAX, RDX }));
  if (isDiv) {
    // If division, move %rax into temp
    Temp(RAX).toAssemInstrs(temp, instrs);
  } else {
    // If mod, move %rdx into temp
    Temp(RDX).toAssemInstrs(temp, instrs);
  }
}


void BinOp::handleOthers(std::string asmCode, int temp, std::vector<assem::InstrPtr>& instrs)
    const {
  int t1 = expr1_->toAssemInstrs(instrs);
  int t2 = expr2_->toAssemInstrs(instrs);
  asmCode.append(" `8S1, `8D0");


  if (t1 == temp) {
    instrs.emplace_back(new assem::Operation(move(asmCode), { t1, t2 }, { t1 }));
  } else if (t2 == temp) {
    if (asmCode == "subq") {
      // Only subtraction is non-commutative
      instrs.emplace_back(new assem::Operation("negq `8D0", { t2 }, { t2 }));
      instrs.emplace_back(new assem::Operation("addq `8S1, `8D0", { t2, t1 }, { t2 }));
    } else {
      instrs.emplace_back(new assem::Operation(move(asmCode), { t2, t1 }, { t2 }));
    }
  } else {
    instrs.emplace_back(new assem::Move(t1, temp));
    instrs.emplace_back(new assem::Operation(move(asmCode), { temp, t2 }, { temp }));
  }
}


/************
 * MemDeref *
 ************/

const char* movExtendSuffix(u_char numBytes) {
  switch (numBytes) {
    case 8:
      return "";
    case 4:
      return "slq";
    case 2:
      return "swq";
    case 1:
      return "sbq";
    default:
      throw invalid_argument("movExtendSuffix: " + to_string(numBytes));
  }
}

MemDeref::MemDeref(long offset, ExprPtr&& addr, ExprPtr&& mult, u_char numBytes)
    : offset_(offset), baseAddr_(move(addr)), mult_(move(mult)), numBytes_(numBytes) {}

void MemDeref::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  vector<int> srcTemps;
  srcTemps.push_back(baseAddr_->toAssemInstrs(instrs));

  if (mult_) {
    srcTemps.push_back(mult_->toAssemInstrs(instrs));
  }

  ostringstream asmOp;
  asmOp << "mov" << movExtendSuffix(numBytes_) << ' ' << genAsmCode(0) << ", `8D0";
  instrs.emplace_back(new assem::Operation(asmOp.str(), move(srcTemps), { temp }, true));
}

string MemDeref::genAsmCode(size_t srcIndex) const {
  ostringstream code;
  if (offset_ != 0) {
    code << offset_;
  }
  code << "(`8S" << srcIndex;

  if (mult_) {
    code << ", `8S" << srcIndex + 1 << ", " << (size_t)numBytes_;
  }
  code << ')';
  return code.str();
}

/**************
 * DoThenEval *
 **************/

DoThenEval::DoThenEval(vector<StmtPtr>&& stmts, ExprPtr expr)
    : stmts_(move(stmts)), expr_(move(expr)) {}

void DoThenEval::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
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
  instrs.emplace_back(
      new assem::Operation(string("leaq ").append(name_).append("(%rip), `8D0"), {}, { temp }));
}


/************
 * CallExpr *
 ************/
CallExpr::CallExpr(ExprPtr&& addr, vector<ExprPtr>&& params, bool hasReturnValue)
    : addr_(move(addr)), params_(move(params)), hasReturnValue_(hasReturnValue) {}


void CallExpr::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  vector<int> argTemps;
  size_t numParams = params_.size();

  // Move all parameters to temps. We don't move the parameters straight into registers because
  // evaluating a later parameter may overwrite a parameter that we previously moved (e.g., we may
  // do a function call as the arg #1 and overwrite arg #0)
  for (size_t i = 0; i < numParams; ++i) {
    argTemps.push_back(params_[i]->toAssemInstrs(instrs));
  }

  // Put up to 6 parameters in argument registers
  vector<int> srcTemps;
  size_t numRegParams = min(numParams, (size_t)6);
  for (size_t i = 0; i < numRegParams; ++i) {
    int argReg = ARG_REGS[i];
    Temp(argTemps[i]).toAssemInstrs(argReg, instrs);
    srcTemps.push_back(argReg);
  }

  // Put rest on the stack
  for (long i = (long)numParams - 1; i >= 6; --i) {
    instrs.emplace_back(new assem::Operation("pushq `8S0", { argTemps[i] }, {}));
  }

  if (addr_->getCategory() == Expr::Category::LABEL_ADDR) {
    // If we are calling a function name, just call it directly
    instrs.emplace_back(new assem::Operation(
        "callq " + static_cast<LabelAddr*>(addr_.get())->name_,
        move(srcTemps),
        regsAsInts(CALLER_SAVE_REGS)));
  } else {
    // If we are calling an address, we need to put it in a register
    int t = addr_->toAssemInstrs(instrs);
    srcTemps.push_back(t);
    instrs.emplace_back(new assem::Operation(
        "callq *`8S" + to_string(numRegParams), move(srcTemps), regsAsInts(CALLER_SAVE_REGS)));
  }

  // Decrement stack for overflow parameters
  size_t numStackParams = numParams - numRegParams;
  if (numStackParams != 0) {
    instrs.emplace_back(new assem::Operation(
        string("addq $").append(to_string(numStackParams * 8)).append(", `8S0"), { RSP }, {}));
  }

  // Move result from %rax to temp if needed
  if (hasReturnValue_) {
    instrs.emplace_back(new assem::Move(RAX, temp));
  }
}


/***********
 * IntCast *
 ***********/
IntCast::IntCast(ExprPtr&& expr, u_char toNumBytes) : expr_(move(expr)), toNumBytes_(toNumBytes) {}

void IntCast::toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const {
  int t = expr_->toAssemInstrs(instrs);
  ostringstream asmOp;
  asmOp << "mov" << movExtendSuffix(toNumBytes_) << " `" << (size_t)toNumBytes_ << "S0, `8D0";
  instrs.emplace_back(new assem::Operation(asmOp.str(), { t }, { temp }));
}


/*************
 * HalfConst *
 *************/

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
        instrs.emplace_back(new assem::Operation("negq `8D0", { temp }, { temp }));
        instrs.emplace_back(new assem::Operation(
            string("addq $").append(to_string(n_)).append(", `8D0"), { temp }, { temp }));
        return;
      }
      asmCode = "subq";
      break;
    case BOp::MUL:
      if (n_ == -1) {
        // x * - 1 = -n
        expr_->toAssemInstrs(temp, instrs);
        instrs.emplace_back(new assem::Operation("negq `8D0", { temp }, { temp }));
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
      asmCode.append(" $").append(to_string(n_)).append(", `8D0"), { temp }, { temp }));
}


/********
 * Leaq *
 ********/

void Leaq::toAssemInstrs(int temp, vector<assem::InstrPtr>& instrs) const {
  vector<int> srcTemps;
  srcTemps.push_back(baseAddr_->toAssemInstrs(instrs));
  if (mult_) {
    srcTemps.push_back(mult_->toAssemInstrs(instrs));
  }

  ostringstream asmCode;
  asmCode << "leaq " << genAsmCode(0) << ", `8D0";
  instrs.emplace_back(new assem::Operation(asmCode.str(), move(srcTemps), { temp }, true));
}


/**********
 * IncDec *
 **********/

IncDec::IncDec(ExprPtr&& expr, bool inc) : expr_(move(expr)), inc_(inc) {}

void IncDec::toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const {
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

std::string Expr::asmChunk(size_t numBytes, bool asSrc, size_t index) const {
  ostringstream ret;
  ret << '`' << numBytes << (asSrc ? 'S' : 'D') << index;
  return ret.str();
}

std::ostream& operator<<(std::ostream& out, Expr::Category exprType) {
  switch (exprType) {
    case Expr::Category::BINOP:
      return out << "BINOP";
    case Expr::Category::CONST:
      return out << "CONST";
    case Expr::Category::TEMP:
      return out << "TEMP";
    case Expr::Category::MEM_DEREF:
      return out << "MEM_DEREF";
    case Expr::Category::DO_THEN_EVAL:
      return out << "DO_THEN_EVAL";
    case Expr::Category::LABEL_ADDR:
      return out << "LABEL_ADDR";
    case Expr::Category::CALL:
      return out << "CALL";
    case Expr::Category::HALF_CONST:
      return out << "HALF_CONST";
    case Expr::Category::LEAQ:
      return out << "LEAQ";
    case Expr::Category::INC_DEC:
      return out << "INC_DEC";
    default:
      throw invalid_argument("im::Expr::Category: operator<<");
  }
}

std::ostream& operator<<(std::ostream& out, BOp bOp) {
  switch (bOp) {
    case BOp::PLUS:
      return out << "+";
    case BOp::MUL:
      return out << "*";
    case BOp::MINUS:
      return out << "-";
    case BOp::DIV:
      return out << "/";
    case BOp::MOD:
      return out << "%";
    case BOp::AND:
      return out << "&";
    case BOp::OR:
      return out << "|";
    case BOp::XOR:
      return out << "^";
    case BOp::LSHIFT:
      return out << "<<";
    case BOp::ARSHIFT:
      return out << ">>";
    default:
      throw invalid_argument("BOp::operator<<");
  }
}

}  // namespace im
