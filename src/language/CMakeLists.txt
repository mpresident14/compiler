# Project's name
project(language)

add_library(lang_lib
    src_file.cpp
    decl.cpp
    stmt.cpp
    expr_ctor.cpp
    expr_to_im.cpp
    expr_as_bool.cpp
    typecheck/context.cpp
    typecheck/ctx_tree.cpp
    typecheck/type.cpp
    utils.cpp)

target_link_libraries(lang_lib my_parser_lib im_lib misc_lib)
