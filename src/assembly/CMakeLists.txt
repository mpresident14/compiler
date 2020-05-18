# Project's name
project(assembly)

add_library(asm_lib
    asm_decl.cpp
    asm_instr.cpp
    reg_alloc/flow_graph.cpp
    reg_alloc/interference_graph.cpp)

