load("opam_ops.bzl",
     "opam_install_pkg",
     "is_pkg_installed",
     "file_exists", "run_cmd", "print_cwd", "print_tree")
load("opam_checksums.bzl", "sha256")
load("colors.bzl",
     "CCRED", "CCYEL", "CCYELBGH", "CCRESET")

arch_map = {
    "x86": "i686",
    "x86_64": "x86_64",
    "amd64": "x86_64",
    "aarch64_be": "arm64",
    "aarch64": "arm64",

    #ppcle|ppc64le) ARCH="ppc64le";;
    #s390x) ARCH="s390x";;
    # armv5*|armv6*|earmv6*|armv7*|earmv7*| "armhf"
    "armv8b": "armhf",
    "armv8l": "armhf"
}

os_map = {
    "mac os x": "macos",
    "linux": "linux"
}

XDG_ROOT    = "/Users/gar/.local/share/obazl"
XDP_OPAM_CTX = "/Users/gar/.local/share/obazl/opam"

################
def _install_opam(mctx, XDG_OPAM_BINDIR, opam_version):
    cmd = ["mkdir", "-vp", XDG_OPAM_BINDIR]
    run_cmd(mctx, cmd)
    #### download opam
    OPAM_BIN_URL_BASE='https://github.com/ocaml/opam/releases/download'
    # tag = "2.3.0"
    arch = arch_map[mctx.os.arch]
    os   = os_map[mctx.os.name]

    OPAM_BIN="opam-{TAG}-{ARCH}-{OS}".format(
        TAG=opam_version,
        ARCH=arch,
        OS=os
    )
    OPAM_BIN_URL="{BASE}/{TAG}/{BIN}".format(
        BASE=OPAM_BIN_URL_BASE,
        TAG=opam_version,
        BIN=OPAM_BIN
    )

    SHA256 = sha256[OPAM_BIN]

    mctx.report_progress("Downloading: %s" % OPAM_BIN_URL)
    mctx.download(
        url = OPAM_BIN_URL,
        output = "./bin/opam", # .format(OPAM_BIN),
        executable = True,
        sha256 = SHA256
    )

    cmd = ["cp", "-v", "./bin/opam", XDG_OPAM_BINDIR]
    run_cmd(mctx, cmd)

################
def _init_opam(mctx, opambin, opam_version, OPAMROOT, verbosity):

    cmd = ["mkdir", "-vp", OPAMROOT]

    # print("OPAM ROOT PATH: %s" % OPAMROOT)

    cmd = [opambin,
           "init",
           "--root={}".format(OPAMROOT),
           "--bare",
           "--no-setup", # don't update shell stuff
           "--no-opamrc",
           "--no" # answer no to q about modifying shell rcfiles
           ]
    mctx.report_progress("""
{c}INFO{reset}: Initializing OPAM {v} root at {r}
    """.format(c = CCYEL, reset = CCRESET,
               v = opam_version, r = OPAMROOT)
                         )
    res = mctx.execute(cmd, quiet = verbosity < 1)
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

################
def _create_switch(mctx, opambin, opam_version,
                   OPAMROOT, switch_id,
                   opam_verbosity, verbosity):

    cmd = [opambin,
           "switch",
           "create",
           switch_id,
           switch_id, ## compiler version
           "--root={}".format(OPAMROOT),
           # "--verbose"
           ]
    mctx.report_progress("""
{c}INFO{reset}: Creating opam switch {s} with OCaml version {v}
      in OPAM {o} root {r}
    """.format(c = CCYEL, reset = CCRESET,
               s = switch_id, v = switch_id,
               o = opam_version, r = OPAMROOT))
    res = mctx.execute(cmd,
                       environment = {
                           "PATH": "/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/usr/sbin"
                       },
                       quiet = verbosity < 1)
    if res.return_code != 0:
        if res.return_code != 2: # already installed
            print("cmd: %s" % cmd)
            print("rc: %s" % res.return_code)
            print("stdout: %s" % res.stdout)
            print("stderr: %s" % res.stderr)
            fail("cmd failure: %s" % cmd)

    ocaml_version = None

    return switch_id, ocaml_version

################
def config_xdg_toolchain(mctx,
                         opam_version,
                         ocaml_version,
                         pkgs,
                         debug,
                         opam_verbosity, verbosity):
    if debug > 0: print("config_xdg_toolchain")

    #### make Bazel aware that this dir is to be preserved?
    mctx.file("REPO.bazel", content = "")

    XDG_OPAM_BINDIR = "{}/{}/bin".format(
        XDP_OPAM_CTX, opam_version
    )
    opambin = XDG_OPAM_BINDIR + "/opam"

    # cmd = ["file", "-E", "-b", "xxx{}".format(opambin)]
    # check = run_cmd(mctx, cmd) #, verbosity=1)
    # print("check opam binary: '%s'" % check)

    if not file_exists(mctx, opambin):
        _install_opam(mctx, XDG_OPAM_BINDIR, opam_version)

    OPAMROOT = "{}/{}/root".format(
        XDP_OPAM_CTX, opam_version
    )
    # cmd = ["file", "-b", "{}".format(OPAMROOT)]
    # check = run_cmd(mctx, cmd) #, verbosity=1)
    # print("check root: '%s'" % check)
    # if check != "directory":

    if not file_exists(mctx, OPAMROOT):
        _init_opam(mctx, opambin, opam_version, OPAMROOT, verbosity)

    if ocaml_version:
        switch_id = ocaml_version
        if not file_exists(mctx, "{}/{}".format(OPAMROOT, switch_id)):
            (switch_id,
             ocaml_version) = _create_switch(mctx, opambin,
                                             opam_version,
                                             OPAMROOT,
                                             ocaml_version, # switch_id,
                                             opam_verbosity,
                                             verbosity)
        # else:
            # found switch in xdg env
    else:
        # no ocaml_version specified, use default
        cmd = ["opam", "var", "sys-ocaml-version",
               "--root", OPAMROOT]
        res = mctx.execute(cmd,
                           # environment = {
                           #     "PATH": "/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/usr/sbin"
                           # },
                           quiet = (opam_verbosity < 1))
    if res.return_code != 0:

        (switch_id,
         ocaml_version) = _create_switch(mctx, opambin,
                                         opam_version, OPAMROOT,
                                         None, # switch_id,
                                         opam_verbosity,
                                         verbosity)

    cmd = [opambin, "var", "prefix", "--root", OPAMROOT,
           "--switch", switch_id]
    res = mctx.execute(cmd)
    if res.return_code == 0:
        switch_pfx = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("stdout: {stdout}".format(stdout= res.stdout))
        print("stderr: {stderr}".format(stderr= res.stderr))
        fail("cmd failure.")
    if debug > 0: print("switch pfx: %s" % switch_pfx)

    cmd = [opambin, "var", "bin", "--root", OPAMROOT,
           "--switch", switch_id]
    res = mctx.execute(cmd)
    if res.return_code == 0:
        switch_bin = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("stdout: {stdout}".format(stdout= res.stdout))
        print("stderr: {stderr}".format(stderr= res.stderr))
        fail("cmd failure.")
    if debug > 0: print("switch bin: %s" % switch_bin)

    SDKBIN = switch_bin

    tot = len(pkgs)
    for i,pkg in enumerate(pkgs):
        if verbosity > 1: print("Installing pkg '{}'".format(pkg))
        if not is_pkg_installed(mctx, opambin, pkg,
                                OPAMROOT, ocaml_version):
            opam_install_pkg(mctx,
                             opambin,
                             pkg,
                             switch_id,
                             switch_pfx,
                             SDKBIN,
                             OPAMROOT,
                             i, tot,
                             debug, verbosity)

    # get all installed pkgs
    cmd = [opambin,
           "var", "lib",
           "--switch", ocaml_version,
           "--root", "{}".format(OPAMROOT),
          "--yes"]
    switch_lib = None
    res = mctx.execute(cmd, quiet = (verbosity < 1))
    if res.return_code == 0:
        switch_lib = res.stdout.strip()
    else:
            print("cmd: %s" % cmd)
            print("rc: %s" % res.return_code)
            print("stdout: %s" % res.stdout)
            print("stderr: %s" % res.stderr)
            fail("cmd failure")

    if debug > 1: print("switch_lib: %s" % switch_lib)
    cmd = ["ls", "-1", "{}/{}/lib".format(OPAMROOT, switch_id)]
    deps = run_cmd(mctx, cmd) ## , verbosity=0)
    deps = deps.splitlines()
    torem = []
    for dep in deps:
        if dep.endswith(".conf"):
            torem.append(dep)
    for x in torem:
        deps.remove(x)

    return (str(opambin), str(OPAMROOT),
            switch_lib, # sdklib
            switch_id,
            ocaml_version, deps)
