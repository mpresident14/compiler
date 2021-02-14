load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "temp",
    srcs = [
        "temp.cpp",
    ],
    hdrs = [
        "temp.hpp",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "assembly",
    srcs = [
        "asm_decl.cpp",
        "asm_instr.cpp",
        "flow_graph.cpp",
        "interference_graph.cpp",
    ],
    hdrs = [
        "assembly.hpp",
        "flow_graph.hpp",
        "interference_graph.hpp",
    ],
    visibility = ["//visibility:public"],
    deps = [":temp"],
)
