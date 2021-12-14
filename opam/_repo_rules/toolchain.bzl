load("//opam/_functions:opam_cmds.bzl",
     "opam_get_current_switch_libdir",
     "opam_get_current_switch_prefix")
load("//opam/_functions:xdg.bzl", "get_xdg_paths")

#####################################
def _install_opam_symlinks(repo_ctx, opam_switch_prefix):
    if repo_ctx.attr.verbose:
        repo_ctx.report_progress("creating OPAM symlinks")

    repo_ctx.file("bin/BUILD.bazel",
                  content = """exports_files(glob([\"**\"]))""")
    bindir = opam_switch_prefix + "/bin"
    binpath = repo_ctx.path(bindir)
    binfiles = binpath.readdir()
    for file in binfiles:
        repo_ctx.symlink(file, "bin/" + file.basename)

    ## now the 'kernel' packages: bigarray, dynlink, etc.
    kernel_dir = opam_switch_prefix + "/lib/ocaml"
    kernel_path = repo_ctx.path(kernel_dir)
    kernel_files = kernel_path.readdir()

    ## bigarray is a virtual pkg, files in lib/ocaml
    buildfile_path = repo_ctx.path(
        Label("//opam/_templates:ocaml.bigarray.REPO")
    )
    buildfile = repo_ctx.read(buildfile_path)
    repo_ctx.file("bigarray/BUILD.bazel", content = buildfile)
    for file in kernel_files:
        if file.basename.startswith("bigarray"):
            repo_ctx.symlink(file, "bigarray/" + file.basename)

    ## compiler-libs
    buildfile_path = repo_ctx.path(
        Label("//opam/_templates:ocaml.compiler-libs.REPO")
    )
    buildfile = repo_ctx.read(buildfile_path)
    repo_ctx.file("compiler-libs/BUILD.bazel", content = buildfile)

    lib_dir = opam_switch_prefix + "/lib/ocaml/compiler-libs"
    lib_path = repo_ctx.path(lib_dir)
    lib_files = lib_path.readdir()
    for file in lib_files:
        repo_ctx.symlink(file, "compiler-libs/" + file.basename)

    ## dynlink
    buildfile_path = repo_ctx.path(
        Label("//opam/_templates:ocaml.dynlink.REPO")
    )
    buildfile = repo_ctx.read(buildfile_path)
    repo_ctx.file("dynlink/BUILD.bazel", content = buildfile)
    for file in kernel_files:
        if file.basename.startswith("dynlink"):
            repo_ctx.symlink(file, "dynlink/" + file.basename)

    ## str
    buildfile_path = repo_ctx.path(
        Label("//opam/_templates:ocaml.str.REPO")
    )
    buildfile = repo_ctx.read(buildfile_path)
    repo_ctx.file("str/BUILD.bazel", content = buildfile)
    for file in kernel_files:
        if file.basename.startswith("str."):
            repo_ctx.symlink(file, "str/" + file.basename)
        if file.basename == "libcamlstr.a":
            repo_ctx.symlink(file, "str/" + file.basename)
        ## stublibs/dllcamlstr.so?

    ## threads
    ## FIXME: organize thread, threads, threadUnix, libthreads, libthreadsnat
    buildfile_path = repo_ctx.path(
        Label("//opam/_templates:ocaml.threads.REPO")
    )
    buildfile = repo_ctx.read(buildfile_path)
    repo_ctx.file("threads/BUILD.bazel", content = buildfile)

    lib_dir = opam_switch_prefix + "/lib/ocaml/threads"
    lib_path = repo_ctx.path(lib_dir)
    lib_files = lib_path.readdir()
    for file in lib_files:
        repo_ctx.symlink(file, "threads/" + file.basename)
        ## then:
        ## ocaml/libthreads.a, ocaml/libthreadsnat.a,
        ## ocaml/stublibs/dllthreads.so

    ## unix
    buildfile_path = repo_ctx.path(
        Label("//opam/_templates:ocaml.unix.REPO")
    )
    buildfile = repo_ctx.read(buildfile_path)
    repo_ctx.file("unix/BUILD.bazel", content = buildfile)
    for file in kernel_files:
        if file.basename.startswith("unix"):
            repo_ctx.symlink(file, "unix/" + file.basename)
        if file.basename.startswith("libunix"):
            repo_ctx.symlink(file, "unix/" + file.basename)

###############################
def _opam_toolchain_impl(repo_ctx):

    print("running opam_toolchain for pkg: %s" % repo_ctx.attr.package)
    debug = False
    # if (ctx.label.name == "zexe_backend_common"):
    #     debug = True

    pkg_path = opam_get_current_switch_libdir(repo_ctx) + "/" + repo_ctx.attr.package
    print("NEW LOCAL pkg_path: %s" % pkg_path)

    opam_switch_prefix = opam_get_current_switch_prefix(repo_ctx)

    _install_opam_symlinks(repo_ctx, opam_switch_prefix)

    ## link each file, so the BUILD file is in same dir, and
    ## user-provided content can refer to files in current dir.
    ## can't read dir contents in starlark, so:
    # link_tgt = pkg_path + "/*"
    # repo_ctx.execute(["sh", "-c", "ln -s {} .".format(link_tgt)])

    # if repo_ctx.attr.workspace_file:
    #     ## todo: copy file
    #     fail("NOT IMPLEMENTED YET...")
    # elif repo_ctx.attr.workspace_file_content:
    #     repo_ctx.file(
    #         "WORKSPACE.bazel",
    #         content = repo_ctx.attr.build_file_content,
    #         executable=False
    #     )
    # else:
    #     repo_ctx.file(
    #         "WORKSPACE.bazel",
    #         content = """workspace( name = \"{pkg}\" )
    #         """.format(pkg = repo_ctx.attr.package),
    #         executable=False
    #     )

    # if repo_ctx.attr.build_file:
    #     ## todo: copy file
    #     fail("NOT IMPLEMENTED YET...")
    # elif repo_ctx.attr.build_file_content:
    #     repo_ctx.file(
    #         "BUILD.bazel",
    #         content = repo_ctx.attr.build_file_content,
    #         executable=False
    #     )
    # else:
    #     fail("One of build_file and build_file_content required.")

##################################
_opam_toolchain = repository_rule(
    doc = "Create an opam-based ocaml toolchain",
    implementation = _opam_toolchain_impl,
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

        verbose = attr.bool()
        # _rule = attr.string( default = "new_local_opam_repositories" ),
    ),
)

def opam_toolchain():
    print("opam_toolchain (fn)")
    return _opam_toolchain(name="ocaml.toolchain")
