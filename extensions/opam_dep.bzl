OBAZL_PKGS = [
    "ocamlsdk",
    ## ocaml psuedo-pkgs
    "compiler-libs",
    "dynlink",
    "ffi",
    "ocamldoc",
    "profiling",
    "runtime_events",
    "stdlib",
    "str",
    "threads",
    "unix",
    ## special cases
    "findlib",
    "stublibs"
]
    # if repo_pkg not in ["ocamlsdk",


# def _opam_install_pkg(rctx,
#                       xopam,
#                       repo_pkg,  ocaml_version, root,
#                       debug, verbosity):
#     if repo_pkg not in OBAZL_PKGS:
#         rctx.report_progress("Installing pkg %s" % repo_pkg)
#         cmd = [xopam,
#                "install",
#                repo_pkg,
#                "--switch", ocaml_version,
#                "--root", "{}".format(root),
#                "--yes"]

#         res = rctx.execute(cmd, quiet = (verbosity < 1))
#         if res.return_code == 0:
#             if rctx.attr.debug > 0:
#                 print("pkg installed: %s" % repo_pkg)
#         else:
#             print("cmd: %s" % cmd)
#             print("rc: %s" % res.return_code)
#             print("stdout: %s" % res.stdout)
#             print("stderr: %s" % res.stderr)
#             fail("cmd failure")

##############################
def _opam_dep_repo_impl(rctx):
    ## repo_cts.name == tools_opam++opam+opam.ounit2
    debug      = rctx.attr.debug
    opam_verbosity = rctx.attr.opam_verbosity
    verbosity  = rctx.attr.verbosity
    if debug > 1: print("opam_dep(%s)" % rctx.name)

#     rctx.file("MODULE.bazel",
#               content = """
# bazel_dep(name = "tools_opam", version = "1.0.0")
#               """)

    if verbosity > 0:
        print("\n  Creating repo: '" + rctx.name + "'")

    opambin = rctx.path(rctx.attr.opam)
    if debug > 1: print("OPAMBIN: %s" % opambin)

    opamroot = rctx.path(rctx.attr.root)
    # opamroot = None
    # cmd = [opambin, "var", "root"]
    # res = rctx.execute(cmd, quiet = (verbosity < 1))
    # if res.return_code == 0:
    #     opamroot = res.stdout.strip()
    # else:
    #     print("cmd: %s" % cmd)
    #     print("rc: %s" % res.return_code)
    #     print("stdout: %s" % res.stdout)
    #     print("stderr: %s" % res.stderr)
    #     fail("cmd failure")

    if debug > 1: print("OPAMROOT: %s" % opamroot)
    opamswitch = rctx.attr.switch
    if debug > 1: print("OPAMSWITCH: %s" % opamswitch)

    # config tool needs switch prefix
    cmd = [opambin, "var", "prefix",
           "--switch", "{}".format(opamswitch),
           "--root", opamroot]
    res = rctx.execute(cmd, quiet = (opam_verbosity < 1))
    switch_pfx = None
    if res.return_code == 0:
        switch_pfx = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

    # if rctx.attr.debug > 1:
    #     print("SWITCH PFX %s" % switch_pfx)

    repo = rctx.name.removeprefix("tools_opam++opam+")
    repo_pkg = repo.removeprefix(rctx.attr.obazl_pfx)

    ## Running rctx.execute cmd creates the repo
    ## so the ensuing symlink would fail with 'already exists'
    ## so we delete it before creating the symlink:

    rctx.delete(".")

    # if xroot != None:
    #     # sdk downloaded
    #     if rctx.attr.install:
    #         _opam_install_pkg(rctx,
    #                           "@opam//bin:opam",
    #                           repo_pkg,  rctx.attr.ocaml_version, root, debug, verbosity)

    # if debug > 0: print("\nSDKLIB: %s" % rctx.attr.sdklib)
    if debug > 0: print("Calling config tool: {}{}, {}".format(
        rctx.attr.obazl_pfx,
        repo_pkg,
        switch_pfx)
    )
    cmd = [rctx.attr.config_tool,
           "--pkg", repo_pkg,
           "--pkg-pfx", rctx.attr.obazl_pfx,
           "--switch-pfx", switch_pfx]
    if rctx.attr.ocaml_version:
        cmd.extend(["--ocaml-version", rctx.attr.ocaml_version])
    rctx.report_progress("Configuring pkg %s" % repo_pkg)
    res = rctx.execute(cmd,
                       environment = {
                           "OBAZL_SDKLIB": rctx.attr.sdklib
                       },
                       quiet = (verbosity < 1))

    if res.return_code == 0:
        _pkg_deps = res.stdout.strip()
        # if rctx.attr.debug > 0:
            # print("pkg {} deps: {}".format(repo_pkg, _pkg_deps))
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

###########################
opam_dep = repository_rule(
    implementation = _opam_dep_repo_impl,
    attrs = {
        "install": attr.bool(default = True),
        "opam": attr.string(),
        "root": attr.string(),
        "sdklib": attr.string(),
        "switch": attr.string(),
        # "switch_pfx": attr.string(),
        # "switch_lib": attr.string(),
        "ocaml_version": attr.string(),
        "obazl_pfx": attr.string(),
        # "switch_id": attr.string(mandatory = True),
        # "switch_pfx": attr.string(mandatory = True),
        # "switch_lib": attr.string(mandatory = True),
        "pkg_version": attr.string(
            mandatory = False,
        ),
        "config_tool": attr.string(),
        "debug":      attr.int(default=0),
        "opam_verbosity": attr.int(default=0),
        "verbosity":  attr.int(default=0),
    },
)

