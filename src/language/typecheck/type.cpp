#include "src/language/typecheck/type.hpp"
#include "src/language/typecheck/context.hpp"

#include <sstream>

using namespace std;

bool operator==(const Type& t1, const Type& t2) noexcept {
  if (t1.typeName == t2.typeName) {
    switch (t1.typeName) {
      case TypeName::ARRAY:
        return static_cast<const Array&>(t1).arrType == static_cast<const Array&>(t2).arrType;
      case TypeName::CLASS:
        return static_cast<const Class&>(t1).className == static_cast<const Class&>(t2).className;
      default:
        return true;
    }
  }
  return false;
}

bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept {
  return *t1 == *t2;
}

ostream& operator<<(ostream& out, const Type& type) {
  switch (type.typeName) {
    case TypeName::INT:
      out << "int";
      break;
    case TypeName::BOOL:
      out << "bool";
      break;
    case TypeName::VOID:
      out << "void";
      break;
    case TypeName::ARRAY:
      out << *static_cast<const Array&>(type).arrType << "[]";
      break;
    case TypeName::CLASS:
      out << static_cast<const Class&>(type).className;
      break;
    default: throw invalid_argument("Type::operator<<");
  }
  return out;
}

void typeError(const Type& expected, const Type& got, size_t line) {
  ostringstream& err = ctx::getLogger().logError(line);
  err << "Expected " << expected << ", got " << got;
}
