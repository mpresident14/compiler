# Note: Env variable CXX specifies clang compiler
# Create Makefile with "cmake . "
# Then build executables with "make -j <threads>"

# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.17.0)

# Project's name
project(parser)

add_executable(write_config_lexer
    write_config_lexer.cpp
    generate.cpp)
target_link_libraries(write_config_lexer parse_lib regex_lib)
add_custom_command(
    OUTPUT config_lexer.hpp config_lexer.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND write_config_lexer
    COMMAND clang-format -i ${PROJECT_SOURCE_DIR}/config_lexer.hpp ${PROJECT_SOURCE_DIR}/config_lexer.cpp
    DEPENDS write_config_lexer)

# This is the main executable
add_executable(parsegen
    parsegen.cpp
    config_parse.cpp
    config_lexer.cpp
    generate.cpp)
target_link_libraries(parsegen parse_lib regex_lib)
