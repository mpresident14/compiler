#include "parse.hpp"
#include "monadic/parser.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;
using namespace parsers;

int main() {
  vector<string> strings;
  word.consume([&strings](string&& str) mutable { strings.push_back(move(str)); }).parse("hello");
  cout << strings << endl;
  return 0;
}
