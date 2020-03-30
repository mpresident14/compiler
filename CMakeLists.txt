# Note: Used "EXPORT CXX=$(which clang++)" to specify clang compiler
# Create Makefile with "cmake . "
# Then build executables with "make -j <threads>"

# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.17.0)

# Project's name
project(compiler)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Set compilation flags
set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wextra -pedantic -g -I . -I ./src")

# The following folders will be included
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/test)


set(PARSER_SRCS src/build_parser.cpp src/null_first.cpp)
set(REGEX_SRCS src/regex.cpp src/regex_parse.cpp src/regex_eval.cpp)
# So regex sources don't have to be rebuilt for each executable
add_library(parse_lib STATIC ${REGEX_SRCS} ${PARSER_SRCS})

# Test executables
add_executable(dfa_test test/dfa_test.cpp)
add_executable(null_first_test test/null_first_test.cpp src/null_first.cpp)
add_executable(build_parser_test test/build_parser_test.cpp ${PARSER_SRCS})
add_executable(regex_test test/regex_test.cpp)
target_link_libraries(regex_test parse_lib)
add_executable(tokenize_test_exe test/tokenize_test.cpp test/tokenize.cpp)

# Other executables
add_executable(write_lexer test/write_lexer.cpp)
target_link_libraries(write_lexer parse_lib)
add_executable(generate src/generate.cpp)
target_link_libraries(generate parse_lib)

# Ensure that we write the regex DFA before building the corresponding test
add_custom_command(OUTPUT test/gen_lexer.hpp COMMAND write_lexer DEPENDS write_lexer)
add_custom_target(tokenize_test COMMAND tokenize_test_exe DEPENDS test/gen_lexer.hpp)

# Testing
# https://cmake.org/cmake/help/v3.17/manual/ctest.1.html#description
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/ctest/Testing-With-CTest
include(CTest)
add_test(dfa dfa_test)
add_test(null_first null_first_test)
add_test(build_parser build_parser_test)
add_test(regex regex_test)
add_test(tokenize tokenize_test)


set(MYTESTS dfa_test null_first_test build_parser_test regex_test tokenize_test)
set_tests_properties(
    dfa null_first build_parser regex tokenize
    PROPERTIES PASS_REGULAR_EXPRESSION "All tests passed!")

# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
add_custom_target(
    check
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS ${MYTESTS})



set(CTEST_MEMORYCHECK_COMMAND "valgrind")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full")
add_custom_target(
    valgrind_check
    COMMAND ${CMAKE_CTEST_COMMAND} -T memcheck
    DEPENDS ${MYTESTS})

# Remove generated files on make clean
# set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES test/gen_regex_dfa.hpp)
