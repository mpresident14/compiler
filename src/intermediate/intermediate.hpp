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
    virtual assem::DeclPtr toAssemDecl() const = 0;
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
    virtual void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const = 0;
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
    assem::DeclPtr toAssemDecl() const override;

  private:
    std::string name_;
    std::vector<StmtPtr> stmts_;
  };


  /********
   * Stmt *
   ********/

  /* Sequence of statemnts */
  // TODO: Remove
  // class Block : public Stmt {
  // public:
  //   Block(std::vector<StmtPtr>&& stmts);
  //   void toAssemInstrs(std::vector<assem::InstrPtr>& instrs) override;

  // private:
  //   std::vector<StmtPtr> stmts_;
  // };

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


  /* Jump to appropriate label depending on result of comparison */
  // TODO: The ifFalse will often just fall through so we will have
  // jmp IF_FALSE followed by IF_FALSE. Optimize these out and put a
  // comment saying we did that.
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
    void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const override;

  private:
    void handleShifts(
        std::string asmCode,
        int temp,
        std::vector<assem::InstrPtr>& instrs) const;
    void handleDiv(bool isDiv, int temp, std::vector<assem::InstrPtr>& instrs) const;
    void handleOthers(
        std::string asmCode,
        int temp,
        std::vector<assem::InstrPtr>& instrs) const;

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
    void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const override;
    constexpr int getInt() { return n_; }

  private:
    int n_;
  };


  class Temp : public Expr {
  public:
    explicit constexpr Temp(int t) : t_(t) {}
    constexpr ExprType getType() const noexcept override {
      return ExprType::TEMP;
    }
    void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const override;

    constexpr int getTemp() { return t_; }

  private:
    int t_;
  };


  /* *expr */
  class MemDeref : public Expr {
  public:
    MemDeref(ExprPtr&& addr);
    constexpr ExprType getType() const noexcept override {
      return ExprType::MEM_DEREF;
    }
    void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const override;

    const ExprPtr& getAddr() const { return addr_; }

  private:
    ExprPtr addr_;
  };

  class DoThenEval : public Expr {
  public:
    DoThenEval(std::vector<StmtPtr>&& stmts, ExprPtr expr);
    constexpr ExprType getType() const noexcept override {
      return ExprType::DO_THEN_EVAL;
    }
    void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const override;

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
    void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const override;

    const std::string& getName() { return name_; }

  private:
    std::string name_;
  };


  /* Result of a function call  */
  class CallExpr : public Expr {
  public:
    CallExpr(
        ExprPtr&& addr,
        std::vector<ExprPtr>&& params,
        bool hasReturnValue);
    constexpr ExprType getType() const noexcept override {
      return ExprType::CALL;
    }
    void toAssemInstrs(int temp, std::vector<assem::InstrPtr>& instrs) const override;

  private:
    ExprPtr addr_;
    std::vector<ExprPtr> params_;
    bool hasReturnValue_;
  };


}  // namespace im

#endif  // INTERMEDIATE_HPP
