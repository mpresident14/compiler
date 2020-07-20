#include "src/language/typecheck/type.hpp"

#include "src/language/typecheck/context.hpp"
#include "src/language/utils.hpp"

#include <limits>
#include <sstream>

#include <prez/print_stuff.hpp>

using namespace std;

Array::Array(const TypePtr& type) : Type(TypeName::ARRAY, 8), arrType(type) {}

Class::Class(vector<string>&& quals, string_view name)
    : Type(TypeName::CLASS, 8), qualifiers(move(quals)), className(name) {}


bool isConvertible(const Type& from, const Type& to, bool* isNarrowing) {
  if (from == to) {
    if (isNarrowing) {
      *isNarrowing = false;
    }
    return true;
  }

  if (isIntegral(from) && isIntegral(to)) {
    if (isNarrowing) {
      if (to.numBytes < from.numBytes) {
        *isNarrowing = true;
      } else {
        *isNarrowing = false;
      }
    }
    return true;
  }

  return false;
}

pair<long, long> minMaxValue(const Type& integralType) {
  switch (integralType.typeName) {
    case TypeName::LONG:
      return { numeric_limits<long>::min(), numeric_limits<long>::max() };
    case TypeName::INT:
      return { numeric_limits<int>::min(), numeric_limits<int>::max() };
    case TypeName::SHORT:
      return { numeric_limits<short>::min(), numeric_limits<short>::max() };
    case TypeName::CHAR:
      return { numeric_limits<char>::min(), numeric_limits<char>::max() };
    default:
      throw invalid_argument("minMaxValue: Not an integral type");
  }
}

const TypePtr& smallestIntegral(long n) {
  static const TypePtr* constIntTypes[]{ &shortType, &intType, &longType };
  for (const TypePtr* type : constIntTypes) {
    auto p = minMaxValue(**type);
    if (p.first <= n && n <= p.second) {
      return *type;
    }
  }
  throw invalid_argument("smallestIntegral");
}


bool operator==(const Type& t1, const Type& t2) noexcept {
  if (t1.typeName == TypeName::ANY || t2.typeName == TypeName::ANY) {
    return true;
  }

  if (t1.typeName == t2.typeName) {
    switch (t1.typeName) {
      case TypeName::ARRAY:
        return static_cast<const Array&>(t1).arrType == static_cast<const Array&>(t2).arrType;
      case TypeName::CLASS:
      // Everything equals ID_UNKNOWN so we don't give an error explosion for an undefined class
        return static_cast<const Class&>(t1).id == static_cast<const Class&>(t2).id ||
               static_cast<const Class&>(t1).id == Class::ID_UNKNOWN ||
               static_cast<const Class&>(t2).id == Class::ID_UNKNOWN;
      default:
        return true;
    }
  }
  return false;
}

bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept { return *t1 == *t2; }

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
      if (static_cast<const Class&>(type).id == Class::ID_UNKNOWN) {
        out << "???";
      } else {
        out << qualifiedName(
            static_cast<const Class&>(type).qualifiers, static_cast<const Class&>(type).className);
      }
      break;
    case TypeName::ANY:
      out << "???";
      break;
    default:
      throw invalid_argument("Type::operator<<");
  }
  return out;
}
