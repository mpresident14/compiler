#include "null_first.hpp"
#include "test/nullable_grammar.hpp"

#include <bitset>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

using BitSetVars = bitset<4>;
using BitSetToks = bitset<4>;

UnitTest TESTER = UnitTest::createTester();

void testGetNullabilities() {
  // NULLABLE(Y) = true
  // NULLABLE(X) = false
  // NULLABLE(C) = true
  // NULLABLE(S) = false
  // (bit positions decrease from left to right)
  BitSetVars expected("1010");
  TESTER.assertEquals(expected, getNullabilities<4>(GRAMMAR));
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

  vector<BitSetToks> actual = getFirsts<4, 4>(GRAMMAR);
  TESTER.assertEquals(expectedS, actual[S]);
  TESTER.assertEquals(expectedC, actual[C]);
  TESTER.assertEquals(expectedX, actual[X]);
  TESTER.assertEquals(expectedY, actual[Y]);
}

int main() {
  testGetNullabilities();
  testGetFirsts();

  return 0;
}
