#include "regex_dfa.hpp"

#include <iostream>
#include <cstddef>
#include <string>

using namespace std;


int main(int, char**) {
  auto root = getRoot();
  cout << root.get() << endl;

  return 0;
}
