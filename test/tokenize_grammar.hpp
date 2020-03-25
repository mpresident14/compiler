#ifndef REGEX_GRAMMAR_HPP
#define REGEX_GRAMMAR_HPP

#include <string>
#include <string_view>
#include <cstdlib>
#include <ostream>

enum class Symbol {
  NONE,
  CHAR,
  FOR,
  WHILE,
  INT,
  IDENTIFIER,
};

enum class Concrete { NONE };

inline std::ostream& operator<<(std::ostream& out, const Symbol& sym) {
  switch (sym) {
    case Symbol::NONE:
      out << "NONE";
      break;
    case Symbol::FOR:
      out << "FOR";
      break;
    case Symbol::WHILE:
      out << "WHILE";
      break;
    case Symbol::CHAR:
      out << "CHAR";
      break;
    case Symbol::INT:
      out << "INT";
      break;
    case Symbol::IDENTIFIER:
      out << "IDENTIFIER";
      break;
  }
  return out;
}

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
