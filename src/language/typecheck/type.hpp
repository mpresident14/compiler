#ifndef TYPE_HPP
#define TYPE_HPP

#include <memory>
#include <stdexcept>
#include <iostream>

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

std::ostream& operator<<(std::ostream& out, Type type);


const Type intType = { TypeName::INT, "" };
const Type boolType = { TypeName::BOOL, "" };
const Type voidType = { TypeName::VOID, "" };

// TODO: Store all type errors in global stream
void typeError(const std::string& msg);

#endif  // TYPE_HPP
