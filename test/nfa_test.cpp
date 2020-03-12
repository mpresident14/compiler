#include "nfa.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest tester = UnitTest::createTester();

void testAddAndRun() {
  NFA<int, char> nfa(1);
  const NFA<int, char>::Node* node2 = nfa.addTransition(nfa.getRoot(), 'a', 2);
  const NFA<int, char>::Node* node3 = nfa.addTransition(nfa.getRoot(), 'b', 3);
  const NFA<int, char>::Node* node4 = nfa.addTransition(node2, 'c', 4);
  const NFA<int, char>::Node* node5 = nfa.addTransition(node2, 'd', 5);
  // Node value already present, should add transition, but not create a new node
  const NFA<int, char>::Node* node6 = nfa.addTransition(node3, 'e', 5);
  // Duplicate transition, should ignore.
  const NFA<int, char>::Node* node7 = nfa.addTransition(node3, 'e', 2);

  tester.assertEquals(node2, nfa.run({'a'}));
  tester.assertEquals(node3, nfa.run({'b'}));
  tester.assertEquals(node4, nfa.run({'a', 'c'}));
  tester.assertEquals(node5, nfa.run({'a', 'd'}));
  tester.assertEquals(nullptr, nfa.run({'c'}));
  tester.assertEquals(nullptr, nfa.run({'a', 'b'}));
  tester.assertEquals(node5, nfa.run({'b', 'e'}));
  tester.assertEquals(nullptr, node6);
  tester.assertEquals(nullptr, node7);
}

int main(int, char**) {
  testAddAndRun();

  return 0;
}
