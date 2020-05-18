# Project's name
project(x86gen_test)


set(SRC_DIR ${SRC}/x86gen)
set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)

# Test executables
add_executable(compile_tests compile_tests.cpp)
target_link_libraries(compile_tests x86gen_lib)

add_custom_command(
  OUTPUT test0
  WORKING_DIRECTORY ${GEN_DIR}
  COMMAND compile_tests
  COMMAND clang -g -o test0 test0.s ${SRC_DIR}/runtime.c)

# Running Tests
# For testing
include(CTest)
set(MYTESTS test0)
add_test(NAME test0 COMMAND run_test.bash ${GEN_DIR}/test0 11)
add_custom_target(
      x86gen_check
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      DEPENDS ${MYTESTS})
set_tests_properties(
    ${MYTESTS}
    PROPERTIES
      PASS_REGULAR_EXPRESSION "Pass"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Remove generated files on make clean
# file(GLOB GENFILES LIST_DIRECTORIES false "${GEN_DIR}/*")
# set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${GENFILES}")
remove_gen_files("${GEN_DIR}")
