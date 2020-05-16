#ifndef TYPE_HPP
#define TYPE_HPP

#include <memory>

/********
 * Type *
 ********/

enum class TypeName { INT, BOOL, VOID };

// TODO: Find a different representation for types so we don't have to keep
// creating pointers for built-in types

class Type {
public:
  virtual ~Type() {};
  virtual constexpr TypeName getType() const noexcept = 0;
};

class IntTy : public Type {
public:
  constexpr TypeName getType() const noexcept { return TypeName::INT; }
};

class BoolTy : public Type {
public:
  constexpr TypeName getType() const noexcept { return TypeName::BOOL; }
};

class VoidTy : public Type {
public:
  constexpr TypeName getType() const noexcept { return TypeName::VOID; }
};


using TypePtr = std::unique_ptr<Type>;


#endif // TYPE_HPP
