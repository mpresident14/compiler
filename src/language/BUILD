load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

# "exports_files" + putting target in "data" section of
# cc_binary allows binary to find the file at runtime.
cc_binary(
    name = "main",
    srcs = [
        "main.cpp",
    ],
    data = [
        "//src/language/built_in:string.prez",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":language",
    ],
)

cc_library(
    name = "language",
    srcs = [
        "ctx.cpp",
        "ctx_tree.cpp",
        "decl.cpp",
        "expr_as_bool.cpp",
        "expr_ctor.cpp",
        "expr_to_im.cpp",
        "src_file.cpp",
        "stmt.cpp",
        "type.cpp",
        "utils.cpp",
        ":generate_parser",
    ],
    hdrs = [
        "ctx.hpp",
        "decl.hpp",
        "expr.hpp",
        "language.hpp",
        "src_file.hpp",
        "stmt.hpp",
        "type.hpp",
        "utils.hpp",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/assembly",
        "//src/intermediate",
        "@boost//:algorithm",
        "@boost//:dynamic_bitset",
        "@mpresident14_cpplibs//src/logger",
    ],
)

genrule(
    name = "generate_parser",
    srcs = [
        "lang.pgen",
    ],
    outs = [
        "parser.hpp",
        "parser.cpp",
    ],
    cmd_bash = "$(execpath @mpresident14_parsers//src/generator:parsegen) -g $(rootpath lang.pgen) -d $(RULEDIR)/ -i src/language/ -n parser",
    message = "Generating parser",
    tools = ["@mpresident14_parsers//src/generator:parsegen"],
)
