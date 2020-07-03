#include "src/language/typecheck/type.hpp"

#include "src/language/typecheck/context.hpp"

#include <limits>
#include <sstream>

#include <boost/algorithm/string/split.hpp>

using namespace std;

Array::Array(TypePtr type) : Type(TypeName::ARRAY, 8), arrType(type) {}

// TODO: Reuse from ctx_tree.cpp
namespace {

vector<string> splitPath(string_view importPath) {
  importPath = importPath.substr(0, importPath.size() - sizeof(".prez") + 1);
  vector<string> pathParts;
  boost::split(pathParts, importPath, [](char c) { return c == '/'; });
  return pathParts;
}

}  // namespace

Class::Class(string_view name, string_view declFile)
    : Type(TypeName::CLASS, 8), fullQuals(splitPath(declFile)), className(name) {}


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

TypePtr smallestIntegral(long n) {
  static TypePtr constIntTypes[]{ shortType, intType, longType };
  for (const TypePtr& type : constIntTypes) {
    auto p = minMaxValue(*type);
    if (p.first <= n && n <= p.second) {
      return type;
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
        return static_cast<const Class&>(t1).className == static_cast<const Class&>(t2).className;
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
      out << static_cast<const Class&>(type).className;
      break;
    case TypeName::ANY:
      out << "???";
      break;
    default:
      throw invalid_argument("Type::operator<<");
  }
  return out;
}
