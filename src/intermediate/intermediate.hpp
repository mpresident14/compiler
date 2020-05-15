#ifndef INTERMEDIATE_HPP
#define INTERMEDIATE_HPP

#include "src/x86gen/instruction.hpp"
#include "src/x86gen/temp.hpp"

#include <memory>
#include <string>
#include <vector>

namespace intermediate {

  class Decl {
  public:
    Decl() = default;
    virtual ~Decl(){};
    virtual void toInstrs(std::vector<InstrPtr>& instrs) const = 0;

  private:
  };

  class Stmt {
  public:
    Stmt() = default;
    virtual ~Stmt(){};
    virtual void toInstrs(std::vector<InstrPtr>& instrs) = 0;

  private:
  };

  enum class ExprType { BINOP, CONST, TEMP, MEM_DEREF, LABEL_ADDR, CALL };

  class Expr {
  public:
    Expr() = default;
    virtual ~Expr() {}
    virtual ExprType getType() const noexcept = 0;
    /* Add instructions that put the value of this Expr into this temp */
    virtual void toInstrs(int temp, std::vector<InstrPtr>& instrs) const = 0;

  private:
  };

  enum class Bop {
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

  enum class Rop { EQ, NEQ, LESS, GREATER, LESSEQ, GREATEREQ };

  using ExprPtr = std::unique_ptr<Expr>;
  using StmtPtr = std::unique_ptr<Stmt>;
  using DeclPtr = std::unique_ptr<Decl>;

  /********
   * Stmt *
   ********/

  class Block : public Stmt {
  public:
    Block(std::vector<StmtPtr>&& stmts);
    void toInstrs(std::vector<InstrPtr>& instrs) override;

  private:
    std::vector<StmtPtr> stmts_;
  };


  class MakeLabel : public Stmt {
  public:
    MakeLabel(const std::string& name);
    void toInstrs(std::vector<InstrPtr>& instrs) override;
    /* Generates the instruction to create a label, stores it in the
     * data member, returns the raw pointer to it. We basically
     * use it so that Jumps can have an Instruction* target when they
     * is constructed. */
    Label* genInstr();

  private:
    std::string name_;
    std::unique_ptr<Label> instr_ = nullptr;
  };


  class Jump : public Stmt {
  public:
    Jump(Label* label);
    void toInstrs(std::vector<InstrPtr>& instrs) override;

  private:
    /* Owned by some MakeLabel or already in the InstrPtr vector */
    Label* label_;
  };

  class CondJump : public Stmt {
  public:
    CondJump(ExprPtr e1, ExprPtr e2, Rop rop, Label* ifTrue);
    void toInstrs(std::vector<InstrPtr>& instrs) override;

  private:
    ExprPtr e1_;
    ExprPtr e2_;
    Rop rop_;
    /* Owned by some MakeLabel or already in the InstrPtr vector */
    Label* ifTrue_;
  };

  class Assign : public Stmt {
  public:
    Assign(ExprPtr e1, ExprPtr e2);
    void toInstrs(std::vector<InstrPtr>& instrs) override;

  private:
    ExprPtr e1_;
    ExprPtr e2_;
  };


  /********
   * Expr *
   ********/

  class BinOp : public Expr {
  public:
    BinOp(ExprPtr&& expr1, ExprPtr&& expr2, Bop bop);
    constexpr ExprType getType() const noexcept override {
      return ExprType::BINOP;
    }
    void toInstrs(int temp, std::vector<InstrPtr>& instrs) const override;

  private:
    void handleShifts(
        std::string asmCode,
        int temp,
        std::vector<InstrPtr>& instrs) const;
    void handleDiv(bool isDiv, int temp, std::vector<InstrPtr>& instrs) const;
    void handleOthers(
        std::string asmCode,
        int temp,
        std::vector<InstrPtr>& instrs) const;

    ExprPtr expr1_;
    ExprPtr expr2_;
    Bop bop_;
  };


  class Const : public Expr {
  public:
    explicit constexpr Const(int n) : n_(n) {}
    constexpr ExprType getType() const noexcept override {
      return ExprType::CONST;
    }
    void toInstrs(int temp, std::vector<InstrPtr>& instrs) const override;
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
    void toInstrs(int temp, std::vector<InstrPtr>& instrs) const override;

    constexpr int getTemp() { return t_; }

  private:
    int t_;
  };


  class MemDeref : public Expr {
  public:
    MemDeref(ExprPtr&& addr);
    constexpr ExprType getType() const noexcept override {
      return ExprType::MEM_DEREF;
    }
    void toInstrs(int temp, std::vector<InstrPtr>& instrs) const override;

    const ExprPtr& getAddr() const { return addr_; }

  private:
    ExprPtr addr_;
  };

  class DoThenEval : public Expr {
  public:
    DoThenEval(std::vector<StmtPtr>&& stmts, ExprPtr expr);
    void toInstrs(int temp, std::vector<InstrPtr>& instrs) const override;

  private:
    std::vector<StmtPtr> stmts_;
    ExprPtr expr_;
  };

  class LabelAddr : public Expr {
  public:
    LabelAddr(const std::string& name);
    constexpr ExprType getType() const noexcept override {
      return ExprType::LABEL_ADDR;
    }
    void toInstrs(int temp, std::vector<InstrPtr>& instrs) const override;

    const std::string& getName() { return name_; }

  private:
    std::string name_;
  };


  class Call : public Expr {
  public:
    Call(ExprPtr&& addr, std::vector<ExprPtr>&& params, bool hasReturnValue);
    constexpr ExprType getType() const noexcept override {
      return ExprType::CALL;
    }
    void toInstrs(int temp, std::vector<InstrPtr>& instrs) const override;

  private:
    ExprPtr addr_;
    std::vector<ExprPtr> params_;
    bool hasReturnValue_;
  };


}  // namespace intermediate

#endif  // INTERMEDIATE_HPP
