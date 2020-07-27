#ifndef DECL_HPP
#define DECL_HPP

#include "src/intermediate/intermediate.hpp"
#include "src/language/typecheck/type.hpp"

#include <variant>
#include <vector>

class Ctx;
namespace language {


class Decl {
public:
  enum class Category { CLASS, FUNC };
  constexpr Decl(size_t line) : line_(line) {}
  virtual ~Decl() {}
  virtual void toImDecls(std::vector<im::DeclPtr>&, Ctx&) = 0;
  virtual void addToCtx(Ctx& ctx) = 0;
  virtual Category getCategory() const noexcept = 0;

  size_t line_;
};

using DeclPtr = std::unique_ptr<Decl>;


class Block;
class Func : public Decl {
public:
  enum Modifier { NONE = 0x0, VIRTUAL = 0x2, OVERRIDE = 0x4, STATIC = 0x8, CTOR = 0x16 };

  constexpr bool static isVirtual(int mods) noexcept {
    return (mods & Modifier::VIRTUAL) || (mods & Modifier::OVERRIDE);
  }
  constexpr bool static isStatic(int mods) noexcept {
    return mods & Modifier::STATIC;
  }
  constexpr bool static isCtor(int mods) noexcept {
    return mods & Modifier::CTOR;
  }

  Func(
      int modifiers,
      TypePtr&& returnType,
      std::string_view name,
      std::vector<std::pair<TypePtr, std::string>>&& params,
      std::unique_ptr<Block>&& body,
      size_t line);
  void toImDecls(std::vector<im::DeclPtr>& imDecls, Ctx& ctx) override;
  void addToCtx(Ctx& ctx) override;
  Category getCategory() const noexcept override { return Category::FUNC; }
  void checkTypes(Ctx& ctx) const;
  constexpr bool isVirtual() const noexcept { return Func::isVirtual(modifiers_); }

  int modifiers_;
  TypePtr returnType_;
  std::string name_;
  std::vector<TypePtr> paramTypes_;
  std::vector<std::string> paramNames_;
  std::unique_ptr<Block> body_;
  size_t id_;

protected:
  virtual std::vector<im::StmtPtr> paramsToImStmts(Ctx& ctx);

private:
  /* Check if for no return at end of function and handle accordingly */
  void checkForReturn(Ctx& ctx);

  static size_t nextId_;
};

class Constructor : public Func {
public:
  Constructor(
      std::string_view name,
      std::vector<std::pair<TypePtr, std::string>>&& params,
      std::unique_ptr<Block>&& body,
      size_t line);
  void toImDecls(std::vector<im::DeclPtr>& imDecls, Ctx& ctx) override;

  std::optional<std::string> vTableName_ = {};
};


class ClassDecl : public Decl {
public:
  struct Field {
    TypePtr type;
    std::string name;
    size_t line;
  };

  struct ClassElem {
    enum class Type { FIELD, CTOR, METHOD };

    Type type;
    std::variant<Field, Constructor, std::unique_ptr<Func>> elem;
  };

  ClassDecl(std::string_view name, std::vector<ClassElem>&& classElems, size_t line);
  ClassDecl(
      std::string_view name,
      std::vector<std::string>&& superQuals,
      std::string_view superName,
      std::vector<ClassElem>&& classElems,
      size_t line);
  void toImDecls(std::vector<im::DeclPtr>& imDecls, Ctx& ctx) override;
  void addToCtx(Ctx& ctx) override;
  Category getCategory() const noexcept override { return Category::CLASS; }

  std::string name_;
  std::vector<std::string> superQuals_;
  std::optional<std::string> superName_;
  std::vector<Field> fields_;
  std::vector<Constructor> ctors_;
  std::vector<std::unique_ptr<Func>> staticMethods_;
  std::vector<std::unique_ptr<Func>> nonVMethods_;
  std::vector<std::unique_ptr<Func>> vMethods_;
  std::vector<std::unique_ptr<Func>> overrideMethods_;
  int id_;
  /* All the entries in the vtable, including from the superclass */
  std::vector<std::string> vTableEntries_;

private:
  bool hasVirtualFns() const noexcept;
  std::string vTableName();

  static int nextId_;
};

}  // namespace language

#endif  // DECL_HPP
