#ifndef TYPE_HPP
#define TYPE_HPP

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

/********
 * Type *
 ********/

class Ctx;
class Type;
using TypePtr = std::shared_ptr<Type>;

class Type {
public:
  enum class Category { LONG, INT, SHORT, CHAR, BOOL, VOID, ARRAY, CLASS, ANY };

  static const TypePtr LONG_TYPE;
  static const TypePtr INT_TYPE;
  static const TypePtr SHORT_TYPE;
  static const TypePtr CHAR_TYPE;
  static const TypePtr BOOL_TYPE;
  static const TypePtr VOID_TYPE;
  static const TypePtr STRING_TYPE;
  /* ANY used to proceed through errors */
  static const TypePtr ANY_TYPE;

  static const TypePtr LONG_TYPE_FIN;
  static const TypePtr INT_TYPE_FIN;
  static const TypePtr SHORT_TYPE_FIN;
  static const TypePtr CHAR_TYPE_FIN;
  static const TypePtr BOOL_TYPE_FIN;

  /* Same as operator== except that ANY is not equal to everything */
  static bool equalNoAny(const Type& t1, const Type& t2) noexcept;
  /* Determines the smallest integral type (excluding CHAR) that the arg fits in */
  static const TypePtr& smallestIntegral(long n);
  static TypePtr makeFinal(const TypePtr& t);
  static TypePtr makeConst(const TypePtr& t);

  virtual ~Type() {}
  constexpr Type(Category name, u_char nBytes, bool aIsFinal = false)
      : typeName(name), numBytes(nBytes), isFinal(aIsFinal) {}
  bool isIntegral() const noexcept;
  virtual bool isConvertibleTo(const Type& to, bool* isNarrowing, const Ctx& ctx) const noexcept;
  std::pair<long, long> minMaxValue() const;

  Category typeName;
  u_char numBytes;
  bool isFinal = false;
  bool isConst = false;

protected:
  constexpr bool canConvertConst(const Type& to) const noexcept {
    return !isConst || to.isConst;
  }
};


class Array : public Type {
public:
  Array(const TypePtr& type);

  TypePtr arrType;
};

class Class : public Type {
public:
  static const int ID_EMPTY = -1;
  static const int ID_UNKNOWN = -2;
  Class(std::vector<std::string>&& quals, std::string_view name);
  virtual bool isConvertibleTo(const Type& to, bool* isNarrowing, const Ctx& ctx) const
      noexcept override;

  std::vector<std::string> qualifiers;
  std::string className;
  int id = Class::ID_EMPTY;
};

bool operator==(const Type& t1, const Type& t2) noexcept;
bool operator==(const Type& t1, const Type& t2) noexcept;
// Because I will inevitably call this one by mistake instead of the one above
bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept;

std::ostream& operator<<(std::ostream& out, const Type& type);

#endif  // TYPE_HPP
