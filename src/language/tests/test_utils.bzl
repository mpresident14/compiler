"""Compile and run .prez tests"""

load("//src/language:compile_and_run.bzl", "compile_and_run")

def test_success(name, expected, imports = []):
    compile_and_run(
        file = name,
        src = "//src/language/tests:success/%s.prez" % name,
    )
    native.sh_test(
        name = name + "_test",
        srcs = ["test_success.bash"],
        args = ["src/language/tests/" + name, "'%s'" % expected],
        data = [":" + name],
    )

# FOR FUTURE REFERENCE ONLY

# def _check_success_output(exe, expected):
#     return """
# #!/bin/bash
# pwd
# ls -R
# echo $(src/language/tests/arith)
# exit 0
# """.format(exe)

# def _success_test_impl(ctx):
#     ctx.actions.write(
#         output = ctx.outputs.executable,
#         content = _check_success_output(ctx.file.exe.path, ctx.attr.expected),
#         is_executable = True,
#     )
#     runfiles = ctx.runfiles(files = [ctx.file.exe])
#     return [DefaultInfo(runfiles = runfiles, executable = ctx.outputs.executable)]

# success_test = rule(
#     implementation = _success_test_impl,
#     attrs = {
#         "exe": attr.label(
#             mandatory = True,
#             allow_single_file = True,
#         ),
#         "expected": attr.string(
#             mandatory = True,
#         ),

#         # "expected": attr.label(mandatory = True),
#     },
#     test = True,
# )
