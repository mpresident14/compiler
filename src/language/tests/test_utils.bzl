"""Compile and run .prez tests"""

load("//src/language:compile_and_run.bzl", "compile_and_run")
load("@rules_python//python:defs.bzl", "py_test")

def test_success(testname, expected):
    compile_and_run(
        file = testname,
        src = "//src/language/tests:success/%s.prez" % testname,
    )
    native.sh_test(
        name = testname + "_success_test",
        srcs = ["test_success.bash"],
        args = ["src/language/tests/" + testname, "'%s'" % expected],
        data = [":" + testname],
    )



def test_error(testname, expected):
    quoted_expected = ["\"%s\"" % check for check in expected]
    py_test(
        name = testname + "_error_test",
        main = "test_error.py",
        srcs = ["test_error.py"],
        args = [
            "src/language/main",
            "src/language/tests/error/%s.prez" % testname,
        ] + quoted_expected,
        data = [
            "//src/language/tests:error/%s.prez" % testname,
            "//src/language:main",
        ],
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
