"""Test rules for compilation success and errors."""

def expect_error():
    return """
TEST="$1"
EXPECTED_OUTPUT=$(echo "$2" | sed 's/,/\n/g')

OUTPUT=$($TEST)
if [[ "$OUTPUT" == "$EXPECTED_OUTPUT" ]]; then
  echo "Pass!"
  exit(0)
else
  echo "Failure..."
  echo "Expected:"
  echo "$EXPECTED_OUTPUT"
  echo
  echo "Got"
  echo "$OUTPUT"
  exit(1)
fi
"""

def error_test_impl(ctx):
    # script = expect_error(expected)

    ctx.actions.run(
        executable = ctx.executable.main,
        inputs = [ctx.file.src],
        arguments = [ctx.file.src.path, ctx.outputs.asm.path],
        outputs = [ctx.outputs.asm],
        progress_message = "Compiling %s into %s" % (ctx.file.src.path, ctx.outputs.asm.path),
    )

    # ctx.actions.run_shell(
    #     outputs = [ctx.outputs.asm],
    #     inputs = [ctx.outputs.asm],
    #     arguments = [ctx.outputs.asm.path],
    #     command = "as $1",
    # )

    # runfiles = ctx.runfiles(files = ctx.file.prez_src)
    # return [DefaultInfo(runfiles = runfiles)]

my_rule = rule(
    implementation = error_test_impl,
    # test = True,
    attrs = {
        "main": attr.label(
            executable = True,
            cfg = "target",
            allow_single_file = True,
            default = Label("//src/language:main_for_test"),
        ),
        "src": attr.label(
            mandatory = True,
            allow_single_file = True,
        ),
        "asm": attr.output(
            mandatory = True,
        ),
        # "expected": attr.label(mandatory = True),
    },
)
