#include "NFA.hpp"

#include <iostream>
#include <cstddef>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;


UnitTest tester = UnitTest::createTester();

void testAddAndRun() {
  NFA<int, char> nfa(1);
  NFA<int, char>::Node* node2 = nfa.addTransition(nfa.getRoot(), 2, 'a');
  NFA<int, char>::Node* node3 = nfa.addTransition(nfa.getRoot(), 3, 'b');
  NFA<int, char>::Node* node4 = nfa.addTransition(node2, 4, 'c');
  NFA<int, char>::Node* node5 = nfa.addTransition(node2, 5, 'd');

  tester.assertEquals(nfa.run({'a'}), node2);
  tester.assertEquals(nfa.run({'b'}), node3);
  tester.assertEquals(nfa.run({'a', 'c'}), node4);
  tester.assertEquals(nfa.run({'a', 'd'}), node5);
  tester.assertEquals(nfa.run({'c'}), nullptr);
  tester.assertEquals(nfa.run({'a', 'b'}), nullptr);
  tester.assertEquals(nfa.run({'b', 'c'}), nullptr);
}

int main(int, char**)
{
  testAddAndRun();

  return 0;
}
