# Note: Env variable CXX specifies clang compiler
# Create Makefile with "cmake . "
# Then build executables with "make -j <threads>"

# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.17.0)

# Project's name
project(parser)

set(REL_TO_ROOT_DIR src/parser)

# So regex sources don't have to be rebuilt for each executable
add_library(parse_lib STATIC build_parser.cpp null_first.cpp)
add_library(regex_lib STATIC regex.cpp regex_parse.cpp regex_eval.cpp)
target_link_libraries(regex_lib parse_lib)

# Write the lexer for the .gp files
add_executable(write_config_lexer
    write_config_lexer.cpp
    generate.cpp)
target_link_libraries(write_config_lexer regex_lib)
add_custom_command(
    OUTPUT config_lexer.hpp config_lexer.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND write_config_lexer
    COMMAND clang-format -i ${REL_TO_ROOT_DIR}/config_lexer.hpp ${REL_TO_ROOT_DIR}/config_lexer.cpp
    DEPENDS write_config_lexer)

# This is the main executable
add_executable(parsegen
    parsegen.cpp
    config_parse.cpp
    config_lexer.cpp
    generate.cpp)
target_link_libraries(parsegen regex_lib)
