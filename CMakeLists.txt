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

# Add subdirectories
add_subdirectory(${SRC}/parser)
add_subdirectory(${SRC}/assembly)
add_subdirectory(${TEST}/parser)


