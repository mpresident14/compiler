#include "src/language/typecheck/type.hpp"

#include "src/language/typecheck/context.hpp"
#include "src/language/utils.hpp"

#include <limits>
#include <sstream>

#include <prez/print_stuff.hpp>

using namespace std;

Array::Array(const TypePtr& type) : Type(Type::Category::ARRAY, 8), arrType(type) {}

Class::Class(vector<string>&& quals, string_view name)
    : Type(Type::Category::CLASS, 8), qualifiers(move(quals)), className(name) {}


bool Type::isIntegral() const noexcept {
  switch (typeName) {
    case Type::Category::LONG:
      return true;
    case Type::Category::INT:
      return true;
    case Type::Category::SHORT:
      return true;
    case Type::Category::CHAR:
      return true;
    default:
      return false;
  }
}

bool Type::isConvertibleTo(const Type& to, bool* isNarrowing) const noexcept {
  if (*this == to) {
    if (isNarrowing) {
      *isNarrowing = false;
    }
    return true;
  }

  if (isIntegral() && to.isIntegral()) {
    if (isNarrowing) {
      if (to.numBytes < numBytes) {
        *isNarrowing = true;
      } else {
        *isNarrowing = false;
      }
    }
    return true;
  }

  return false;
}

pair<long, long> Type::minMaxValue() const {
  switch (typeName) {
    case Type::Category::LONG:
      return { numeric_limits<long>::min(), numeric_limits<long>::max() };
    case Type::Category::INT:
      return { numeric_limits<int>::min(), numeric_limits<int>::max() };
    case Type::Category::SHORT:
      return { numeric_limits<short>::min(), numeric_limits<short>::max() };
    case Type::Category::CHAR:
      return { numeric_limits<char>::min(), numeric_limits<char>::max() };
    default:
      throw invalid_argument("minMaxValue: Not an integral type");
  }
}

const TypePtr& Type::smallestIntegral(long n) {
  static const TypePtr* constIntTypes[]{ &shortType, &intType, &longType };
  for (const TypePtr* type : constIntTypes) {
    auto p = (*type)->minMaxValue();
    if (p.first <= n && n <= p.second) {
      return *type;
    }
  }
  throw invalid_argument("smallestIntegral");
}


bool operator==(const Type& t1, const Type& t2) noexcept {
  if (t1.typeName == Type::Category::ANY || t2.typeName == Type::Category::ANY) {
    return true;
  }

  if (t1.typeName == t2.typeName) {
    switch (t1.typeName) {
      case Type::Category::ARRAY:
        return static_cast<const Array&>(t1).arrType == static_cast<const Array&>(t2).arrType;
      case Type::Category::CLASS:
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
    case Type::Category::LONG:
      out << "long";
      break;
    case Type::Category::INT:
      out << "int";
      break;
    case Type::Category::SHORT:
      out << "short";
      break;
    case Type::Category::CHAR:
      out << "char";
      break;
    case Type::Category::BOOL:
      out << "bool";
      break;
    case Type::Category::VOID:
      out << "void";
      break;
    case Type::Category::ARRAY:
      out << *static_cast<const Array&>(type).arrType << "[]";
      break;
    case Type::Category::CLASS:
      if (static_cast<const Class&>(type).id == Class::ID_UNKNOWN) {
        out << "???";
      } else {
        out << qualifiedName(
            static_cast<const Class&>(type).qualifiers, static_cast<const Class&>(type).className);
      }
      break;
    case Type::Category::ANY:
      out << "???";
      break;
    default:
      throw invalid_argument("Type::operator<<");
  }
  return out;
}
