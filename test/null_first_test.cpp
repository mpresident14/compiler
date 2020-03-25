#include "null_first.hpp"
#include "test/nullable_grammar.hpp"

#include <bitset>

#include <prez/print_stuff.hpp>
#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;


UnitTest TESTER = UnitTest::createTester();

void testGetNullabilities() {
  // NULLABLE(S) = false
  // NULLABLE(C) = true
  // NULLABLE(X) = false
  // NULLABLE(Y) = true
  vector<bool> expected = {false, true, false, true};

  TESTER.assertEquals(expected, getNullabilities(GRAMMAR));
}

void testGetFirsts() {
  // FIRST(S) = {t, b, a, z}
  // FIRST(C) = {b, a, z}
  // FIRST(X) = {a}
  // FIRST(Y) = {b, z}
  BitSetToks expectedS = {true, true, true, true};
  BitSetToks expectedC = {false, true, true, true};
  BitSetToks expectedX = {false, false, true, false};
  BitSetToks expectedY = {false, true, false, true};

  vector<BitSetToks> actual = getFirsts(GRAMMAR, 4);
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
