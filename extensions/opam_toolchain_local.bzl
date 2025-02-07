load("opam_ops.bzl",
     "file_exists",
     "is_pkg_installed",
     "opam_install_pkg",
     "run_cmd")
load("opam_utils.bzl", "get_sdk_lib")

load("colors.bzl",
     "CCRED", "CCYEL", "CCRESET")

##########################################
def _opam_create_local_switch(ctx, opambin,
                              ocaml_version,
                              proj_root,
                              debug, verbosity):

    switch_id = proj_root

    cmd = [opambin, "switch",
           "create", ".", ocaml_version,
           "--deps-only",
           "--no-switch" # do not automatically select
           ]
    if verbosity > 2:
        cmd.extend(["--verbose"])
    if debug > 0: print("Creating switch:\n%s" % cmd)
    ctx.report_progress("Creating local switch for compiler {version}, at {id}".format(
        id=switch_id, version=ocaml_version))
    res = ctx.execute(cmd,
                      working_directory = str(proj_root),
                      quiet = (verbosity < 1))
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure: %s" % cmd)

    return switch_id

################################
def config_local_toolchain(mctx,
                           ocaml_version,
                           deps,
                           debug, verbosity):
    if debug > 0: print("\nconfig_local_toolchain")

    opambin = mctx.which("opam")
    if debug > 0: print("\nOPAM: %s" % opambin)

    ## in the output_base dir:
    p = mctx.path("../../DO_NOT_BUILD_HERE")
    proj_root = mctx.path(mctx.read(p))
    if not proj_root.exists:
        fail("WSROOT NOT FOUND: %s" % proj_root)

    switch_pfx = mctx.path(str(proj_root) + "/_opam")
    if switch_pfx.exists:
        # verify ocaml version
        cmd = [opambin, "var", "ocaml:version",
               "--switch", proj_root,]
        res = mctx.execute(cmd,
                           environment = {
                               "PATH": "/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/usr/sbin"
                           },
                           quiet = (verbosity < 1))
        if res.return_code == 0:
            switch =  proj_root
            installed_version = res.stdout.strip()
            if ocaml_version:
                if installed_version != ocaml_version:
                    print("""

{c}WARNING{reset}: You have requested the local switch with OCaml version {y}{v1}{reset},
but your local switch ('_opam') uses version {y}{v2}{reset}.

I will proceed with the build using your local switch.

To remove this warning, either:

    a) set 'ocaml_version" (in your MODULE.bazel file) to "{v2}",
    b) remove the 'ocaml_version' attribute, or
    c) delete the local switch ("./_opam") so I can create a fresh local switch using OCaml version {v1}.
                    """.format(
                        c=CCRED, reset=CCRESET, y = CCYEL,
                        v1=ocaml_version, v2=installed_version)
                          )
                else:
                    switch = proj_root
            else:
                ocaml_version = installed_version
        else:
            print("cmd: %s" % cmd)
            print("stdout: {stdout}".format(stdout= res.stdout))
            print("stderr: {stderr}".format(stderr= res.stderr))
            fail("cmd failure.")

    else:
        if debug > 0: print("\nLOCAL SWITCH PFX NOT FOUND: %s" % switch_pfx)
        switch = _opam_create_local_switch(mctx, opambin,
                                           ocaml_version,
                                           proj_root,
                                           debug, verbosity)
        # switch will be path to proj root dir
        switch_pfx = str(switch) + "/_opam"

    if debug > 1: print("\nlocal switch: %s" % switch)

    # now we have a local switch. next task is to find
    # the sdk lib, e.g. <>/lib/ocaml.

    # if the swith uses the system-compiler, the sdk lib
    # will be e.g. /opt/homebrew/lib/ocaml.

    # cannot rely on 'opam exec' since it relies on the
    # root config file, which may be out of sync.

    # only reliable method I can see is to look for
    # _opam/.opam-switch/config/ocaml-system.config
    SDKLIB = get_sdk_lib(mctx, opambin, switch_pfx, debug)
    SDKBIN = SDKLIB.removesuffix("/lib") + "/bin"
    if debug > 0: print("SDKBIN: %s" % SDKBIN)

    ## now get the OPAMROOT
    cmd = [opambin, "var", "root"]
    res = mctx.execute(cmd)
    if res.return_code == 0:
        OPAMROOT = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("stdout: {stdout}".format(stdout= res.stdout))
        print("stderr: {stderr}".format(stderr= res.stderr))
        fail("cmd failure.")

    cmd = [opambin, "var", "bin", "--switch", switch]
    res = mctx.execute(cmd)
    if res.return_code == 0:
        switch_bin = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("stdout: {stdout}".format(stdout= res.stdout))
        print("stderr: {stderr}".format(stderr= res.stderr))
        fail("cmd failure.")

    cmd = [opambin, "var", "lib", "--switch", switch]
    res = mctx.execute(cmd)
    if res.return_code == 0:
        switch_lib = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("stdout: {stdout}".format(stdout= res.stdout))
        print("stderr: {stderr}".format(stderr= res.stderr))
        fail("cmd failure.")

    tot = len(deps)
    for i, pkg in enumerate(deps):
        pkg_path = "{}/{}".format(switch_lib, pkg)
        if not file_exists(mctx, pkg_path):
            if verbosity > 1: print("Installing pkg '{}'".format(pkg))
            opam_install_pkg(mctx,
                             opambin,
                             pkg,
                             switch,
                             switch_pfx,
                             SDKBIN,
                             OPAMROOT,
                             i, tot,
                             debug, verbosity)

    # get all installed pkgs
    cmd = ["ls", "-1", "{}".format(switch_lib)]
    deps = run_cmd(mctx, cmd) ## , verbosity=0)
    deps = deps.splitlines()
    torem = []
    for dep in deps:
        if dep.endswith(".conf"):
            torem.append(dep)
    for x in torem:
        deps.remove(x)

    return (opambin, OPAMROOT, SDKLIB,
            str(switch), ocaml_version, deps)

