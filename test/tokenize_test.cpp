#include "test/generated/random_lexer.hpp"

#include <string>
#include <vector>

#include <prez/unit_test.hpp>
#include <prez/print_stuff.hpp>

using namespace std;
using namespace prez;
using namespace test::generated::random_lexer;

UnitTest TESTER = UnitTest::createTester();

constexpr static int DNOTABC = -1;
constexpr static int FOR = -2;
constexpr static int HabLLO = -3;
constexpr static int HDOTLLO = -4;
constexpr static int H_LLO = -5;
constexpr static int STRLIT = -6;
constexpr static int NUMBER = -7;
constexpr static int IDENT = -8;

void testGeneralTokenization() {
  vector<StackObj> tokens = tokenize(R"( "Hello" "Hi \"Bob\"" 123 abc123 de h]llo )");

  StackObj t0 = tokens[0];
  TESTER.assertEquals(STRLIT, t0.symbol);
  if (t0.symbol == STRLIT) {
    TESTER.assertEquals("Hello", *(string*) t0.obj);
  }

  StackObj t1 = tokens[1];
  TESTER.assertEquals(STRLIT, t1.symbol);
  if (t1.symbol == STRLIT) {
    TESTER.assertEquals(R"(Hi \"Bob\")", *(string*) t1.obj);
  }

  StackObj t2 = tokens[2];
  TESTER.assertEquals(NUMBER, t2.symbol);
  if (t2.symbol == NUMBER) {
    TESTER.assertEquals(123, *(int*) t2.obj);
  }

  StackObj t3 = tokens[3];
  TESTER.assertEquals(IDENT, t3.symbol);
  if (t3.symbol == IDENT) {
    TESTER.assertEquals("abc123", *(string*) t3.obj);
  }

  StackObj t4 = tokens[4];
  TESTER.assertEquals(DNOTABC, t4.symbol);
  if (t4.symbol == DNOTABC) {
    TESTER.assertEquals('e', **(char**) t4.obj);
  }

  TESTER.assertEquals(HabLLO, tokens[5].symbol);

  for_each(tokens.cbegin(), tokens.cend(), deleteObj);
}

void testFirstComeFirstServed() {
  vector<StackObj> tokens = tokenize("for h.llo hello hullo hollow");

  TESTER.assertEquals(FOR, tokens[0].symbol);
  TESTER.assertEquals(HDOTLLO, tokens[1].symbol);
  TESTER.assertEquals(H_LLO, tokens[2].symbol);
  TESTER.assertEquals(H_LLO, tokens[3].symbol);
  StackObj t4 = tokens[4];
  TESTER.assertEquals(IDENT, t4.symbol);
  if (tokens[4].symbol == IDENT) {
    TESTER.assertEquals("hollow", *(string*) t4.obj);
  }

  for_each(tokens.cbegin(), tokens.cend(), deleteObj);
}

void testMatchLongest() {
  vector<StackObj> tokens = tokenize("forbid digit");

  StackObj t0 = tokens[0];
  TESTER.assertEquals(IDENT, t0.symbol);
  if (t0.symbol == IDENT) {
    TESTER.assertEquals("forbid", *(string*) t0.obj);
  }
  StackObj t1 = tokens[1];
  TESTER.assertEquals(IDENT, t1.symbol);
  if (t1.symbol == IDENT) {
    TESTER.assertEquals("digit", *(string*) t1.obj);
  }

  for_each(tokens.cbegin(), tokens.cend(), deleteObj);
}

void testNoLeaksOnError() {
  string err = TESTER.assertThrows([](){ tokenize("dg {}"); });
  TESTER.assertNotEqual(string::npos, err.find("NOTABC"));
}

int main() {
  testGeneralTokenization();
  testFirstComeFirstServed();
  testMatchLongest();
  testNoLeaksOnError();

  return 0;
}
