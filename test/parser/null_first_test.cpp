#include "src/parser/null_first.hpp"

#include "test/parser/utils/nullable_grammar.hpp"

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
  vector<bool> expected = { false, true, false, true };

  TESTER.assertEquals(expected, getNullabilities(GRAMMAR_DATA));
}

void testGetFirsts() {
  // FIRST(S) = {t, b, a, z}
  // FIRST(C) = {b, a, z}
  // FIRST(X) = {a}
  // FIRST(Y) = {b, z}
  vector<bool> expectedS = { true, true, true, true };
  vector<bool> expectedC = { false, true, true, true };
  vector<bool> expectedX = { false, false, true, false };
  vector<bool> expectedY = { false, true, false, true };

  vector<vector<bool>> actual = getFirsts(GRAMMAR_DATA);
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
