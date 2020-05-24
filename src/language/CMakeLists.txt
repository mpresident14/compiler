# Project's name
project(language)

add_library(lang_lib
    lang_decl.cpp
    lang_stmt.cpp
    lang_expr.cpp
    typecheck/context.cpp
    utils.cpp)
