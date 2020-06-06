#include "src/language/typecheck/type.hpp"

#include "src/language/typecheck/context.hpp"

#include <limits>
#include <sstream>

using namespace std;

string Type::getId(const unordered_map<string, string>&) const {
  switch (typeName) {
    case TypeName::LONG:
      return "l";
    case TypeName::INT:
      return "i";
    case TypeName::SHORT:
      return "s";
    case TypeName::CHAR:
      return "c";
    case TypeName::BOOL:
      return "b";
    case TypeName::VOID:
      return "v";
    case TypeName::ANY:
      return "x";
    default:
      throw invalid_argument("Type::getId");
  }
}

string Array::getId(const unordered_map<string, string>& typeIds) const {
  return string("a_").append(arrType->getId(typeIds));
}

string Class::getId(const unordered_map<string, string>& typeIds) const {
  return typeIds.at(className);
}


bool isConvertible(const Type& from, const Type& to, bool* isNarrowing) {
  if (isIntegral(from) && isIntegral(to)) {
    // TODO: We don't want to report narrowing for short n = 3;
    // Check if the expression is a const, and then if the number fits in the
    // specified number of bytes
    if (isNarrowing && to.numBytes < from.numBytes) {
      *isNarrowing = true;
    }
    return true;
  }
  return false;
}

pair<long, long> minMaxValue(const Type& integralType) {
  switch (integralType.typeName) {
    case TypeName::LONG:
      return { std::numeric_limits<long>::min(),
               std::numeric_limits<long>::max() };
    case TypeName::INT:
      return { std::numeric_limits<int>::min(),
               std::numeric_limits<int>::max() };
    case TypeName::SHORT:
      return { std::numeric_limits<short>::min(),
               std::numeric_limits<short>::max() };
    case TypeName::CHAR:
      return { std::numeric_limits<char>::min(),
               std::numeric_limits<char>::max() };
    default:
      throw invalid_argument("minMaxValue: Not an integral type");
  }
}

bool operator==(const Type& t1, const Type& t2) noexcept {
  if (t1.typeName == TypeName::ANY || t2.typeName == TypeName::ANY) {
    return true;
  }

  if (t1.typeName == t2.typeName) {
    switch (t1.typeName) {
      case TypeName::ARRAY:
        return static_cast<const Array&>(t1).arrType ==
               static_cast<const Array&>(t2).arrType;
      case TypeName::CLASS:
        return static_cast<const Class&>(t1).className ==
               static_cast<const Class&>(t2).className;
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
    case TypeName::LONG:
      out << "long";
      break;
    case TypeName::INT:
      out << "int";
      break;
    case TypeName::SHORT:
      out << "short";
      break;
    case TypeName::CHAR:
      out << "char";
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
    case TypeName::ANY:
      out << "any";
      break;
    default:
      throw invalid_argument("Type::operator<<");
  }
  return out;
}
