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
        "//src/misc:logger",
        "@boost//:boost_headers",
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
    cmd_bash = "$(execpath //src/parser:parsegen) -g $(rootpath lang.pgen) -f $(RULEDIR)/parser",
    message = "Generating parser",
    tools = ["//src/parser:parsegen"],
)
