SHELL = /bin/bash

CXX = clang++
CXXFLAGS = -std=c++2a -Wall -Wextra -pedantic -g -I ./src
BINARIES = main rules_test nfa_test
HEADERS = src/nfa.hpp src/rules.hpp

all: $(BINARIES)

clean:
	find . -type f -executable -delete

main: src/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) src/main.cpp -o main

nfa_test: test/nfa_test.cpp src/nfa.hpp
	$(CXX) $(CXXFLAGS) test/nfa_test.cpp -o nfa_test

rules_test: test/rules_test.cpp src/rules.hpp src/nfa.hpp
	$(CXX) $(CXXFLAGS) test/rules_test.cpp -o rules_test
