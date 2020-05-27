#include "src/language/typecheck/type.hpp"


bool operator==(const Type& t1, const Type& t2) noexcept {
  return t1.type == t2.type && t1.className == t2.className;
}

std::ostream& operator<<(std::ostream& out, Type type) {
  switch (type.type) {
    case TypeName::INT:
      return out << "int";
    case TypeName::BOOL:
      return out << "bool";
    case TypeName::VOID:
      return out << "void";
    default:
      return out << type.className;
  }
  return out;
}

// TODO: Store all type errors in global stream
void typeError(const std::string& msg) {
  throw std::runtime_error(msg.c_str());
}
