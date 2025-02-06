load("opam_dep.bzl", "OBAZL_PKGS")
load("opam_ops.bzl", "opam_install_pkg",
     "print_cwd", "print_tree", "run_cmd")
load("opam_checksums.bzl", "sha256")

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

##############################
def _opam_repo_impl(rctx):
    print("OPAM REPO running")

    # if tc == local: created if needed, symlink
    # elif tc == global, symlink to opam binary & root
    # else: embedded, proceed as follows:

    rctx.file("REPO.bazel", content = "")

    rctx.file(
        "MODULE.bazel",
        content = """
module(
    name = "opam",
    version = "{}",
)
        """.format(rctx.attr.opam_version)
    )

    rctx.file("bin/BUILD.bazel",
        content = """
exports_files(["opam", "ocamlfind"])
        """)

    #### download opam
    OPAM_BIN_URL_BASE='https://github.com/ocaml/opam/releases/download'
    # tag = "2.3.0"
    arch = arch_map[rctx.os.arch]
    os   = os_map[rctx.os.name]

    OPAM_BIN="opam-{TAG}-{ARCH}-{OS}".format(
        TAG=rctx.attr.opam_version,
        ARCH=arch,
        OS=os
    )
    OPAM_BIN_URL="{BASE}/{TAG}/{BIN}".format(
        BASE=OPAM_BIN_URL_BASE,
        TAG=rctx.attr.opam_version,
        BIN=OPAM_BIN
    )

    SHA256 = sha256[OPAM_BIN]

    rctx.report_progress("Downloading: %s" % OPAM_BIN_URL)
    rctx.download(
        url = OPAM_BIN_URL,
        output = "./bin/opam", # .format(OPAM_BIN),
        executable = True,
        sha256 = SHA256
    )
    if rctx.attr.debug > 1:
        print_cwd(rctx)
        print_tree(rctx)
    opambin = rctx.path("./bin/opam")
    print("OPAMBIN dl: %s" % opambin)
    run_cmd(rctx, ["bin/opam", "--version"])

    root = rctx.path("./.opam")
    rctx.symlink("./.opam", "./config/root")
    if rctx.attr.debug > 1:
        print("OPAM ROOT: %s" % root)
    # root = root + ".opam"
    # print("ROOT: %s" % root)

    # if rctx.attr.root == None:
    cmd = [opambin,
           "init",
           "--root={}".format(root),
           "--bare",
           "--no-setup", # don't update shell stuff
           "--no-opamrc",
           "--no" # answer no to q about modifying shell rcfiles
           ]
    if rctx.attr.debug > 0:
        print("Initializing opam root:\n%s" % cmd)
    rctx.report_progress("Initializing opam root: {}".format(
        root
    ))
    res = rctx.execute(cmd,
                           quiet = rctx.attr.verbosity < 1)
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

    cmd = [opambin,
           "switch",
           "create",
           rctx.attr.ocaml_version,
           "--root={}".format(root),
           # "--verbose"
           ]
    if rctx.attr.debug > 0: print("Creating switch:\n%s" % cmd)
    rctx.report_progress("Creating opam switch %s" % rctx.attr.ocaml_version)
    res = rctx.execute(cmd, quiet = rctx.attr.verbosity < 1)
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure: %s" % cmd)

    if rctx.attr.debug > 1:
        print_cwd(rctx)
        print_tree(rctx, root, 1)

    switch = None
    cmd = [opambin, "var", "switch",
           "--root",  root]
    res = rctx.execute(cmd, quiet = rctx.attr.verbosity < 1)
    if res.return_code == 0:
        switch = res.stdout
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure: %s" % cmd)

    print("OPAM SWITCH %s" % switch)

    ## FIXME: do we really need ocamlfind?
    ## install ocamlfind
    rctx.report_progress("Installing pkg 'ocamlfind'")
    cmd = [opambin,
           "install",
           "ocamlfind",
           "--switch", rctx.attr.ocaml_version,
           "--root", "{}".format(root),
           "--yes"]

    res = rctx.execute(cmd, quiet = (rctx.attr.verbosity < 1))
    if res.return_code == 0:
        if rctx.attr.debug > 0:
            print("ocamlfind installed: ocamlfind")
        else:
            print("cmd: %s" % cmd)
            print("rc: %s" % res.return_code)
            print("stdout: %s" % res.stdout)
            print("stderr: %s" % res.stderr)
            fail("cmd failure")

    ## now get path to ocamlfind
    cmd = [opambin,
           "var",
           "ocamlfind:bin",
           "--switch", rctx.attr.ocaml_version,
           "--root", "{}".format(root),
          "--yes"]
    ocamlfind = None
    res = rctx.execute(cmd, quiet = (rctx.attr.verbosity < 1))
    if res.return_code == 0:
        ocamlfind = res.stdout.strip() + "/ocamlfind"
        if rctx.attr.debug > 0: print("ocamlfind bin: %s" % ocamlfind)
        else:
            print("cmd: %s" % cmd)
            print("rc: %s" % res.return_code)
            print("stdout: %s" % res.stdout)
            print("stderr: %s" % res.stderr)
            fail("cmd failure")

    rctx.symlink(ocamlfind, "bin/ocamlfind")

    rctx.file("config/BUILD.bazel",
        content = """
exports_files(["root", "switch"])
# load("@bazel_skylib//rules:common_settings.bzl", "string_setting")
# string_setting(name = "root", build_setting_default = {})
# string_setting(name = "switch", build_setting_default = {})
        """.format(root, rctx.attr.ocaml_version))

    for pkg in rctx.attr.pkgs:
        opam_install_pkg(rctx,
                          opambin,
                          pkg,  rctx.attr.ocaml_version,
                          root,
                          rctx.attr.debug,
                          rctx.attr.verbosity)

    if rctx.attr.debug > 1:
        print_cwd(rctx)
        print_tree(rctx, root, 1)

###############################
opam_repo = repository_rule(
    implementation = _opam_repo_impl,
    attrs = {
        "toolchain": attr.string(),
        "opam_version": attr.string(),
        "ocaml_version": attr.string(),
        "pkgs": attr.string_list(
            mandatory = True,
            allow_empty = False
        ),
        "debug": attr.int(default=0),
        "verbosity": attr.int(default=0)
    },
)
