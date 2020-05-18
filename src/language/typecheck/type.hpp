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

inline bool operator==(const Type& t1, const Type& t2) noexcept {
  return t1.type == t2.type && t1.className == t2.className;
}

inline std::ostream& operator<<(std::ostream& out, Type type) {
  switch (type.type) {
    case TypeName::INT: return out << "int";
    case TypeName::BOOL: return out << "bool";
    case TypeName::VOID: return  out << "void";
    default: return out << type.className;
  }
  return out;
}


const Type intType = { TypeName::INT, "" };
const Type boolType = { TypeName::BOOL, "" };
const Type voidType = { TypeName::VOID, "" };

// TODO: Implement
inline void typeError(const std::string& msg) {
  throw std::runtime_error(msg.c_str());
}

#endif  // TYPE_HPP
