#ifndef TYPE_HPP
#define TYPE_HPP

#include <memory>

/********
 * Type *
 ********/

enum class TypeName { INT, BOOL, VOID, CLASS };

struct Type {
public:
  TypeName type;
  std::string className;
};

inline bool operator==(const Type& t1, const Type& t2) noexcept {
  return t1.type == t2.type && t1.className == t2.className;
}


const Type intType = {TypeName::INT, ""};
const Type boolType = {TypeName::BOOL, ""};
const Type voidType = {TypeName::VOID, ""};


#endif // TYPE_HPP
