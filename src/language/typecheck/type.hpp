#ifndef TYPE_HPP
#define TYPE_HPP

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>

/********
 * Type *
 ********/

// TODO: Benchmark virtual function call for getType() vs a <BaseClass>Type data
// member

constexpr size_t OBJ_SIZE = 8;

enum class TypeName { INT, BOOL, VOID, ARRAY, CLASS };

struct Type {
  Type(TypeName name) : typeName(name) {}
  TypeName typeName;
};

// TODO: Shared pointer might make more sense, especially when we have more complicated
// types like templates cuz we have a lot of copying
using TypePtr = std::shared_ptr<Type>;

struct Array : public Type {
  Array(TypePtr type)
      : Type{ TypeName::ARRAY }, arrType(type) {}

  TypePtr arrType;
};

struct Class : public Type {
  Class(std::string_view name) : Type{ TypeName::CLASS }, className(name) {}

  std::string className;
};

bool operator==(const Type& t1, const Type& t2) noexcept;
bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept;


std::ostream& operator<<(std::ostream& out, const Type& type);

const TypePtr intType = std::make_shared<Type>(TypeName::INT);
const TypePtr boolType = std::make_shared<Type>(TypeName::BOOL);
const TypePtr voidType = std::make_shared<Type>(TypeName::VOID);

void typeError(const Type& expected, const Type& got, size_t line, Ctx& ctx);


#endif  // TYPE_HPP
