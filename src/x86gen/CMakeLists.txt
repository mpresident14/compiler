# Project's name
project(x86gen)

add_library(x86gen_lib
    instruction.cpp
    flow_graph.cpp
    interference_graph.cpp
    function.cpp)

add_executable(main main.cpp)
target_link_libraries(main x86gen_lib)
