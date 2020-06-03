# Project's name
project(language)

add_library(lang_lib
    lang_decl.cpp
    lang_stmt.cpp
    lang_expr.cpp
    typecheck/context.cpp
    typecheck/ctx_tree.cpp
    typecheck/type.cpp
    utils.cpp)

# Commented out to speed up compilation
target_link_libraries(lang_lib my_parser_lib misc_lib)
