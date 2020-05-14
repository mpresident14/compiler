# SETUP
# mdkir build
# cd build
# cmake .. -DCMAKE_C_COMPILER=$(which clang) -DCMAKE_CXX_COMPILER=$(which clang++)
# Then build executables (from build dir) with "make -j <threads>"

# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.17.0)

# Project's name
project(compiler)

# Set the output folder where your program will be created
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
# These two are supposedly deprecated in favor of the two above,
# but those don't seem to be working
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(SRC ${CMAKE_SOURCE_DIR}/src)
set(TEST ${CMAKE_SOURCE_DIR}/test)

# Append these directories to compiler include path
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${LIBRARY_OUTPUT_PATH})
include_directories(${CMAKE_BINARY_DIR})

# Set compilation flags
set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Wextra -pedantic -g")


##  Helper functions ##

# Create tests
# https://cmake.org/cmake/help/v3.17/manual/ctest.1.html#description
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/ctest/Testing-With-CTest
# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
set(CTEST_MEMORYCHECK_COMMAND "valgrind")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full")

function(create_tests TARGET MYTESTS)
  # Add the tests
  foreach(TEST ${MYTESTS})
    add_test(NAME ${TEST} COMMAND ${TEST})
  endforeach()

  # Custom targets to run all the tests with/without valgrind
  add_custom_target(
      ${TARGET}_check
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      DEPENDS ${MYTESTS})
  add_custom_target(
      ${TARGET}_valgrind_check
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -T memcheck
      DEPENDS ${MYTESTS})

  # Default pass regex
  if (NOT ${ARGV2})
    set_tests_properties(
      ${MYTESTS}
      PROPERTIES PASS_REGULAR_EXPRESSION "All tests passed!")
  endif()

endfunction()

function(remove_gen_files GEN_DIR)
  file(GLOB GENFILES LIST_DIRECTORIES false "${GEN_DIR}/*")
  set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${GENFILES}")
endfunction()



# Add subdirectories
add_subdirectory(${SRC}/parser)
add_subdirectory(${SRC}/x86gen)
add_subdirectory(${SRC}/intermediate)

add_subdirectory(${TEST}/parser)
add_subdirectory(${TEST}/x86gen)
