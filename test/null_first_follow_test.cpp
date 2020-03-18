#include "grammar1.hpp"
#include "null_first_follow.hpp"

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest tester = UnitTest::createTester();

void testGetNullabilities() {
  // Y = true, X = false, C = true, S = false
  // (bit positions decrease from left to right)
  BitSet expected("1010");
  tester.assertEquals(expected, getNullabilities());
}

int main(int, char**) {
  testGetNullabilities();

  return 0;
}
