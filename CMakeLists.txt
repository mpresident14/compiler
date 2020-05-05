# Note: Env variable CXX specifies clang compiler
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
set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wextra -pedantic -g")

# Directory variables
set(SRC_DIR src)
set(TEST_DIR test)
set(TEST_UTIL_DIR ${TEST_DIR}/utils)
set(TEST_GEN_DIR ${TEST_DIR}/generated)

# Append these directories to compiler include path
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/${SRC_DIR} ${PROJECT_SOURCE_DIR}/${TEST_DIR})

# So regex sources don't have to be rebuilt for each executable
set(PARSER_SRCS ${SRC_DIR}/build_parser.cpp ${SRC_DIR}/null_first.cpp)
set(REGEX_SRCS ${SRC_DIR}/regex.cpp ${SRC_DIR}/regex_parse.cpp ${SRC_DIR}/regex_eval.cpp)
add_library(parse_lib STATIC ${PARSER_SRCS})
add_library(regex_lib STATIC ${REGEX_SRCS})
target_link_libraries(regex_lib parse_lib)


# Test executables
add_executable(dfa_test ${TEST_DIR}/dfa_test.cpp)

add_executable(null_first_test ${TEST_DIR}/null_first_test.cpp ${SRC_DIR}/null_first.cpp)

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

add_executable(parsegen_test
    ${TEST_DIR}/parsegen_test.cpp
    ${TEST_GEN_DIR}/gp_expr_parser.cpp)


# Parser and lexer generation for testing
add_executable(write_parser ${TEST_UTIL_DIR}/write_parser.cpp ${SRC_DIR}/generate.cpp)
target_link_libraries(write_parser parse_lib regex_lib)
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

add_executable(write_lexer ${TEST_UTIL_DIR}/write_lexer.cpp ${SRC_DIR}/generate.cpp)
target_link_libraries(write_lexer parse_lib regex_lib)
add_custom_command(
    OUTPUT ${TEST_GEN_DIR}/random_lexer.hpp ${TEST_GEN_DIR}/random_lexer.cpp
    COMMAND write_lexer
    COMMAND clang-format -i ${TEST_GEN_DIR}/random_lexer.hpp ${TEST_GEN_DIR}/random_lexer.cpp
    DEPENDS write_lexer)

add_executable(write_config_lexer ${SRC_DIR}/write_config_lexer.cpp ${SRC_DIR}/generate.cpp)
target_link_libraries(write_config_lexer parse_lib regex_lib)
add_custom_command(
    OUTPUT ${SRC_DIR}/config_lexer.hpp ${SRC_DIR}/config_lexer.cpp
    COMMAND write_config_lexer
    COMMAND clang-format -i ${SRC_DIR}/config_lexer.hpp ${SRC_DIR}/config_lexer.cpp
    DEPENDS write_config_lexer)

# This is the main executable
add_executable(parsegen
    ${SRC_DIR}/parsegen.cpp
    ${SRC_DIR}/config_parse.cpp
    ${SRC_DIR}/config_lexer.cpp
    ${SRC_DIR}/generate.cpp)
target_link_libraries(parsegen parse_lib regex_lib)
add_custom_command(
    OUTPUT ${TEST_GEN_DIR}/gp_expr_parser.hpp ${TEST_GEN_DIR}/gp_expr_parser.cpp
    COMMAND parsegen ${TEST_UTIL_DIR}/expr.gp ${TEST_GEN_DIR}/gp_expr_parser
    COMMAND clang-format -i ${TEST_GEN_DIR}/gp_expr_parser.hpp ${TEST_GEN_DIR}/gp_expr_parser.cpp
    DEPENDS parsegen ${TEST_UTIL_DIR}/expr.gp)

# Running Tests
# https://cmake.org/cmake/help/v3.17/manual/ctest.1.html#description
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/ctest/Testing-With-CTest
include(CTest)
add_test(dfa dfa_test)
add_test(null_first null_first_test)
add_test(build_parser build_parser_test)
add_test(regex regex_test)
add_test(parse parse_test)
add_test(tokenize tokenize_test)
add_test(parsegen parsegen_test)

set(MYTESTS
    dfa_test
    null_first_test
    build_parser_test
    regex_test
    parse_test
    tokenize_test
    parsegen_test)
set_tests_properties(
    dfa null_first build_parser regex parse tokenize parsegen
    PROPERTIES PASS_REGULAR_EXPRESSION "All tests passed!")

# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
add_custom_target(
    check
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS ${MYTESTS})

# Memory check tests with Valgrind
set(CTEST_MEMORYCHECK_COMMAND "valgrind")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full")
add_custom_target(
    valgrind_check
    COMMAND ${CMAKE_CTEST_COMMAND} -T memcheck
    DEPENDS ${MYTESTS})

# Remove generated files on make clean
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${TEST_GEN_DIR}/*")
