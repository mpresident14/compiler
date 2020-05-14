# Project's name
project(x86gen)

add_library(x86gen_lib
    instruction.cpp
    flow_graph.cpp
    interference_graph.cpp
    function.cpp)

