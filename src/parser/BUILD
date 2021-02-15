load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//src/parser:__subpackages__"])

cc_library(
    name = "common",
    hdrs = [
        "rules.hpp",
        "utils.hpp",
    ],
)

cc_library(
    name = "dfa",
    hdrs = ["dfa.hpp"],
    deps = [":common"],
)

cc_library(
    name = "null_first",
    srcs = ["null_first.cpp"],
    hdrs = ["null_first.hpp"],
    deps = [
        ":common",
        "@boost//:dynamic_bitset",
    ],
)

cc_library(
    name = "queue_set",
    hdrs = ["queue_set.hpp"],
)

cc_library(
    name = "build_parser",
    srcs = [
        "build_parser.cpp",
        "queue_set.hpp",
    ],
    hdrs = [
        "build_parser.hpp",
    ],
    linkopts = ["-pthread"],
    deps = [
        ":common",
        ":dfa",
        ":null_first",
        "//src/misc:logger",
        "@boost//:dynamic_bitset",
    ],
)

cc_library(
    name = "regex",
    srcs = [
        "regex.cpp",
        "regex.hpp",
        "regex_merge.cpp",
        "regex_parser.cpp",
    ],
    hdrs = [
        "regex_merge.hpp",
        "regex_parser.hpp",
    ],
    deps = [
        ":common",
        ":dfa",
        "//src/misc:logger",
    ],
)

cc_library(
    name = "generate",
    srcs = [
        "generate.cpp",
        "parse_info.hpp",
    ],
    hdrs = [
        "generate.hpp",
    ],
    deps = [
        ":build_parser",
        ":regex",
    ],
)

cc_binary(
    name = "write_config_lexer",
    srcs = [
        "config_grammar.hpp",
        "write_config_lexer.cpp",
    ],
    deps = [
        ":common",
        ":generate",
        "//src/misc:logger",
    ],
)

# TODO: Rename config_parse/lexer to pgen_parse/lexer

# Needed to add --host_copt="-std=c++20" to bazelrc
# (see https://docs.bazel.build/versions/master/guide.html#build-configurations-and-cross-compilation)
genrule(
    name = "generate_pgen_lexer",
    outs = [
        "config_lexer.hpp",
        "config_lexer.cpp",
    ],
    cmd_bash = "$(execpath :write_config_lexer) $(RULEDIR)/config_lexer",
    message = "Generating pgen_lexer",
    tools = [":write_config_lexer"],
)

cc_library(
    name = "config_parse",
    srcs = [
        "config_parse.cpp",
        ":generate_pgen_lexer",
    ],
    hdrs = [
        "config_grammar.hpp",
        "config_parse.hpp",
        "parse_info.hpp",
    ],
    deps = [
        ":common",
        "//src/misc:logger",
    ],
)

cc_binary(
    name = "parsegen",
    srcs = [
        "parsegen.cpp",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":config_parse",
        ":generate",
        "//src/misc:logger",
    ],
)
