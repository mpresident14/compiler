#include "src/intermediate/intermediate.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace im {


/*********
 * Const *
 *********/

ExprPtr Const::optimize() { return make_unique<Const>(n_); }


/*********
 * Temp *
 *********/

ExprPtr Temp::optimize() { return make_unique<Temp>(t_); }

/*********
 * BinOp *
 *********/

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

  // Leaq optimization: x + y*k, where k is 1,2,4, or 8
  // Note that if k is one of these, HalfConst will already be optimized to
  // a shift, so we act accordingly
  if (bOp_ == BOp::PLUS) {
    HalfConst* halfConst = dynamic_cast<HalfConst*>(eOpt1.get());
    if (isValidLeaq(halfConst)) {
      return make_unique<Leaq>(
          0, move(eOpt2), move(halfConst->expr_), 1 << halfConst->n_);
    }

    halfConst = dynamic_cast<HalfConst*>(eOpt2.get());
    if (isValidLeaq(halfConst)) {
      return make_unique<Leaq>(
          0, move(eOpt1), move(halfConst->expr_), 1 << halfConst->n_);
    }
  }

  if (ExprPtr optConst = optimizeConst(eOpt1, eOpt2)) {
    return optConst;
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
      tryHalfConst = true;
      op = [](long a, long b) { return a + b; };
      break;
    case BOp::MINUS:
      tryHalfConst = true;
      op = [](long a, long b) { return a - b; };
      break;
    case BOp::MUL:
      tryHalfConst = true;
      op = [](long a, long b) { return a * b; };
      break;
    case BOp::AND:
      tryHalfConst = true;
      op = [](long a, long b) { return a & b; };
      break;
    case BOp::OR:
      tryHalfConst = true;
      op = [](long a, long b) { return a | b; };
      break;
    case BOp::XOR:
      tryHalfConst = true;
      op = [](long a, long b) { return a ^ b; };
      break;
    default:
      throw invalid_argument("BinOp::optimizeConst");
  }

  // Const optimization
  if (eOpt2->getType() == ExprType::CONST) {
    long n2 = static_cast<const Const*>(eOpt2.get())->n_;
    if (eOpt1->getType() == ExprType::CONST) {
      // const OP const
      long n1 = static_cast<const Const*>(eOpt1.get())->n_;
      return make_unique<Const>(op(n1, n2));
    } else if (tryHalfConst) {
      // x OP const
      return HalfConst(move(eOpt1), n2, bOp_, false).optimize();
    }
  } else if (tryHalfConst && eOpt1->getType() == ExprType::CONST) {
    // const OP x
    long n1 = static_cast<const Const*>(eOpt1.get())->n_;
    return HalfConst(move(eOpt2), n1, bOp_, true).optimize();
  }

  return nullptr;
}


/************
 * MemDeref *
 ************/

ExprPtr MemDeref::optimize() {
  ExprPtr multOpt = mult_ ? mult_->optimize() : nullptr;

  if (multOpt) {
    // If multiplier is a constant, just increase the offset
    if (const Const* constMult = dynamic_cast<const Const*>(mult_.get())) {
      offset_ += constMult->n_ * numBytes_;
      multOpt = nullptr;
    }
  }

  return make_unique<MemDeref>(
      offset_, addr_->optimize(), move(multOpt), numBytes_);
}

/**************
 * DoThenEval *
 **************/

ExprPtr DoThenEval::optimize() {
  return make_unique<DoThenEval>(move(stmts_), expr_->optimize());
}

/*************
 * LabelAddr *
 *************/

ExprPtr LabelAddr::optimize() { return make_unique<LabelAddr>(move(name_)); }

/************
 * CallExpr *
 ************/

ExprPtr CallExpr::optimize() {
  vector<ExprPtr> optParams;
  optParams.reserve(params_.size());
  for (const ExprPtr& param : params_) {
    optParams.push_back(param->optimize());
  }
  return make_unique<CallExpr>(
      addr_->optimize(), move(optParams), hasReturnValue_);
}

/********
 * Cast *
 ********/

ExprPtr Cast::optimize() {
  return make_unique<Cast>(expr_->optimize(), toNumBytes_);
}

/*************
 * HalfConst *
 *************/

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
      case BOp::LSHIFT:
        if (reversed_) {
          throw invalid_argument("HalfConst::optimize(n == 0, LSHIFT)");
        }
        // Fall thru
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
      case BOp::LSHIFT:
        if (reversed_) {
          throw invalid_argument("HalfConst::optimize(n == 0, LSHIFT)");
        }
        // Fall thru
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
              BOp::PLUS)
              ->optimize());
    }
  }

  // If expr is a leaq (which may be wrapped in a DoThenEval), just set the leaq
  // offset to n_
  if (bOp_ == BOp::PLUS) {
    if (Leaq* leaq = dynamic_cast<Leaq*>(eOpt.get())) {
      return make_unique<Leaq>(
          leaq->offset_ + n_, move(leaq->e1_), move(leaq->e2_), leaq->n_);
    } else if (DoThenEval* dte = dynamic_cast<DoThenEval*>(eOpt.get())) {
      if (Leaq* leaq = dynamic_cast<Leaq*>(dte->expr_.get())) {
        return make_unique<DoThenEval>(
          move(dte->stmts_),
          make_unique<Leaq>(
              leaq->offset_ + n_, move(leaq->e1_), move(leaq->e2_), leaq->n_));
      }
    }
  }

  return make_unique<HalfConst>(move(eOpt), n_, bOp_, reversed_);
}


/********
 * Leaq *
 ********/

ExprPtr Leaq::optimize() {
  return make_unique<Leaq>(offset_, e1_->optimize(), e2_->optimize(), n_);
}


/**********
 * IncDec *
 **********/

ExprPtr IncDec::optimize() {
  return make_unique<IncDec>(expr_->optimize(), inc_);
}

}  // namespace im
