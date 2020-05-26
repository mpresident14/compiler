# Project's name
project(translator_test)

set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)
set(MY_TESTS "")
set(MY_TEST_EXES "")

function(make_test TEST_NAME EXPECTED_OUT)

  set(ASM_FILE ${TEST_NAME})
  string(APPEND ASM_FILE ".s")

  set(SRC_FILE ${TEST_NAME})
  string(APPEND SRC_FILE ".prez")

  set(EXE ${GEN_DIR}/${TEST_NAME})
  set(MY_TEST_EXES "${MY_TEST_EXES};${EXE}" PARENT_SCOPE)
  set(MY_TESTS "${MY_TESTS};${TEST_NAME}" PARENT_SCOPE)

  add_custom_command(
    OUTPUT ${GEN_DIR}/${TEST_NAME}
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/main
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}
        ${GEN_DIR}/${ASM_FILE}
    COMMAND clang -g
        -o ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
        ${GEN_DIR}/${ASM_FILE}
        ${SRC}/main/runtime.c
    DEPENDS main ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE})

  add_test(
      NAME ${TEST_NAME}
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_test.bash
          ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
          ${EXPECTED_OUT})
endfunction()

# Running Tests
# For testing
include(CTest)

make_test(arith_test "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17")
make_test(comp_test "1,2,3,4,5,6,7,8,9")
make_test(while_test "15,120")
make_test(ternary_test "10,10")
make_test(function_test "1,2")

message(${MY_TESTS})
message(${MY_TEST_EXES})


add_custom_target(
      translator_check
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      DEPENDS ${MY_TEST_EXES})

set_tests_properties(
    ${MY_TESTS}
    PROPERTIES
      PASS_REGULAR_EXPRESSION "Pass!"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Remove generated files on make clean
file(GLOB GENFILES LIST_DIRECTORIES false "${GEN_DIR}/*")
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${GENFILES}")
remove_gen_files("${GEN_DIR}")
