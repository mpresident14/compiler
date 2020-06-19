#ifndef INTERMEDIATE_HPP
#define INTERMEDIATE_HPP

#include "src/assembly/assembly.hpp"
#include "src/assembly/temp.hpp"

#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace im {

class Decl;
class Stmt;
class Expr;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using DeclPtr = std::unique_ptr<Decl>;

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
  CALL,
  HALF_CONST,
  LEAQ,
  INC_DEC
};
std::ostream& operator<<(std::ostream& out, ExprType exprType);


class Expr {
public:
  virtual ~Expr() {}
  virtual constexpr ExprType getType() const noexcept = 0;
  /* Add instructions that put the value of this Expr into this temp.
   * Because of the way we wrote the spill code (i.e., poorly), any
   * temp that is a destination must be marked with a D, even if it is also
   * a source. */
  virtual void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const = 0;
  /* Same as above but allow the Expr to determine the temp it is put into
   * (mainly to avoid unnecessary moves for Expr::Temp) */
  virtual int toAssemInstrs(std::vector<assem::InstrPtr>& instrs) const;
  virtual std::string asmChunk(size_t numBytes, bool asSrc, size_t index) const;
  /* Return an optimized version of the expression. This object is invalidated
   */
  // TODO: Constant propagation
  virtual ExprPtr optimize() = 0;
};

enum class BOp { PLUS, MINUS, MUL, DIV, MOD, AND, OR, LSHIFT, ARSHIFT, XOR };

enum class ROp { EQ, NEQ, LT, GT, LTE, GTE };


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
class ExprStmt : public Stmt {
public:
  ExprStmt(ExprPtr&& expr);
  void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) override;

private:
  ExprPtr expr_;
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

class Const : public Expr {
public:
  explicit constexpr Const(long n) : n_(n) {}
  constexpr ExprType getType() const noexcept override {
    return ExprType::CONST;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;
  std::string asmChunk(size_t numBytes, bool asSrc, size_t index)
      const override;
  constexpr long getInt() const noexcept { return n_; }
  ExprPtr optimize() override;

private:
  long n_;
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
  ExprPtr optimize() override;

  constexpr int getTemp() const noexcept { return t_; }


private:
  int t_;
};


class BinOp : public Expr {
public:
  BinOp(ExprPtr&& expr1, ExprPtr&& expr2, BOp bOp);
  constexpr ExprType getType() const noexcept override {
    return ExprType::BINOP;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;
  ExprPtr optimize() override;

private:
  void handleShifts(
      std::string asmCode,
      int temp,
      std::vector<assem::InstrPtr>& instrs) const;
  void handleDiv(bool isDiv, int temp, std::vector<assem::InstrPtr>& instrs)
      const;

  void handleOthers(
      std::string asmCode,
      int temp,
      std::vector<assem::InstrPtr>& instrs) const;

  ExprPtr optimizeConst(ExprPtr& eOpt1, ExprPtr& eOpt2);

  ExprPtr expr1_;
  ExprPtr expr2_;
  BOp bOp_;
};


/* *expr */
class MemDeref : public Expr {
public:
  /* mult may be nullptr */
  MemDeref(long offset, ExprPtr&& addr, ExprPtr&& mult, u_char numBytes);
  constexpr ExprType getType() const noexcept override {
    return ExprType::MEM_DEREF;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;
  ExprPtr optimize() override;

  std::string genAsmCode(size_t srcIndex) const;

  const ExprPtr& getAddr() const noexcept { return addr_; }
  u_char getNumBytes() const noexcept { return numBytes_; }
  long getOffset() const noexcept { return offset_; }
  const ExprPtr& getMult() const noexcept { return mult_; }

private:
  long offset_;
  ExprPtr addr_;
  ExprPtr mult_;
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
  ExprPtr optimize() override;

// private:
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
  ExprPtr optimize() override;

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
  ExprPtr optimize() override;

private:
  ExprPtr addr_;
  std::vector<ExprPtr> params_;
  bool hasReturnValue_;
};


/* Functions below are mainly used to produce more efficient assembly code
 * via the optimize() method */


/* See BinOp::optimizeConst for valid BOps */
class HalfConst : public Expr {
public:
  HalfConst(ExprPtr&& expr, long n, BOp bOp, bool reversed);
  constexpr ExprType getType() const noexcept override {
    return ExprType::HALF_CONST;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;
  ExprPtr optimize() override;

  // private:
  ExprPtr expr_;
  long n_;
  BOp bOp_;
  bool reversed_;
};


class Leaq : public Expr {
public:
  Leaq(long offset, ExprPtr&& e1, ExprPtr&& e2, u_char n);
  constexpr ExprType getType() const noexcept override {
    return ExprType::LEAQ;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;
  ExprPtr optimize() override;

  // private:
  long offset_;
  ExprPtr e1_;
  ExprPtr e2_;
  u_char n_;
};


class IncDec : public Expr {
public:
  IncDec(ExprPtr&& expr, bool inc);
  constexpr ExprType getType() const noexcept override {
    return ExprType::INC_DEC;
  }
  void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs)
      const override;
  ExprPtr optimize() override;

  // private:
  ExprPtr expr_;
  bool inc_;
};


constexpr bool isConstChunk(std::string_view asmChunk) noexcept {
  return asmChunk.front() == '$';
}

}  // namespace im

#endif  // INTERMEDIATE_HPP
