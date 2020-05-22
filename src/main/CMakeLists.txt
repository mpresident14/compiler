# Project's name
project(main)

add_library(my_parser_lib parser.cpp)
target_link_libraries(my_parser_lib lang_lib)
# add_custom_target(parser.cpp
#     PRE_BUILD
#     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#     COMMAND cp ${RUNTIME_OUTPUT_DIRECTORY}/parsegen ${CMAKE_CURRENT_SOURCE_DIR}/parsegen_copy
#     COMMAND ./parsegen_copy -g lang.pgen -f parser
#     COMMAND clang-format -i parser.hpp parser.cpp)

add_executable(main main.cpp)
target_link_libraries(main my_parser_lib)


add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/parser.hpp ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${RUNTIME_OUTPUT_DIRECTORY}/parsegen -g lang.pgen -f parser -l ${LOG_DIRECTORY}/parser.log
    COMMAND clang-format -i parser.hpp parser.cpp
    DEPENDS parsegen ${CMAKE_CURRENT_SOURCE_DIR}/lang.pgen)
