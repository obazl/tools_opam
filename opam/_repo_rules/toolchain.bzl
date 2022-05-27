load("//opam/_functions:opam_cmds.bzl",
     "opam_get_current_switch_libdir",
     "opam_get_current_switch_prefix")
load("//opam/_functions:xdg.bzl", "get_xdg_paths")

#####################################
def _install_opam_symlinks(repo_ctx, opam_switch_prefix):
    repo_ctx.report_progress("symlinking opam toolchain")

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

    print("_opam_toolchain_impl (repo rule)")
    debug = False
    # if (ctx.label.name == "zexe_backend_common"):
    #     debug = True

    opam_switch_prefix = opam_get_current_switch_prefix(repo_ctx)

    _install_opam_symlinks(repo_ctx, opam_switch_prefix)

##################################
_opam_toolchain = repository_rule(
    doc = "Create an opam-based ocaml toolchain",
    implementation = _opam_toolchain_impl,
    attrs = dict(
        switch = attr.string(
            ## TODO: by default we use the current switch. support
            ## switch selection.
        ),
    ),
)

def opam_toolchain():
    # print("opam_toolchain (fn)")

    # repository rule creates symlinks to opam switch:
    # _opam_toolchain(name="ocaml.toolchain")

    # toolchains depend on targets in @ocaml.toolchain
    native.register_toolchains("//toolchain:ocaml_macos")
    native.register_toolchains("//toolchain:ocaml_linux_x86_64")

    ## TODO: one toolchain for each supported platform
