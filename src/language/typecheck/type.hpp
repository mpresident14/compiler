#ifndef TYPE_HPP
#define TYPE_HPP

#include <iostream>
#include <memory>
#include <stdexcept>

/********
 * Type *
 ********/

enum class TypeName { INT, BOOL, VOID, CLASS };

struct Type {
public:
  TypeName type;
  std::string className;
};

bool operator==(const Type& t1, const Type& t2) noexcept;

std::ostream& operator<<(std::ostream& out, const Type& type);

const Type intType = { TypeName::INT, "" };
const Type boolType = { TypeName::BOOL, "" };
const Type voidType = { TypeName::VOID, "" };

void typeError(const Type& expected, const Type& got, size_t line);

#endif  // TYPE_HPP
