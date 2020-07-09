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

/* ANY used to proceed through errors */
// TODO: Nest within Type
enum class TypeName { LONG, INT, SHORT, CHAR, BOOL, VOID, ARRAY, CLASS, ANY };

struct Type {
  virtual ~Type() {}
  constexpr Type(TypeName name, u_char nBytes, bool integral = false)
      : typeName(name), numBytes(nBytes), isIntegral(integral) {}
  virtual std::string getId() const;

  TypeName typeName;
  u_char numBytes;
  bool isIntegral;
};

using TypePtr = std::shared_ptr<Type>;

struct Array : public Type {
  Array(const TypePtr& type);
  virtual std::string getId() const override;

  TypePtr arrType;
};

struct Class : public Type {
  static const int ID_EMPTY = -1;
  static const int ID_UNKNOWN = -2;
  Class(std::vector<std::string>&& quals, std::string_view name);
  virtual std::string getId() const override;

  std::vector<std::string> qualifiers;
  std::string className;
  int id = Class::ID_EMPTY;
};

constexpr bool isIntegral(const Type& t) { return t.isIntegral; }
bool isConvertible(const Type& from, const Type& to, bool* isNarrowing);
std::pair<long, long> minMaxValue(const Type& integralType);
const TypePtr& smallestIntegral(long n);

bool operator==(const Type& t1, const Type& t2) noexcept;
// Because I will inevitably call this one by mistake instead of the one above
bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept;

std::ostream& operator<<(std::ostream& out, const Type& type);

const TypePtr longType = std::make_shared<Type>(TypeName::LONG, 8, true);
const TypePtr intType = std::make_shared<Type>(TypeName::INT, 4, true);
const TypePtr shortType = std::make_shared<Type>(TypeName::SHORT, 2, true);
const TypePtr charType = std::make_shared<Type>(TypeName::CHAR, 1, true);
const TypePtr boolType = std::make_shared<Type>(TypeName::BOOL, 1);
const TypePtr voidType = std::make_shared<Type>(TypeName::VOID, 0);
const TypePtr anyType = std::make_shared<Type>(TypeName::ANY, 0, true);

#endif  // TYPE_HPP
