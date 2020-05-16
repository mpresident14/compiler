#ifndef TYPE_HPP
#define TYPE_HPP

#include <memory>

/********
 * Type *
 ********/

enum class TypeType { INT, BOOL, VOID };

// TODO: Just use strings?
class Type {
public:
  virtual ~Type() {};
  virtual constexpr TypeType getType() const noexcept = 0;
};

class IntTy : public Type {
public:
  constexpr TypeType getType() const noexcept { return TypeType::INT; }
};

class BoolTy : public Type {
public:
  constexpr TypeType getType() const noexcept { return TypeType::BOOL; }
};

class VoidTy : public Type {
public:
  constexpr TypeType getType() const noexcept { return TypeType::VOID; }
};


using TypePtr = std::unique_ptr<Type>;


#endif // TYPE_HPP
