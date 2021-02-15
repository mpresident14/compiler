package(
    default_visibility = [
        "//sandbox:__subpackages__",
        "//src/language:__subpackages__",
    ],
)

# "exports_files" + putting target in "data" section of
# cc_binary allows binary to find the file at runtime.
exports_files(
    glob([
        "*.prez",
    ]) + [
        "syscall.s",
    ],
)

genrule(
    name = "compile_string",
    srcs = [
        "string.prez",
    ],
    outs = [
        "string.s",
    ],
    # sed to prevent duplicate labels
    cmd_bash = "$(execpath //src/language:main) $(rootpath string.prez) $(RULEDIR)/string.s builtin \
                && sed 's/_L/_M/g' -i $(RULEDIR)/string.s",
    message = "Generating string.s",
    tools = ["//src/language:main"],
)
