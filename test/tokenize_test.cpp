#include "test/tokenize_grammar.hpp"
#include "tokenize.hpp"

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest TESTER = UnitTest::createTester();

/* See test/write_dfa.cpp */
void testTokenize() {
  vector<StackObj> actual = tokenize("123for a myVar while");

  TESTER.assertEquals(5, actual.size());

  TESTER.assertEquals(Symbol::INT, actual[0].symbol);
  TESTER.assertEquals(Symbol::FOR, actual[1].symbol);
  TESTER.assertEquals(Symbol::CHAR, actual[2].symbol);
  TESTER.assertEquals(Symbol::IDENTIFIER, actual[3].symbol);
  TESTER.assertEquals(Symbol::WHILE, actual[4].symbol);

  TESTER.assertEquals(123, *(int*) actual[0].obj);
  TESTER.assertEquals(nullptr, actual[1].obj);
  TESTER.assertEquals('a', *(char*) actual[2].obj);
  TESTER.assertEquals("myVar", *(string*) actual[3].obj);
  TESTER.assertEquals(nullptr, actual[4].obj);
}


void testTokenize_withInvalidInput() {
  TESTER.assertThrows([](){ tokenize("123fo$a"); });
}

int main() {
  testTokenize();
  testTokenize_withInvalidInput();

  return 0;
}
