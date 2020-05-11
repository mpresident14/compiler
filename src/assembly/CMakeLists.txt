# Project's name
project(assembly)

set(REL_TO_ROOT_DIR src/assembly)

add_library(regalloc_lib
    instruction.cpp
    flow_graph.cpp
    interference_graph.cpp
    function.cpp)

add_executable(main main.cpp)
target_link_libraries(main regalloc_lib)
