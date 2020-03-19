// null_first.hpp relies on a grammar, so we have to include grammar1.hpp before.
// This setup allows us to use arbitrary grammars for null_first without getting
// multiple definitions of Symbol, Concrete, etc.
#include "test/grammar1.hpp"
#include "null_first.hpp"

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest tester = UnitTest::createTester();

void testGetNullabilities() {
  // NULLABLE(Y) = true
  // NULLABLE(X) = false
  // NULLABLE(C) = true
  // NULLABLE(S) = false
  // (bit positions decrease from left to right)
  BitSetVars expected("1010");
  tester.assertEquals(expected, getNullabilities());
}

void testGetFirsts() {
  // FIRST(S) = {Z, A, B, T}
  // FIRST(C) = {Z, A, B}
  // FIRST(X) = {A}
  // FIRST(Y) = {Z, B}
  BitSetToks expectedS("1111");
  BitSetToks expectedC("1110");
  BitSetToks expectedX("0100");
  BitSetToks expectedY("1010");

  vector<BitSetToks> actual = getFirsts();
  tester.assertEquals(expectedS, actual[toInt(Symbol::S)]);
  tester.assertEquals(expectedC, actual[toInt(Symbol::C)]);
  tester.assertEquals(expectedX, actual[toInt(Symbol::X)]);
  tester.assertEquals(expectedY, actual[toInt(Symbol::Y)]);
}

int main() {
  testGetNullabilities();
  testGetFirsts();

  return 0;
}
