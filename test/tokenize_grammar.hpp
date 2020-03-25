#ifndef REGEX_GRAMMAR_HPP
#define REGEX_GRAMMAR_HPP

#include <string>
#include <string_view>
#include <cstdlib>


enum class Symbol {
  CHAR,
  INT,
  IDENTIFIER,
  DATALESS
};

enum class Concrete { NONE };

/*****************************
 *   CONSTRUCTION/DELETION   *
 *****************************/

struct StackObj {
  // Can't delete from here since it is a void*, see constructObj
  void* obj;
  Symbol symbol;
  Concrete type;

  void deleteObj() const noexcept;
};


inline void StackObj::deleteObj() const noexcept {
  switch (symbol) {
    case Symbol::INT:
      delete (int*)obj;
      break;
    case Symbol::CHAR:
      delete (char*)obj;
      break;
    case Symbol::IDENTIFIER:
      delete (std::string*)obj;
      break;
    default:
      return;
  }
}

inline StackObj constructTokenObj(Symbol token, const std::string_view& str) {
  switch (token) {
    case Symbol::INT:
      return { new int(atoi(str.data())), token, Concrete::NONE };
      break;
    case Symbol::CHAR:
      return { new char(str[0]), token, Concrete::NONE };
    case Symbol::IDENTIFIER:
      return { new std::string(str), token, Concrete::NONE };
    default:
      return { nullptr, token, Concrete::NONE };
  }
}


#endif
