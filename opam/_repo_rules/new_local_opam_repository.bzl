load("//opam/_functions:opam_cmds.bzl", "opam_get_current_switch_libdir")
load("//opam/_functions:xdg.bzl", "get_xdg_paths")

###############################
def impl_new_local_opam_repository(repo_ctx):

    # print("running new_local_opam_repository for pkg: %s" % repo_ctx.attr.package)
    debug = False
    # if (ctx.label.name == "zexe_backend_common"):
    #     debug = True

    pkg_path = opam_get_current_switch_libdir(repo_ctx) + "/" + repo_ctx.attr.package
    # print("NEW LOCAL pkg_path: %s" % pkg_path)

    ## TODO: verify pkg path exists

    ## link each file, so the BUILD file is in same dir, and
    ## user-provided content can refer to files in current dir.
    ## can't read dir contents in starlark, so:
    link_tgt = pkg_path + "/*"
    repo_ctx.execute(["sh", "-c", "ln -s {} .".format(link_tgt)])

    if repo_ctx.attr.workspace_file:
        ## todo: copy file
        fail("NOT IMPLEMENTED YET...")
    elif repo_ctx.attr.workspace_file_content:
        repo_ctx.file(
            "WORKSPACE.bazel",
            content = repo_ctx.attr.build_file_content,
            executable=False
        )
    else:
        repo_ctx.file(
            "WORKSPACE.bazel",
            content = """workspace( name = \"{pkg}\" )
            """.format(pkg = repo_ctx.attr.package),
            executable=False
        )

    if repo_ctx.attr.build_file:
        ## todo: copy file
        fail("NOT IMPLEMENTED YET...")
    elif repo_ctx.attr.build_file_content:
        repo_ctx.file(
            "BUILD.bazel",
            content = repo_ctx.attr.build_file_content,
            executable=False
        )
    else:
        fail("One of build_file and build_file_content required.")

#####################################
## if opam pkg has no META file, no BUILD.bazel will be generated for it.
## in which case we need to manual set it up:
new_local_opam_repository = repository_rule(
    doc = """Create a repo linked to a local path""",
    implementation = impl_new_local_opam_repository,
    attrs = dict(

        switch = attr.string(
        ),

        package = attr.string(
            doc = """OPAM package name.

            """,
        ),

        build_file  = attr.label(
            doc = "build file"
        ),

        build_file_content  = attr.string(
            doc = "build file content"
        ),

        workspace_file  = attr.label(
            doc = "workspace file"
        ),

        workspace_file_content  = attr.string(
            doc = "workspace file content"
        ),

        install = attr.bool(
            # run opam commands to install?
            default = False,
        ),

        # _rule = attr.string( default = "new_local_opam_repositories" ),
    ),
)
