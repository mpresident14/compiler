# Project's name
project(translator_test)

set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)
set(SUCCESS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/success)
set(ERROR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/error)
set(MY_TESTS "")
set(MY_TEST_EXES "")


function(success_test TEST_NAME EXPECTED_OUT)
  set(ASM_FILE ${TEST_NAME})
  string(APPEND ASM_FILE ".s")

  set(SRC_FILE ${TEST_NAME})
  string(APPEND SRC_FILE ".prez")

  set(EXE ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME})
  set(MY_TEST_EXES "${MY_TEST_EXES};${EXE}" PARENT_SCOPE)
  set(MY_TESTS "${MY_TESTS};${TEST_NAME}" PARENT_SCOPE)

  add_custom_command(
    OUTPUT
      ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
      ${GEN_DIR}/${ASM_FILE}
    COMMAND #valgrind --leak-check=full
        ${RUNTIME_OUTPUT_DIRECTORY}/main
        ${SUCCESS_DIR}/${SRC_FILE}
        ${GEN_DIR}/${ASM_FILE}
        ${LOG_DIRECTORY}/${ASM_FILE}
    COMMAND clang -g
        -o ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
        ${GEN_DIR}/${ASM_FILE}
        ${SRC}/main/runtime.c
    DEPENDS main ${SUCCESS_DIR}/${SRC_FILE})

  add_test(
      NAME ${TEST_NAME}
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_success.bash
          ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
          ${EXPECTED_OUT})
endfunction()


function(error_test TEST_NAME EXPECTED_OUT)
  set(ASM_FILE ${TEST_NAME})
  string(APPEND ASM_FILE ".s")

  set(SRC_FILE ${TEST_NAME})
  string(APPEND SRC_FILE ".prez")

  set(MY_TESTS "${MY_TESTS};${TEST_NAME}" PARENT_SCOPE)

  add_test(
      NAME ${TEST_NAME}
      COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_error.py
          ${RUNTIME_OUTPUT_DIRECTORY}/main
          ${ERROR_DIR}/${SRC_FILE}
          ${GEN_DIR}/${ASM_FILE}
          ${EXPECTED_OUT})
endfunction()


# Running Tests
# For testing
include(CTest)

success_test(arith_test "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17")
success_test(comp_test "1,2,3,4,5,6,7,8,9")
success_test(while_test "15,120")
success_test(ternary_test "10,10")
success_test(function_test "1,2")
success_test(callee_save_test "9,9")
success_test(spill_test "0,1,2,3,4,5,6,7,8,9,10,11,12,13")

error_test(redef_test
    "('Error','2','Redefinition')
    ('Error','9','Redefinition')
    ('Error','13','Redefinition')
    ('Error','17','Redefinition')")
error_test(undef1_test "('Error','2','Undefined')")
error_test(undef2_test "('Error','3','Undefined')")
error_test(undef3_test "('Error','2','Undefined')")
error_test(undef4_test "('Error','3','Undefined')")
error_test(return_test
    "('Error','2','Expected')
    ('Error','7','Expected')
    ('Error','10','Non-void')")
error_test(param_test
    "('Error','2','Expected')
    ('Error','14','Wrong')")
error_test(type_test
    "('Error','6','Expected')
    ('Error','7','Expected')
    ('Error','8','Expected')
    ('Error','9','Expected')
    ('Error','10','Expected')")
error_test(unused_test
    "('Warning','1','Unused')
    ('Warning','2','Unused')")


add_custom_target(
    translator_check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS main ${MY_TEST_EXES})

set_tests_properties(
    ${MY_TESTS}
    PROPERTIES
      PASS_REGULAR_EXPRESSION "Pass!"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Remove generated files on make clean
file(GLOB GENFILES LIST_DIRECTORIES false "${GEN_DIR}/*")
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${GENFILES}")
remove_gen_files("${GEN_DIR}")
