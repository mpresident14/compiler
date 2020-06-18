# Project's name
project(intermediate)

add_library(im_lib
    im_decl.cpp
    im_stmt.cpp
    im_expr_toassem.cpp
    im_expr_opt.cpp)

target_link_libraries(im_lib asm_lib)
