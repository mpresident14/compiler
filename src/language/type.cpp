#include "src/language/type.hpp"

#include "src/language/ctx.hpp"
#include "src/language/utils.hpp"

#include <limits>
#include <sstream>

#include <prez/print_stuff.hpp>

using namespace std;


const TypePtr Type::LONG_TYPE = std::make_shared<Type>(Type::Category::LONG, 8);
const TypePtr Type::INT_TYPE = std::make_shared<Type>(Type::Category::INT, 4);
const TypePtr Type::SHORT_TYPE = std::make_shared<Type>(Type::Category::SHORT, 2);
const TypePtr Type::CHAR_TYPE = std::make_shared<Type>(Type::Category::CHAR, 1);
const TypePtr Type::BOOL_TYPE = std::make_shared<Type>(Type::Category::BOOL, 1);
const TypePtr Type::VOID_TYPE = std::make_shared<Type>(Type::Category::VOID, 0);
const TypePtr Type::STRING_TYPE = std::make_shared<Class>(std::vector<std::string>(), "String");
const TypePtr Type::ANY_TYPE = std::make_shared<Type>(Type::Category::ANY, 0);

const TypePtr Type::LONG_TYPE_FIN = std::make_shared<Type>(Type::Category::LONG, 8, true);
const TypePtr Type::INT_TYPE_FIN = std::make_shared<Type>(Type::Category::INT, 4, true);
const TypePtr Type::SHORT_TYPE_FIN = std::make_shared<Type>(Type::Category::SHORT, 2, true);
const TypePtr Type::CHAR_TYPE_FIN = std::make_shared<Type>(Type::Category::CHAR, 1, true);
const TypePtr Type::BOOL_TYPE_FIN = std::make_shared<Type>(Type::Category::BOOL, 1, true);

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

namespace {
void maybeSetFalse(bool* ptr) {
  if (ptr) {
    *ptr = false;
  }
}
}  // namespace

bool Type::isConvertibleTo(const Type& to, bool* isNarrowing, const Ctx&) const noexcept {
  if (typeName == Type::Category::ANY || to.typeName == Type::Category::ANY) {
    maybeSetFalse(isNarrowing);
    return true;
  }

  if (equalNoAny(*this, to)) {
    maybeSetFalse(isNarrowing);
    return canConvertConst(to);
  }

  if (isIntegral() && to.isIntegral()) {
    if (isNarrowing) {
      if (to.numBytes < numBytes) {
        *isNarrowing = true;
      } else {
        *isNarrowing = false;
      }
    }
    return canConvertConst(to);
  }

  return false;
}

bool Class::isConvertibleTo(const Type& to, bool* isNarrowing, const Ctx& ctx) const noexcept {
  maybeSetFalse(isNarrowing);
  return to.typeName == Type::Category::ANY || (canConvertConst(to) && ctx.isSubClassOf(id, to));
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
      throw invalid_argument("Type::minMaxValue: Not an integral type");
  }
}

TypePtr Type::makeFinal(const TypePtr& t) {
  switch (t->typeName) {
    case Type::Category::LONG:
      return LONG_TYPE_FIN;
    case Type::Category::INT:
      return INT_TYPE_FIN;
    case Type::Category::SHORT:
      return SHORT_TYPE_FIN;
    case Type::Category::CHAR:
      return CHAR_TYPE_FIN;
    case Type::Category::BOOL:
      return BOOL_TYPE_FIN;
    case Type::Category::ARRAY: {
      if (t->isFinal) {
        return t;
      }
      TypePtr tFinal = make_shared<Array>(static_cast<const Array&>(*t));
      tFinal->isFinal = true;
      return tFinal;
    }
    case Type::Category::CLASS: {
      if (t->isFinal) {
        return t;
      }
      TypePtr tFinal = make_shared<Class>(static_cast<const Class&>(*t));
      tFinal->isFinal = true;
      return tFinal;
    }
    default:
      throw invalid_argument("Type::makeFinal");
  }
}

TypePtr Type::makeConst(const TypePtr& t) {
  // For primitive types, final does the same thing as const
  switch (t->typeName) {
    case Type::Category::LONG:
      return LONG_TYPE_FIN;
    case Type::Category::INT:
      return INT_TYPE_FIN;
    case Type::Category::SHORT:
      return SHORT_TYPE_FIN;
    case Type::Category::CHAR:
      return CHAR_TYPE_FIN;
    case Type::Category::BOOL:
      return BOOL_TYPE_FIN;
    case Type::Category::ARRAY: {
      if (t->isConst) {
        return t;
      }
      TypePtr tConst = make_shared<Array>(static_cast<const Array&>(*t));
      tConst->isConst = true;
      return tConst;
    }
    case Type::Category::CLASS: {
      if (t->isConst) {
        return t;
      }
      TypePtr tConst = make_shared<Class>(static_cast<const Class&>(*t));
      tConst->isConst = true;
      return tConst;
    }
    default:
      throw invalid_argument("Type::makeConst");
  }
}

const TypePtr& Type::smallestIntegral(long n) {
  static const TypePtr* constIntTypes[]{ &SHORT_TYPE, &INT_TYPE, &LONG_TYPE };
  for (const TypePtr* type : constIntTypes) {
    auto p = (*type)->minMaxValue();
    if (p.first <= n && n <= p.second) {
      return *type;
    }
  }
  throw invalid_argument("smallestIntegral");
}

bool operator==(const Type& t1, const Type& t2) noexcept {
  return t1.typeName == Type::Category::ANY || t2.typeName == Type::Category::ANY
         || Type::equalNoAny(t1, t2);
}

bool Type::equalNoAny(const Type& t1, const Type& t2) noexcept {
  if (t1.typeName == t2.typeName) {
    bool constOk = t1.isConst == t2.isConst;
    switch (t1.typeName) {
      case Type::Category::ARRAY:
        return constOk
               && static_cast<const Array&>(t1).arrType == static_cast<const Array&>(t2).arrType;
      case Type::Category::CLASS:
        // Everything equals ID_UNKNOWN so we don't give an error explosion for an undefined class
        return constOk
               && (static_cast<const Class&>(t1).id == static_cast<const Class&>(t2).id
                   || static_cast<const Class&>(t1).id == Class::ID_UNKNOWN
                   || static_cast<const Class&>(t2).id == Class::ID_UNKNOWN);
      default:
        // Through we put 'final' inside the Type class because the impl was easier, the keyword
        // really belongs to the variable, so we don't check that the isFinal field
        return constOk;
    }
  }
  return false;
}

bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept { return *t1 == *t2; }

ostream& operator<<(ostream& out, const Type& type) {
  if (type.isFinal) {
    out << "final ";
  }
  if (type.isConst) {
    out << "const ";
  }
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
        out << lang_utils::qualifiedName(
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

void Type::streamParamTypes(const vector<TypePtr>& paramTypes, ostream& err) {
  streamParamTypes(paramTypes.cbegin(), paramTypes.end(), err);
}

void Type::streamParamTypes(
    std::vector<TypePtr>::const_iterator begin,
    std::vector<TypePtr>::const_iterator end,
    std::ostream& err) {
  err << '(';
  auto penult = prev(end);
  if (begin != end) {
    for (auto iter = begin; iter != penult; ++iter) {
      err << **iter << ", ";
    }
    err << **penult;
  }
  err << ')';
}
