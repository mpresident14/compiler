#include "monadic/parser.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#include <prez/print_stuff.hpp>

using namespace std;
using namespace parsers;
using namespace prez;

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "Enter grammar configuration file." << endl;
    return 1;
  }

  std::ifstream file(argv[1]);

  file.close();
  return 0;
}
