# Note: Used "EXPORT CXX=$(which clang++)" to specify clang compiler
# Create Makefile with "cmake . "
# Then build executables with "make -j <threads>"

# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

# Project's name
project(compiler)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})


# Set compilation flags
set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wextra -pedantic -g -I . -I ./src")

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src")

add_executable(dfa_test test/dfa_test.cpp)
add_executable(null_first_test test/null_first_test.cpp)
add_executable(lr1_test test/lr1_test.cpp)
add_executable(main src/main.cpp src/regex.cpp src/regex_lexer.cpp)


#add_executable(lr0_test test/lr0_test.cpp)
