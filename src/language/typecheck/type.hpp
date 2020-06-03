#ifndef TYPE_HPP
#define TYPE_HPP

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

/********
 * Type *
 ********/

// TODO: Benchmark virtual function call for getType() vs a <BaseClass>Type data
// member

/* ANY used to proceed through errors */
enum class TypeName { INT, CHAR, BOOL, VOID, ARRAY, CLASS, ANY };

struct Type {
  virtual ~Type() {}
  Type(TypeName name, size_t nBytes) : typeName(name), numBytes(nBytes) {}
  virtual std::string getId(
      const std::unordered_map<std::string, std::string>& typeIds) const;

  TypeName typeName;
  size_t numBytes;
};

// TODO: Shared pointer might make more sense, especially when we have more
// complicated types like templates cuz we have a lot of copying
using TypePtr = std::shared_ptr<Type>;

struct Array : public Type {
  Array(TypePtr type) : Type(TypeName::ARRAY, 8), arrType(type) {}
  virtual std::string getId(const std::unordered_map<std::string, std::string>&
                                typeIds) const override;

  TypePtr arrType;
};

struct Class : public Type {
  Class(std::string_view name) : Type(TypeName::CLASS, 8), className(name) {}
  virtual std::string getId(const std::unordered_map<std::string, std::string>&
                                typeIds) const override;

  std::string className;
};

bool operator==(const Type& t1, const Type& t2) noexcept;
bool operator==(const TypePtr& t1, const TypePtr& t2) noexcept;


std::ostream& operator<<(std::ostream& out, const Type& type);

const TypePtr intType = std::make_shared<Type>(TypeName::INT, 8);
const TypePtr charType = std::make_shared<Type>(TypeName::CHAR, 1);
const TypePtr boolType = std::make_shared<Type>(TypeName::BOOL, 1);
const TypePtr voidType = std::make_shared<Type>(TypeName::VOID, 0);
const TypePtr anyType = std::make_shared<Type>(TypeName::ANY, 0);

#endif  // TYPE_HPP
