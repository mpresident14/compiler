#ifndef TYPE_HPP
#define TYPE_HPP

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>

/********
 * Type *
 ********/

// TODO: Benchmark virtual function call for getType() vs a <BaseClass>Type data member

constexpr size_t OBJ_SIZE = 8;

enum class TypeName { INT, BOOL, VOID, ARRAY, CLASS };

struct Type {
  TypeName typeName;
};

struct Array : public Type {
  constexpr Array(const Type& type) : Type{TypeName::ARRAY}, arrType(type) {}
  Type arrType;
};

struct Class : public Type {
  Class(string_view name) : Type{TypeName::CLASS}, className(name) {}
  std::string className;
};

bool operator==(const Type& t1, const Type& t2) noexcept;
bool operator==(const Type& t1, const Type& t2) noexcept;
bool operator==(const Type& t1, const Type& t2) noexcept;

std::ostream& operator<<(std::ostream& out, const Type& type);

const Type intType = { TypeName::INT };
const Type boolType = { TypeName::BOOL };
const Type voidType = { TypeName::VOID };

void typeError(const Type& expected, const Type& got, size_t line);


#endif  // TYPE_HPP
