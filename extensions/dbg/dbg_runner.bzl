def _ocamldebug_runner_impl(ctx):

    bin = ctx.actions.declare_file("dbg")
    ctx.actions.symlink(
        output = bin,
        target_file = ctx.file._bin,
        is_executable = True)

    rtEnvInfo = RunEnvironmentInfo(
        environment = ctx.attr.env
    )

    # if ctx.files._program == @rules_ocaml//cfg:null
    # the user has not passed --@dbg//x=foo,
    # so throw error
    runfiles = ctx.runfiles(files = ctx.files._program)

    defaultInfo = DefaultInfo(
        executable = bin,
        runfiles   = runfiles
    )

    return [defaultInfo, rtEnvInfo]

ocamldebug_runner = rule(
    implementation = _ocamldebug_runner_impl,
    doc = "Runs ocamldebug.",
    executable = True,
    attrs = dict(
        data = attr.label_list(),
        env = attr.string_dict(
            doc = "Env variables",
            allow_empty = True
        ),
        _bin = attr.label(
            doc = "Bazel label of ocamldebug executable.",
            mandatory = False,
            allow_single_file = True,
            executable = True,
            default = "@opam.ocamlsdk//bin:ocamldebug",
            cfg = "exec"
        ),
        _program = attr.label(
            default = "@dbg//x",
            allow_single_file = True,
        ),
    )
)
