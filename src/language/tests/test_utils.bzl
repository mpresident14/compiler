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
        size = "small",
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
        size = "small",
    )
