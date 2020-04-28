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

# So regex sources don't have to be rebuilt for each executable
set(PARSER_SRCS src/build_parser.cpp src/null_first.cpp)
set(REGEX_SRCS src/regex.cpp src/regex_parse.cpp src/regex_eval.cpp)
add_library(parse_lib STATIC ${PARSER_SRCS})
add_library(regex_lib STATIC ${REGEX_SRCS})
target_link_libraries(regex_lib parse_lib)


###########
# TESTING #
###########
set(TEST_DIR test)
set(TEST_UTIL_DIR ${TEST_DIR}/utils)
set(TEST_GEN_DIR ${TEST_DIR}/generated)

# Test executables
add_executable(dfa_test ${TEST_DIR}/dfa_test.cpp)
add_executable(null_first_test ${TEST_DIR}/null_first_test.cpp src/null_first.cpp)
add_executable(build_parser_test ${TEST_DIR}/build_parser_test.cpp)
target_link_libraries(build_parser_test parse_lib)
add_executable(regex_test ${TEST_DIR}/regex_test.cpp)
target_link_libraries(regex_test parse_lib regex_lib)
add_executable(parse_test
    ${TEST_DIR}/parse_test.cpp
    ${TEST_GEN_DIR}/expr_parser.cpp
    ${TEST_GEN_DIR}/bad_expr_parser.cpp)
add_executable(tokenize_test
    ${TEST_DIR}/tokenize_test.cpp
    ${TEST_GEN_DIR}/random_lexer.cpp)

# Parser and lexer generation for testing
add_executable(write_parser ${TEST_UTIL_DIR}/write_parser.cpp src/generate.cpp)
add_executable(write_lexer ${TEST_UTIL_DIR}/write_lexer.cpp src/generate.cpp)
target_link_libraries(write_parser parse_lib regex_lib)
target_link_libraries(write_lexer parse_lib regex_lib)
# Ensure that we generate the correct code before building the corresponding test
add_custom_command(
    OUTPUT
        ${TEST_GEN_DIR}/expr_parser.hpp
        ${TEST_GEN_DIR}/expr_parser.cpp
        ${TEST_GEN_DIR}/bad_expr_parser.hpp
        ${TEST_GEN_DIR}/bad_expr_parser.cpp
    COMMAND write_parser
    COMMAND clang-format -i
        ${TEST_GEN_DIR}/expr_parser.hpp
        ${TEST_GEN_DIR}/expr_parser.cpp
        ${TEST_GEN_DIR}/bad_expr_parser.hpp
        ${TEST_GEN_DIR}/bad_expr_parser.cpp
    DEPENDS write_parser)
add_custom_command(
    OUTPUT ${TEST_GEN_DIR}/random_lexer.hpp ${TEST_GEN_DIR}/random_lexer.cpp
    COMMAND write_lexer
    COMMAND clang-format -i ${TEST_GEN_DIR}/random_lexer.hpp ${TEST_GEN_DIR}/random_lexer.cpp
    DEPENDS write_lexer)
set_source_files_properties(
    ${TEST_DIR}/parse_test.cpp
    PROPERTIES
        OBJECT_DEPENDS ${TEST_GEN_DIR}/expr_parser.hpp
        OBJECT_DEPENDS ${TEST_GEN_DIR}/bad_expr_parser.hpp)
set_source_files_properties(
    ${TEST_DIR}/tokenize_test.cpp
    PROPERTIES OBJECT_DEPENDS ${TEST_GEN_DIR}/random_lexer.hpp)

add_executable(write_config_lexer src/write_config_lexer.cpp src/generate.cpp)
target_link_libraries(write_config_lexer parse_lib regex_lib)
add_custom_command(
    OUTPUT src/config_lexer.hpp src/config_lexer.cpp
    COMMAND write_config_lexer
    COMMAND clang-format -i src/config_lexer.hpp src/config_lexer.cpp
    DEPENDS write_config_lexer)

add_executable(config_parse src/config_parse.cpp src/config_lexer.cpp)

# Running Tests
# https://cmake.org/cmake/help/v3.17/manual/ctest.1.html#description
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/c${TEST_DIR}/Testing-With-CTest
include(CTest)
add_test(dfa dfa_test)
add_test(null_first null_first_test)
add_test(build_parser build_parser_test)
add_test(regex regex_test)
add_test(parse parse_test)
add_test(tokenize tokenize_test)

set(MYTESTS dfa_test null_first_test build_parser_test regex_test parse_test tokenize_test)
set_tests_properties(
    dfa null_first build_parser regex parse tokenize
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
set_directory_properties(
    PROPERTIES
        ADDITIONAL_CLEAN_FILES "${TEST_DIR}/expr_parser.hpp;${TEST_DIR}/expr_parser.cpp;${TEST_DIR}/bad_expr_parser.hpp;${TEST_DIR}/bad_expr_parser.cpp")
