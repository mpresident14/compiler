#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

#include <prez/print_stuff.hpp>

#include <optional>
#include <string_view>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <functional>
#include <algorithm>
#include <cctype>
#include <climits>

constexpr int NONE = INT_MIN;
constexpr int CHAR = -1;
constexpr int FOR = -2;
constexpr int WHILE = -3;
constexpr int INT = -4;
constexpr int IDENTIFIER = -5;

struct StackObj {
  // Can't delete from here since it is a void*, see constructObj
  void* obj;
  int symbol;
};



std::vector<StackObj> tokenize(const std::string& input);
void deleteObj(const StackObj& stackObj) noexcept;



#endif
