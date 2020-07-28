# Project's name
project(translator_test)

include(CTest)

set(SUCCESS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/success)
file(GLOB IMPORTS LIST_DIRECTORIES false "${SRC}/built_in/*")
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

  if (${ARGC} EQUAL 3)
    set(EXTRA_DEPENDS ${ARGV2})
  endif()

  add_custom_command(
    OUTPUT
      ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
      ${LOG_DIRECTORY}/${ASM_FILE}
    WORKING_DIRECTORY ${SUCCESS_DIR}
    COMMAND #valgrind --leak-check=full
        ${RUNTIME_OUTPUT_DIRECTORY}/main
        ${SRC_FILE}
        ${LOG_DIRECTORY}/${ASM_FILE}
    COMMAND as
        ${LOG_DIRECTORY}/${ASM_FILE}
        ${SRC}/built_in/syscall.s
        ${SRC}/built_in/string.s
        -o ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.o
    COMMAND ld
        ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.o
        -o ${RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
    DEPENDS
      main
      ${SRC}/built_in/syscall.s
      ${SRC}/built_in/string.s
      ${SUCCESS_DIR}/${SRC_FILE}
      ${IMPORTS}
      ${EXTRA_DEPENDS})

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

  string(APPEND TEST_NAME "_error")
  set(MY_TESTS "${MY_TESTS};${TEST_NAME}" PARENT_SCOPE)

  add_test(
      NAME ${TEST_NAME}
      COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test_error.py
          ${RUNTIME_OUTPUT_DIRECTORY}/main
          ${SRC_FILE}
          ${LOG_DIRECTORY}/${ASM_FILE}
          ${EXPECTED_OUT})
endfunction()

add_custom_command(
    OUTPUT ${SRC}/built_in/string.s
    WORKING_DIRECTORY ${SRC}/built_in
    COMMAND
        ${RUNTIME_OUTPUT_DIRECTORY}/main
        ${SRC}/built_in/string.prez
        ${SRC}/built_in/string.s
        builtin
    # To prevent duplicate labels
    COMMAND sed 's/_L/_M/g' -i ${SRC}/built_in/string.s
    DEPENDS
        main
        ${SRC}/built_in/syscall.s
        ${SRC}/built_in/string.prez)

add_custom_target(
    sandbox
    COMMAND
        ${RUNTIME_OUTPUT_DIRECTORY}/main
        ${CMAKE_CURRENT_SOURCE_DIR}/sandbox.prez
        ${LOG_DIRECTORY}/sandbox.s
    COMMAND as
        ${LOG_DIRECTORY}/sandbox.s
        ${SRC}/built_in/syscall.s
        ${SRC}/built_in/string.s
        -o ${RUNTIME_OUTPUT_DIRECTORY}/sandbox.o
    COMMAND ld
        ${RUNTIME_OUTPUT_DIRECTORY}/sandbox.o
        -o ${RUNTIME_OUTPUT_DIRECTORY}/sandbox
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/sandbox
    DEPENDS main ${SRC}/built_in/string.s ${SRC}/built_in/syscall.s)

# Running Tests
success_test(arith "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17")
success_test(comp "1,2,3,4,5,6,7,8,9,true")
success_test(while "15,120")
success_test(for "0,1,2,0,0,0")
success_test(ternary "10,10,5,5,5")
success_test(function "1,2,118,4,5,6")
success_test(callee_save "9,9")
success_test(spill "0,1,1,2,3,5,8,13,21,34,55,89,144,233,2584")
success_test(array "0,1,1,2,3,5,8,13,21,34")
success_test(cast "!,!,15,6,6,4096,Derived")
success_test(opt "2,73,89,292,88,-2,89")
success_test(update "1,2,4,1,9,2,3,3")
success_test(incdec "2,3,3,2,1,4,4,5,4,20,5,26,6")
success_test(class "5,5,4,4,1,1,2,2,3,18")
success_test(strlit "Hello world!,Good-bye!,E\"sca\tpe\nd")
success_test(inheritance "100,vFunc,Base,200,vFunc,Derived,dvFunc,nonVFunc,300,vFunc,GrandChild,dvFunc,nonVFunc")
success_test(base_method "Child of Derived,Grandchild of Base,5,true")
success_test(static "Base,Base,Derived")

file(GLOB_RECURSE IMPORT_TEST_SRCS ${SUCCESS_DIR}/imports/*)
success_test(imports "1,2,3,4,5,6,7,8,9,16" "${IMPORT_TEST_SRCS};${SUCCESS_DIR}/file.prez")

error_test(redef
    "('Error','2','Redefinition')
    ('Error','9','Redefinition')
    ('Error','13','Redefinition')
    ('Error','17','Redefinition')")
error_test(undef
    "('Error','2','Undefined')
    ('Error','7','Undefined')
    ('Error','12','Undefined')
    ('Error','13','Undefined')
    ('Error','20','Undefined')")
error_test(return
    "('Error','2','Expected')
    ('Error','7','Expected')
    ('Error','16','Some')
    ('Error','26','Some')
    ('Error','32','Some')")
error_test(param
    "('Error','2','Undefined')
    ('Error','6','Undefined')")
error_test(type
    "('Error','6','Expected')
    ('Error','7','Expected')
    ('Error','8','Operator')
    ('Error','9','Expected')
    ('Error','10','Expected')
    ('Error','11','Expected')
    ('Error','12','Expected')
    ('Error','19','Comparison')
    ('Error','26','Operator')
    ('Error','27','Operator')
    ('Error','32','Expected')")
error_test(unused
    "('Warning','1','Unused')
    ('Warning','2','Unused')")
error_test(imports
    "('Error','2','No')
    ('Note','4','Duplicate')
    ('Error','13','Undefined')
    ('Error','14','Undefined')
    ('Error','15','Undefined')
    ('Error','16','Ambiguous')
    ('Error','17','Ambiguous')
    ('Error','18','Undefined')
    ('Error','20','Undefined')
    ('Error','23','Ambiguous')
    ('Error','23','Expected')")
error_test(narrowing
    "('Warning','2','Narrowing')
    ('Warning','6','Narrowing')
    ('Warning','7','Narrowing')
    ('Warning','10','Narrowing')
    ('Warning','11','Narrowing')
    ('Warning','11','Narrowing')
    ('Warning','11','Narrowing')")
error_test(member
    "('Error','3','Cannot')")
error_test(class
    "('Error','5','Expected')
    ('Error','9','Expected')
    ('Warning','13','Narrowing')
    ('Error','19','Undefined')
    ('Error','20','Expected')
    ('Error','22','Undefined')
    ('Error','23','Field')")
error_test(rvalue_assign
    "('Error','7','Operator')
    ('Error','8','Left')
    ('Error','9','Left')")
error_test(inheritance
    "('Error','22','Redefinition')
    ('Error','26','Redefinition')
    ('Error','32','Redefinition')
    ('Error','34','Redefinition')")
error_test(static
    "('Error','5','Type')
    ('Error','5','Undefined')
    ('Error','13','Undefined')")

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
file(GLOB GENFILES LIST_DIRECTORIES false "${LOG_DIRECTORY}/*")
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${GENFILES}")
remove_gen_files("${LOG_DIRECTORY}")
