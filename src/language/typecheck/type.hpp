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

class Type;
using TypePtr = std::shared_ptr<Type>;

class Type {
public:
  /* ANY used to proceed through errors */
  enum class Category { LONG, INT, SHORT, CHAR, BOOL, VOID, ARRAY, CLASS, ANY };

  /* Determines the smallest integral type (excluding CHAR) that the arg fits in */
  static const TypePtr& smallestIntegral(long n);

  virtual ~Type() {}
  constexpr Type(Category name, u_char nBytes) : typeName(name), numBytes(nBytes) {}
  bool isIntegral() const noexcept;
  bool isConvertibleTo(const Type& to, bool* isNarrowing) const noexcept;
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

  std::vector<std::string> qualifiers;
  std::string className;
  int id = Class::ID_EMPTY;
};


bool operator==(const Type& t1, const Type& t2) noexcept;
// Because I will inevitably call this one by mistake instead of the one above
bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept;

std::ostream& operator<<(std::ostream& out, const Type& type);

const TypePtr longType = std::make_shared<Type>(Type::Category::LONG, 8);
const TypePtr intType = std::make_shared<Type>(Type::Category::INT, 4);
const TypePtr shortType = std::make_shared<Type>(Type::Category::SHORT, 2);
const TypePtr charType = std::make_shared<Type>(Type::Category::CHAR, 1);
const TypePtr boolType = std::make_shared<Type>(Type::Category::BOOL, 1);
const TypePtr voidType = std::make_shared<Type>(Type::Category::VOID, 0);
const TypePtr strType = std::make_shared<Class>(std::vector<std::string>(), "String");
const TypePtr anyType = std::make_shared<Type>(Type::Category::ANY, 0);

#endif  // TYPE_HPP
