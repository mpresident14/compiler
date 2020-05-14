# Project's name
project(parser_test)

# Directory variables
set(PARSER_SRC_DIR ${SRC}/parser)
set(UTIL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/utils)
set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)

# Test executables
add_executable(dfa_test dfa_test.cpp)

add_executable(null_first_test null_first_test.cpp ${PARSER_SRC_DIR}/null_first.cpp)

add_executable(build_parser_test build_parser_test.cpp)
target_link_libraries(build_parser_test parse_lib)

add_executable(regex_test regex_test.cpp)
target_link_libraries(regex_test regex_lib)

add_executable(parse_test
    parse_test.cpp
    ${GEN_DIR}/expr_parser.cpp
    ${GEN_DIR}/bad_expr_parser.cpp)

add_executable(tokenize_test
    tokenize_test.cpp
    ${GEN_DIR}/random_lexer.cpp)

add_executable(parsegen_test
    parsegen_test.cpp
    ${GEN_DIR}/pgen_expr_parser.cpp)


# Parser and lexer generation for testing
add_executable(write_parser ${UTIL_DIR}/write_parser.cpp ${PARSER_SRC_DIR}/generate.cpp)
target_link_libraries(write_parser regex_lib)
add_custom_command(
    OUTPUT
        ${GEN_DIR}/expr_parser.hpp
        ${GEN_DIR}/expr_parser.cpp
        ${GEN_DIR}/bad_expr_parser.hpp
        ${GEN_DIR}/bad_expr_parser.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/write_parser
    COMMAND clang-format -i
        ${GEN_DIR}/expr_parser.hpp
        ${GEN_DIR}/expr_parser.cpp
        ${GEN_DIR}/bad_expr_parser.hpp
        ${GEN_DIR}/bad_expr_parser.cpp
    DEPENDS write_parser)

add_executable(write_lexer ${UTIL_DIR}/write_lexer.cpp ${PARSER_SRC_DIR}/generate.cpp)
target_link_libraries(write_lexer regex_lib)
add_custom_command(
    OUTPUT ${GEN_DIR}/random_lexer.hpp ${GEN_DIR}/random_lexer.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/write_lexer
    COMMAND clang-format -i
        ${GEN_DIR}/random_lexer.hpp
        ${GEN_DIR}/random_lexer.cpp
    DEPENDS write_lexer)

add_custom_command(
    OUTPUT ${GEN_DIR}/pgen_expr_parser.hpp ${GEN_DIR}/pgen_expr_parser.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/parsegen
        -g ${UTIL_DIR}/expr.pgen
        -f ${GEN_DIR}/pgen_expr_parser
    COMMAND clang-format -i
        ${GEN_DIR}/pgen_expr_parser.hpp
        ${GEN_DIR}/pgen_expr_parser.cpp
    DEPENDS parsegen ${UTIL_DIR}/expr.pgen)


# Running Tests
# https://cmake.org/cmake/help/v3.17/manual/ctest.1.html#description
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/ctest/Testing-With-CTest
# For testing
include(CTest)

add_test(NAME dfa COMMAND dfa_test)
add_test(NAME null_first COMMAND null_first_test)
add_test(NAME build_parser COMMAND build_parser_test)
add_test(NAME regex COMMAND regex_test)
add_test(NAME parse COMMAND parse_test)
add_test(NAME tokenize COMMAND tokenize_test)
add_test(NAME parsegen COMMAND parsegen_test)

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

