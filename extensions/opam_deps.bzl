OBAZL_PKGS = [
    "ocamlsdk",
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
    "findlib",
    "stublibs"
]
    # if repo_pkg not in ["ocamlsdk",


def _throw_cmd_error(cmd, r):
    print("cmd: %s" % cmd)
    print("stdout: {stdout}".format(stdout= r.stdout))
    print("stderr: {stderr}".format(stderr= r.stderr))
    fail("cmd failure.")

def _opam_install_pkg(rctx, xopam, repo_pkg,  ocaml_version, root,
                      debug, verbosity):
    if repo_pkg not in OBAZL_PKGS:
        rctx.report_progress("Installing pkg %s" % repo_pkg)
        cmd = [xopam,
               "install",
               repo_pkg,
               "--switch", ocaml_version,
               "--root", "{}".format(root),
               "--yes"]

        res = rctx.execute(cmd, quiet = (verbosity < 1))
        if res.return_code == 0:
            if rctx.attr.debug > 0:
                print("pkg installed: %s" % repo_pkg)
        else:
            print("cmd: %s" % cmd)
            print("rc: %s" % res.return_code)
            print("stdout: %s" % res.stdout)
            print("stderr: %s" % res.stderr)
            fail("cmd failure")

##############################
def _opam_dep_repo_impl(repo_ctx):
    ## repo_cts.name == tools_opam++opam+opam.ounit2
    debug     = repo_ctx.attr.debug
    verbosity = repo_ctx.attr.verbosity

    if debug > 1:
        print("OPAM Configuring " + repo_ctx.name)

    if repo_ctx.attr.xopam == None:
        # local sdk
        xopam = repo_ctx.which("opam")
        root  = None
    else:
        # sdk downloaded
        xopam = repo_ctx.attr.xopam
        cmd = ["dirname", xopam]
        res = repo_ctx.execute(cmd,
                               quiet = (verbosity < 1))
        if res.return_code == 0:
            opam_dirname = res.stdout.strip()
            if repo_ctx.attr.debug > 1:
                print("opam_dirname: %s" % opam_dirname)
        else:
            print("rc: %s" % res.return_code)
            print("stdout: %s" % res.stdout)
            print("stderr: %s" % res.stderr)
            fail("cmd failure: %s" % cmd)
        cwd = repo_ctx.path(opam_dirname)
        root = repo_ctx.path(str(cwd.dirname) + "/.opam")

    if repo_ctx.attr.debug > 0: print("XOPAM: %s" % xopam)
    if repo_ctx.attr.debug > 1: print("ROOT: %s" % root)

    opambin = repo_ctx.path(xopam)
    if repo_ctx.attr.debug > 1:
        print("OPAMBIN: %s" % opambin)
    # switch = opambin.dirname.dirname
    switch = repo_ctx.attr.ocaml_version
    # print("OPAMREPO ROOT: %s" % opam_repo_root)
    # switch = repo_ctx.path("./")
    if repo_ctx.attr.debug > 1:
        print("LOCAL SWITCH: %s" % switch)
    cmd = [xopam, "var", "prefix",
           "--switch", "{}".format(switch)]
    if root:
        cmd.extend(["--root", "{}".format(root)])
    res = repo_ctx.execute(cmd, quiet = (verbosity < 1))
    if res.return_code == 0:
        switch_pfx = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")
    if repo_ctx.attr.debug > 1:
        print("SWITCH PFX %s" % switch_pfx)

    repo = repo_ctx.name.removeprefix("tools_opam++opam+")
    repo_pkg = repo.removeprefix("opam.")

    ## Running repo_ctx.execute cmd creates the repo
    ## so the ensuing symlink would fail with 'already exists'
    ## so we delete it before creating the symlink:

    repo_ctx.delete(".")

    if root != None:
        # sdk downloaded
        if repo_ctx.attr.install:
            _opam_install_pkg(repo_ctx, xopam, repo_pkg,  repo_ctx.attr.ocaml_version, root, debug, verbosity)

    cmd = [repo_ctx.attr.tool,
           "--pkg", repo_pkg,
           "--switch-pfx", switch_pfx]
           # "--switch", repo_ctx.attr.switch_id]
    if repo_ctx.attr.ocaml_version:
        cmd.extend(["--ocaml-version", repo_ctx.attr.ocaml_version])

    repo_ctx.report_progress("Configuring pkg %s" % repo_pkg)
    _pkg_deps = repo_ctx.execute(cmd)
    if _pkg_deps.return_code == 0:
        _pkg_deps = _pkg_deps.stdout.strip()
        # if repo_ctx.attr.debug > 0:
            # print("pkg {} deps: {}".format(repo_pkg, _pkg_deps))
    else:
        print("cmd {cmd} rc    : {rc}".format(
            cmd=cmd,
            rc= _pkg_deps.return_code))
        _throw_cmd_error(cmd, _pkg_deps)

###########################
opam_dep = repository_rule(
    implementation = _opam_dep_repo_impl,
    attrs = {
        "install": attr.bool(default = True),
        "xopam": attr.label(),
        "ocaml_version": attr.string(),
        # "switch_id": attr.string(mandatory = True),
        # "switch_pfx": attr.string(mandatory = True),
        # "switch_lib": attr.string(mandatory = True),
        "pkg_version": attr.string(
            mandatory = False,
        ),
        "tool": attr.string(),
        "debug": attr.int(default=0),
        "verbosity": attr.int(default=0),
    },
)

