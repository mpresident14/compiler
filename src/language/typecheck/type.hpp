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
  static const TypePtr LONG_TYPE;
  static const TypePtr INT_TYPE;
  static const TypePtr SHORT_TYPE;
  static const TypePtr CHAR_TYPE;
  static const TypePtr BOOL_TYPE;
  static const TypePtr VOID_TYPE;
  static const TypePtr STRING_TYPE;
  static const TypePtr ANY_TYPE;

  /* ANY used to proceed through errors */
  enum class Category { LONG, INT, SHORT, CHAR, BOOL, VOID, ARRAY, CLASS, ANY };

  /* Determines the smallest integral type (excluding CHAR) that the arg fits in */
  static const TypePtr& smallestIntegral(long n);

  virtual ~Type() {}
  constexpr Type(Category name, u_char nBytes) : typeName(name), numBytes(nBytes) {}
  bool isIntegral() const noexcept;
  virtual bool isConvertibleTo(const Type& to, bool* isNarrowing, const Ctx& ctx) const noexcept;
  std::pair<long, long> minMaxValue() const;

  Category typeName;
  u_char numBytes;
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
  virtual bool isConvertibleTo(const Type& to, bool* isNarrowing, const Ctx& ctx) const noexcept override;

  std::vector<std::string> qualifiers;
  std::string className;
  int id = Class::ID_EMPTY;
};


bool operator==(const Type& t1, const Type& t2) noexcept;
// Because I will inevitably call this one by mistake instead of the one above
bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept;

std::ostream& operator<<(std::ostream& out, const Type& type);

#endif  // TYPE_HPP
