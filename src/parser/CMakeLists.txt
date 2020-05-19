# Project's name
project(parser)

# So regex sources don't have to be rebuilt for each executable
add_library(parse_lib STATIC build_parser.cpp null_first.cpp)
add_library(regex_lib STATIC regex.cpp regex_parser.cpp regex_eval.cpp)
target_link_libraries(regex_lib parse_lib)

# Write the lexer for the .gp files
add_executable(write_config_lexer
    write_config_lexer.cpp
    generate.cpp)
target_link_libraries(write_config_lexer regex_lib)

# NOTE: OUTPUT and WORKING_DIRECTORY are relative to corresponding
# *build* tree directory
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/config_lexer.hpp ${CMAKE_CURRENT_SOURCE_DIR}/config_lexer.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/write_config_lexer
    COMMAND clang-format -i
        ${CMAKE_CURRENT_SOURCE_DIR}/config_lexer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/config_lexer.cpp
    DEPENDS write_config_lexer ${CMAKE_CURRENT_SOURCE_DIR}/config_grammar.hpp)


# This is the main executable
add_executable(parsegen
    parsegen.cpp
    config_parse.cpp
    config_lexer.cpp
    generate.cpp)
target_link_libraries(parsegen regex_lib)
