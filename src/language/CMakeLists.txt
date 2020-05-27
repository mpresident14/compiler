# Project's name
project(language)

add_library(lang_lib
    lang_decl.cpp
    lang_stmt.cpp
    lang_expr.cpp
    typecheck/context.cpp
    typecheck/type.cpp
    utils.cpp)
target_link_libraries(lang_lib im_lib misc_lib)
