SHELL = /bin/bash

CXX = clang++
CXXFLAGS = -std=c++2a -Wall -Wextra -pedantic -g -I ./src
BINARIES = main lr0_test dfa_test
HEADERS = src/*.hpp src/monadic/*.hpp


all: $(BINARIES)

clean:
	find . -type f -executable -delete

runtests:
	./dfa_test
	./lr0_test

main: src/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) src/main.cpp -o main

dfa_test: test/dfa_test.cpp src/dfa.hpp
	$(CXX) $(CXXFLAGS) test/dfa_test.cpp -o dfa_test

lr0_test: test/lr0_test.cpp src/lr0*.hpp src/parse.hpp src/dfa.hpp
	$(CXX) $(CXXFLAGS) test/lr0_test.cpp -o lr0_test

generate: src/generate.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) src/generate.cpp -o generate
