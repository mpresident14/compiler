#include "src/language/typecheck/type.hpp"
#include "src/language/typecheck/context.hpp"

#include <sstream>

using namespace std;

bool operator==(const Type& t1, const Type& t2) noexcept {
  return t1.type == t2.type && t1.className == t2.className;
}

ostream& operator<<(ostream& out, const Type& type) {
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

void typeError(const Type& expected, const Type& got, size_t line) {
  ostringstream& err = ctx::getLogger().logError(line);
  err << "Expected " << expected << ", got " << got;
}
