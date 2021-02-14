"""Compile and run .prez tests"""

load("//src/language:compile_and_run.bzl", "compile_and_run")

def compile_and_run_success(file, **kwargs):
    compile_and_run(
        file = file,
        src = "//src/language/tests:success/%s.prez" % file,
        **kwargs
    )

def test_success(name, exe):
    native.sh_test(
        name = name,
        srcs = ["test_success.bash"],
        args = [Label(exe), "test"],
        data = [":arith"],
    )

def _check_success_output(exe, expected):
#     return """
# # EXPECTED_OUTPUT=$(echo "{}" | sed 's/,/\n/g')

# OUTPUT=$({})
# echo $OUTPUT
# # if [[ "$OUTPUT" == "$EXPECTED_OUTPUT" ]]; then
# #   echo "Pass!"
# #   exit(0)
# # else
# #   echo "Failure..."
# #   echo "Expected:"
# #   echo "$EXPECTED_OUTPUT"
# #   echo
# #   echo "Got"
# #   echo "$OUTPUT"
# #   exit(1)
# # fi
# """.format(exe)
    return """
echo $({0})
exit(0)
""".format(exe)

def _success_test_impl(ctx):
    ctx.actions.write(
        output = ctx.outputs.executable,
        content = _check_success_output(ctx.file.exe.path, ctx.attr.expected),
    )
    runfiles = ctx.runfiles(files = [ctx.file.exe])
    return [DefaultInfo(runfiles = runfiles)]

success_test = rule(
    implementation = _success_test_impl,
    attrs = {
        "exe": attr.label(
            mandatory = True,
            allow_single_file = True,
        ),
        "expected": attr.string(
            mandatory = True,
        ),

        # "expected": attr.label(mandatory = True),
    },
    test = True,
)
