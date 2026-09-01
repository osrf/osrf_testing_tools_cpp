def _test_runner_sh_test_impl(ctx):
    script_content = """#!/bin/sh
# Script to execute {runner_label} with arguments.

set -e # Exit immediately if a command exits with a non-zero status.

RUNNER_PATH="{runner_path_prefix}/{runner_short_path}"
TEST_PATH="{test_path_prefix}/{test_short_path}"
RUNNER_ARGS=({test_runner_args_string})
TEST_ARGS=({test_args_string})

echo "Executing: $RUNNER_PATH" "${{RUNNER_ARGS[@]}}" "-- $TEST_PATH ${{TEST_ARGS[@]}}"
"$RUNNER_PATH" "${{RUNNER_ARGS[@]}}" -- $TEST_PATH "${{TEST_ARGS[@]}}"
""".format(
        runner_label = ctx.attr.test_runner_binary,
        runner_path_prefix = ctx.attr.test_runner_binary.label.package if ctx.attr.test_runner_binary.label.package else ".",
        runner_short_path = ctx.attr.test_runner_binary.label.name,
        test_path_prefix = ctx.attr.test_binary.label.package if ctx.attr.test_binary.label.package else ".",
        test_short_path = ctx.attr.test_binary.label.name,
        test_runner_args_string = " ".join(["'{}'".format(arg) for arg in ctx.attr.test_runner_args]),
        test_args_string = " ".join(["'{}'".format(arg) for arg in ctx.attr.test_args]),
    )

    script_name = "{}_runner.sh".format(ctx.attr.name)
    ctx.actions.write(
        output = ctx.outputs.executable,
        content = script_content,
        is_executable = True,
    )

    # The runfiles need to include the actual test_runner binary
    runfiles = ctx.runfiles(files = [ctx.executable.test_runner_binary, ctx.executable.test_binary])

    return [DefaultInfo(
        files = depset([ctx.outputs.executable]),
        runfiles = runfiles,
    )]

_test_runner_sh_test = rule(
    implementation = _test_runner_sh_test_impl,
    attrs = {
        "test_runner_binary": attr.label(
            doc = "The test runner binary.",
            mandatory = True,
            executable = True,
            cfg = "exec", # Compile for execution platform
        ),
        "test_binary": attr.label(
            doc = "The binary target to be tested.",
            mandatory = True,
            executable = True,
            cfg = "exec", # Compile for execution platform
        ),
        "test_runner_args": attr.string_list(
            doc = "A list of arguments to pass to the test_runner.",
            default = [],
        ),
        "test_args": attr.string_list(
            doc = "A list of arguments to pass to the test.",
            default = [],
        ),
    },
    executable = True, # Signifies that this rule outputs an executable (the script)
    test = True,       # Signifies that this rule can be run as a test
)

def test_runner_sh_test(name, test_runner, test_binary, test_runner_args = [], test_args = [], **kwargs):
    """
    Generates an sh_test that executes the given test_runner binary with specified arguments.

    Args:
      name: The name of the sh_test.
      test_runner: The label of the cc_binary or other executable target.
      args: A list of strings representing the arguments to pass to the test_runner.
      **kwargs: Additional arguments to pass to the underlying sh_test rule.
    """
    script_name = "{}_generated_script".format(name)

    # Using the internal rule to generate the script
    _test_runner_sh_test(
        name = script_name,
        test_runner_binary = test_runner,
        test_runner_args = test_runner_args,
        test_binary = test_binary,
        test_args = test_args,
        # Ensure the generated script itself is not the test directly,
        # but is used by the native.sh_test
        testonly = True, # Mark as testonly if the script itself isn't meant for release
        tags = kwargs.pop("tags", []) + ["manual"], # Usually you don't run this rule directly
    )

    # Create the sh_test rule using the generated script
    native.sh_test(
        name = name,
        srcs = [":" + script_name],
        data = [test_runner, test_binary],
    )
