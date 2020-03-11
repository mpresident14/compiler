#include "nfa.hpp"

#include <iostream>
#include <cstddef>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;


UnitTest tester = UnitTest::createTester();

void testAddAndRun() {
  NFA<int, char> nfa(1);
  NFA<int, char>::Node* node2 = nfa.getRoot()->addTransition('a', 2);
  NFA<int, char>::Node* node3 = nfa.getRoot()->addTransition('b', 3);
  NFA<int, char>::Node* node4 = node2->addTransition('c', 4);
  NFA<int, char>::Node* node5 = node2->addTransition('d', 4);

  tester.assertEquals(node2, nfa.run({'a'}));
  tester.assertEquals(node3, nfa.run({'b'}));
  tester.assertEquals(node4, nfa.run({'a', 'c'}));
  tester.assertEquals(node5, nfa.run({'a', 'd'}));
  tester.assertEquals(nullptr, nfa.run({'c'}));
  tester.assertEquals(nullptr, nfa.run({'a', 'b'}));
  tester.assertEquals(nullptr, nfa.run({'b', 'c'}));
}

int main(int, char**)
{
  testAddAndRun();

  return 0;
}
