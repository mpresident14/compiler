# Project's name
project(intermediate)

# add_library(x86gen_lib
#     instruction.cpp
#     flow_graph.cpp
#     interference_graph.cpp
#     function.cpp)

add_executable(main main.cpp im_expr.cpp im_stmt.cpp)
target_link_libraries(main x86gen_lib)
