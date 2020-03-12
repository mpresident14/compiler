#include "dfa.hpp"

#include <cstddef>
#include <iostream>
#include <string>

#include <prez/unit_test.hpp>

using namespace std;
using namespace prez;

UnitTest tester = UnitTest::createTester();

void testAddAndRun() {
  DFA<int, char> dfa(1);
  const DFA<int, char>::Node* node2 = dfa.addTransition(dfa.getRoot(), 'a', 2);
  const DFA<int, char>::Node* node3 = dfa.addTransition(dfa.getRoot(), 'b', 3);
  const DFA<int, char>::Node* node4 = dfa.addTransition(node2, 'c', 4);
  const DFA<int, char>::Node* node5 = dfa.addTransition(node2, 'd', 5);
  // Node value already present, should add transition, but not create a new node
  const DFA<int, char>::Node* node6 = dfa.addTransition(node3, 'e', 5);
  // Duplicate transition, should ignore.
  const DFA<int, char>::Node* node7 = dfa.addTransition(node3, 'e', 2);

  tester.assertEquals(node2, dfa.run({'a'}));
  tester.assertEquals(node3, dfa.run({'b'}));
  tester.assertEquals(node4, dfa.run({'a', 'c'}));
  tester.assertEquals(node5, dfa.run({'a', 'd'}));
  tester.assertEquals(nullptr, dfa.run({'c'}));
  tester.assertEquals(nullptr, dfa.run({'a', 'b'}));
  tester.assertEquals(node5, dfa.run({'b', 'e'}));
  tester.assertEquals(nullptr, node6);
  tester.assertEquals(nullptr, node7);
}

int main(int, char**) {
  testAddAndRun();

  return 0;
}
