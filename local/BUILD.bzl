def _impl_opam_import(ctx):

    runner = ctx.actions.declare_file("import.sh")

    cmd = "\n".join([
        "EXECROOT=$PWD",
        "echo BUILD_WORKSPACE_DIRECTORY: $BUILD_WORKSPACE_DIRECTORY",
        "if [ ! -d \"$BUILD_WORKSPACE_DIRECTORY/_opam\" ]; then",
        "    echo Creating local switch with {}".format(ctx.attr.compiler),
        "    cd $BUILD_WORKSPACE_DIRECTORY && opam switch --yes create . {}".format(ctx.attr.compiler),
        "    eval $(opam env)",
        "fi\n",
        "if [ \"$1\" == \"-v\" ]; then",
        "    set -x",
        "fi\n"
    ])

    for repo in ctx.attr.manifests:
        cmd = cmd + " ".join([
            "echo Importing {} to local switch in ./_opam;\n".format(
                repo.label),
            # "echo location: $EXECROOT/{};\n".format(
            #     repo.files.to_list()[0].path),
            "cd", "$BUILD_WORKSPACE_DIRECTORY",
            "&&",
            "opam", "switch",
            # "--dry-run",
            "--verbose",
            "--switch",  ".",
            "--description='Import {}'".format(repo.label),
            "--yes",
            "import",
            "$EXECROOT/{}".format(repo.files.to_list()[0].path),
            "\n",
            "echo \n",
        ])

    print("IMPORT CMD: %s" % cmd)

    ctx.actions.write(
        output  = runner,
        content = cmd,
        is_executable = True,
    )

    myrunfiles = ctx.runfiles(
        files = ctx.files.manifests # + [ctx.file._switch],
    )

    defaultInfo = DefaultInfo(
        executable=runner,
        runfiles = myrunfiles
    )

    return defaultInfo

####################
opam_import = rule(
  implementation = _impl_opam_import,
    doc = """Import opam packages into the local switch""",
    attrs = dict(
        _switch = attr.label(allow_single_file=True),
        compiler = attr.string(doc = "Compiler version"),
        manifests = attr.label_list(
            doc = "List of repo labels.",
            allow_files = True,
            # providers   = [OcamlModuleMarker],
        ),
    ),
    # provides = [OcamlNsResolverProvider],
    executable = True,
    # toolchains = [
    #     "@rules_ocaml//ocaml:toolchain",
    # ],
)

