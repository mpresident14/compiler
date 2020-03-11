SHELL = /bin/bash

CXX = clang++
CXXFLAGS = -std=c++2a -Wall -Wextra -pedantic -g -I ./src
BINARIES = main test

all: $(BINARIES)

clean:
	find . -type f -executable -delete

main: src/main.cpp src/NFA.hpp
	$(CXX) $(CXXFLAGS) src/main.cpp -o main

test: test/NFATest.cpp src/NFA.hpp
	$(CXX) $(CXXFLAGS) test/NFATest.cpp -o NFATest
