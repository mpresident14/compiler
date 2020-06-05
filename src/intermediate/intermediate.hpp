#ifndef INTERMEDIATE_HPP
#define INTERMEDIATE_HPP

#include "src/assembly/assembly.hpp"
#include "src/assembly/temp.hpp"

#include <memory>
#include <string>
#include <vector>

namespace im {

class Decl {
public:
  virtual ~Decl(){};
  virtual assem::DeclPtr toAssemDecl() = 0;
};

class Stmt {
public:
  virtual ~Stmt(){};
  virtual void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) = 0;
};

enum class ExprType {
  BINOP,
  CONST,
  TEMP,
  MEM_DEREF,
  DO_THEN_EVAL,
  LABEL_ADDR,
  CALL
};

class Expr {
public:
  virtual ~Expr() {}
  virtual constexpr ExprType getType() const noexcept = 0;
  /* Add instructions that put the value of this Expr into this temp */
  virtual void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const = 0;
  /* Same as above but allow the Expr to determine the temp it is put into
   * (mainly to avoid unnecessary moves for Expr::Temp) */
  virtual int toAssemInstrs(std::vector<assem::InstrPtr>& instrs) const;
};

enum class BOp {
  PLUS,
  MINUS,
  MUL,
  DIV,
  MOD,
  AND,
  OR,
  LSHIFT,
  RSHIFT,
  ARSHIFT,
  XOR
};

enum class ROp { EQ, NEQ, LT, GT, LTE, GTE };

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;

/********
 * Decl *
 ********/

class Program {
public:
  Program(std::vector<DeclPtr>&& decls);
  assem::Program toAssemProg() const;

private:
  std::vector<DeclPtr> decls_;
};


class Func : public Decl {
public:
  Func(const std::string& name, std::vector<StmtPtr>&& stmts);
  assem::DeclPtr toAssemDecl() override;

private:
  std::string name_;
  std::vector<StmtPtr> stmts_;
};


class Ints : public Decl {
public:
  Ints(std::string_view label, std::vector<int>&& nums);
  assem::DeclPtr toAssemDecl() override;

private:
  std::string label_;
  std::vector<int> nums_;
};


/********
 * Stmt *
 ********/

/* Write a label */
class MakeLabel : public Stmt {
public:
  MakeLabel(const std::string& name);
  void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) override;
  /* Generates the instruction to create a label, stores it in the
   * data member, returns the raw pointer to it. We basically
   * use it so that Jumps can have an Instruction* target when they
   * is constructed. */
  assem::Label* genInstr();

private:
  std::string name_;
  std::unique_ptr<assem::Label> instr_ = nullptr;
};


class Jump : public Stmt {
public:
  Jump(assem::Label* label);
  void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) override;

private:
  /* Owned by some MakeLabel or already in the assem::InstrPtr vector */
  assem::Label* label_;
};


/*
 * Jump to appropriate label depending on result of comparison
 * NOTE: The ifFalse will often just fall through so we will have
 * jmp IF_FALSE followed by IF_FALSE. We remove such occurrences
 * in Function::regAlloc
 */
class CondJump : public Stmt {
public:
  CondJump(
      ExprPtr&& e1,
      ExprPtr&& e2,
      ROp rop,
      assem::Label* ifTrue,
      assem::Label* ifFalse);
  void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) override;

private:
  ExprPtr e1_;
  ExprPtr e2_;
  ROp rop_;
  /* Owned by some MakeLabel or already in the assem::InstrPtr vector */
  assem::Label* ifTrue_;
  assem::Label* ifFalse_;
};


class Assign : public Stmt {
public:
  Assign(ExprPtr&& e1, ExprPtr&& e2);
  void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) override;

private:
  ExprPtr e1_;
  ExprPtr e2_;
};


/* Call function and discard result */
class CallStmt : public Stmt {
public:
  CallStmt(ExprPtr&& addr, std::vector<ExprPtr>&& params);
  void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) override;

private:
  ExprPtr addr_;
  std::vector<ExprPtr> params_;
};


class ReturnStmt : public Stmt {
public:
  /* retValue may be null if returning void */
  ReturnStmt(ExprPtr&& retValue);
  void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) override;

private:
  ExprPtr retValue_;
};


/********
 * Expr *
 ********/

class BinOp : public Expr {
public:
  BinOp(ExprPtr&& expr1, ExprPtr&& expr2, BOp bop);
  constexpr ExprType getType() const noexcept override {
    return ExprType::BINOP;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;

private:
  void handleShifts(
      std::string asmCode,
      int temp,
      std::vector<assem::InstrPtr>& instrs) const;
  void handleDiv(bool isDiv, int temp, std::vector<assem::InstrPtr>& instrs)
      const;

  template <typename BinaryOp>
  void handleOthers(
      std::string asmCode,
      int temp,
      std::vector<assem::InstrPtr>& instrs,
      const BinaryOp& bOp) const;

  ExprPtr expr1_;
  ExprPtr expr2_;
  BOp bop_;
};


class Const : public Expr {
public:
  explicit constexpr Const(int n) : n_(n) {}
  constexpr ExprType getType() const noexcept override {
    return ExprType::CONST;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;
  constexpr int getInt() const noexcept { return n_; }

private:
  int n_;
};


class Temp : public Expr {
public:
  explicit constexpr Temp(int t) : t_(t) {}
  constexpr ExprType getType() const noexcept override {
    return ExprType::TEMP;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;
  int toAssemInstrs(std::vector<assem::InstrPtr>& instrs) const override;

  constexpr int getTemp() const noexcept { return t_; }

private:
  int t_;
};


/* *expr */
class MemDeref : public Expr {
public:
  MemDeref(ExprPtr&& addr, u_char numBytes);
  constexpr ExprType getType() const noexcept override {
    return ExprType::MEM_DEREF;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;

  const ExprPtr& getAddr() const noexcept { return addr_; }
  u_char getNumBytes() const noexcept { return numBytes_; }

private:
  ExprPtr addr_;
  u_char numBytes_;
};


class DoThenEval : public Expr {
public:
  DoThenEval(std::vector<StmtPtr>&& stmts, ExprPtr expr);
  constexpr ExprType getType() const noexcept override {
    return ExprType::DO_THEN_EVAL;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;

private:
  std::vector<StmtPtr> stmts_;
  ExprPtr expr_;
};


/* The address of a label (uses RIP-relative addressing) */
class LabelAddr : public Expr {
public:
  LabelAddr(const std::string& name);
  constexpr ExprType getType() const noexcept override {
    return ExprType::LABEL_ADDR;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;

  const std::string& getName() const noexcept { return name_; }

private:
  std::string name_;
};


/* Result of a function call  */
class CallExpr : public Expr {
public:
  CallExpr(ExprPtr&& addr, std::vector<ExprPtr>&& params, bool hasReturnValue);
  constexpr ExprType getType() const noexcept override {
    return ExprType::CALL;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;

private:
  ExprPtr addr_;
  std::vector<ExprPtr> params_;
  bool hasReturnValue_;
};


template <typename BinaryOp>
void BinOp::handleOthers(
    std::string asmCode,
    int temp,
    std::vector<assem::InstrPtr>& instrs,
    const BinaryOp& bOp) const {
  // TODO: Leaq optimization
  // TODO: Inc/deq optimization
  // TODO: Shift optimization for imulq
  using namespace std;

  if (expr2_->getType() == ExprType::CONST) {
    int n2 = static_cast<const Const*>(expr2_.get())->getInt();
    if (expr1_->getType() == ExprType::CONST) {
      // const OP const
      int n1 = static_cast<const Const*>(expr1_.get())->getInt();
      instrs.emplace_back(new assem::Operation(
          string("movq $").append(to_string(bOp(n1, n2))).append(", `8D0"),
          { },
          { temp }));
      return;
    } else {
      // x OP const
      expr1_->toAssemInstrs(temp, instrs);
      instrs.emplace_back(new assem::Operation(
          asmCode.append(" $").append(to_string(n2)).append(", `8D0"),
          { temp },
          { temp }));
      return;
    }
  } else if (expr1_->getType() == ExprType::CONST && asmCode != "subq") {
    // const OP x (OPs other than minus are commutative)
    int n1 = static_cast<const Const*>(expr1_.get())->getInt();
    expr2_->toAssemInstrs(temp, instrs);
    instrs.emplace_back(new assem::Operation(
        asmCode.append(" $").append(to_string(n1)).append(", `8D0"),
        { temp },
        { temp }));
    return;
  }


  int t1 = expr1_->toAssemInstrs(instrs);
  int t2 = expr2_->toAssemInstrs(instrs);
  // Need to create a new temp first in case t1 or t2 is the same as temp
  // TODO: Handle these cases separately so we don't waste instructions most of
  // the time
  int tRes = newTemp();
  instrs.emplace_back(new assem::Move(t1, tRes));
  instrs.emplace_back(new assem::Operation(
      asmCode.append(" `8S1, `8D0"), { tRes, t2 }, { tRes }));
  instrs.emplace_back(new assem::Move(tRes, temp));
}


}  // namespace im

#endif  // INTERMEDIATE_HPP
