load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "intermediate",
    srcs = [
        "im_decl.cpp",
        "im_expr_opt.cpp",
        "im_expr_toassem.cpp",
        "im_stmt.cpp",
    ],
    hdrs = [
        "intermediate.hpp",
    ],
    visibility = ["//visibility:public"],
    deps = ["//src/assembly"],
)
