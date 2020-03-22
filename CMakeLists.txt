# Note: Used "EXPORT CXX=$(which clang++)" to specify clang compiler
# Create Makefile with "cmake . "
# Then build executables with "make -j <threads>"

# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

# Project's name
project(compiler)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})


# Set compilation flags
set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wextra -pedantic -g -I . -I ./src")

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/test")

add_executable(dfa_test test/dfa_test.cpp)
add_executable(null_first_test test/null_first_test.cpp)
add_executable(lr1_test test/lr1_test.cpp)
add_executable(regex_test test/regex_test.cpp src/regex.cpp src/regex_lexer.cpp src/regex_match.cpp)
add_executable(main src/main.cpp src/regex.cpp src/regex_lexer.cpp)

# https://cmake.org/cmake/help/v3.17/manual/ctest.1.html#description
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/ctest/Testing-With-CTest
enable_testing()
add_test(DFA dfa_test)
add_test(NULL_FIRST null_first_test)
add_test(LR1 lr1_test lr1_test)
add_test(REGEX regex_test regex_test)

# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
add_custom_target(
    check
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS dfa_test null_first_test lr1_test regex_test)
