# Project's name
project(x86gen_test)


set(SRC_DIR ${SRC}/x86gen)
set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)

# Test executables
add_executable(compile_tests compile_tests.cpp)
target_link_libraries(compile_tests x86gen_lib)


set(TEST_PREFIX test)

add_custom_command(
  OUTPUT test0
  WORKING_DIRECTORY ${GEN_DIR}
  COMMAND compile_tests
  COMMAND clang -g -o test0 test0.s ${SRC_DIR}/runtime.c)

# Running Tests
# https://cmake.org/cmake/help/v3.17/manual/ctest.1.html#description
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/ctest/Testing-With-CTest
# For testing
include(CTest)

add_test(NAME test0 COMMAND run_test.bash ${GEN_DIR}/test0 11)


set(MYTESTS
    test0)
set_tests_properties(
    test0
    PROPERTIES
      PASS_REGULAR_EXPRESSION "Failure"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
add_custom_target(
    x86gen_check
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS ${MYTESTS})


# Memory check tests with Valgrind
set(CTEST_MEMORYCHECK_COMMAND "valgrind")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full")
add_custom_target(
    x86gen_valgrind_check
    COMMAND ${CMAKE_CTEST_COMMAND} -T memcheck
    DEPENDS ${MYTESTS})

