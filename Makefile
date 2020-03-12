SHELL = /bin/bash

CXX = clang++
CXXFLAGS = -std=c++2a -Wall -Wextra -pedantic -g -I ./src
BINARIES = main rules_test dfa_test
HEADERS = src/dfa.hpp src/rules.hpp

all: $(BINARIES)

clean:
	find . -type f -executable -delete

main: src/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) src/main.cpp -o main

dfa_test: test/dfa_test.cpp src/dfa.hpp
	$(CXX) $(CXXFLAGS) test/dfa_test.cpp -o dfa_test

rules_test: test/rules_test.cpp src/rules.hpp src/dfa.hpp
	$(CXX) $(CXXFLAGS) test/rules_test.cpp -o rules_test
