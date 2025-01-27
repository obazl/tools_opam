def _opam_repo_impl(repo_ctx):
    # print("opam repo")

    # (seg1, sep, seg2) = repo_ctx.name.partition("~")
    # (version, sep, seg3) = seg2.partition("~")
    # (ext, sep, repo_name) = seg3.partition("~")

    ## testing only
    repo_ctx.file(
        "src/BUILD.bazel",
        content = """
cc_library(
  name = "test",
  srcs = ["foo.c"],
)
        """
    )

    repo_ctx.file(
        "src/foo.c",
        content = "int x;",
        executable = False,
    )


    #### download opam
    ## FIXME: install.sh only installs to /usr/local/bin
    ## We need to choose and download a binary directly
    ## from https://github.com/ocaml/opam/releases
    ## OR, download only and figure out how to chmod u+x

    repo_ctx.report_progress("Downloading opam")
    res = repo_ctx.download(
        ["https://raw.githubusercontent.com/ocaml/opam/master/shell/install.sh"],
        output = "./bin/opam_install.sh",
        executable = True,
        # sha256 = "",
        # integrity = ""
    )

    repo_ctx.file("bin/BUILD.bazel",
        content = """
exports_files(["opam"])
        """)

    repo_ctx.report_progress("Installing opam")
    cmd = ["./bin/opam_install.sh", "--download-only"]
    res = repo_ctx.execute(cmd,
                           quiet = repo_ctx.attr.verbosity < 1)
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure: %s" % cmd)

    #HACK: we need to chmod u+x the downloaded file,
    # but we do not know the file name.
    # This is the only way I could find to do this.
    res = repo_ctx.execute(["ls"])
    files = res.stdout.splitlines()
    opambin = None
    for f in files:
        if f.startswith("opam-"):
            # print("FILE: %s" % f)
            opambin = f
            cmd = ["chmod", "-v", "u+x", opambin]
            res = repo_ctx.execute(cmd, quiet = repo_ctx.attr.verbosity < 1)
            if res.return_code != 0:
                print("cmd: %s" % cmd)
                print("rc: %s" % res.return_code)
                print("stdout: %s" % res.stdout)
                print("stderr: %s" % res.stderr)
                fail("cmd failure: %s" % cmd)

            cmd = ["mv", "-v", opambin, "./bin/" + opambin]
            res = repo_ctx.execute(cmd, quiet = repo_ctx.attr.verbosity < 1)
            if res.return_code != 0:
                print("cmd: %s" % cmd)
                print("rc: %s" % res.return_code)
                print("stdout: %s" % res.stdout)
                print("stderr: %s" % res.stderr)
                fail("cmd failure: %s" % cmd)

            repo_ctx.symlink("./bin/" + opambin, "./bin/opam")

    ## now run 'opam switch create ...' etc.
    # res = repo_ctx.read("./root/config")
    # print("rc: %s" % res.return_code)
    # print("stdout: %s" % res.stdout)
    # print("stderr: %s" % res.stderr)
    root = repo_ctx.path("./.opam")
    if repo_ctx.attr.debug > 1:
        print("ROOT PATH: %s" % root)
    # root = root + ".opam"
    # print("ROOT: %s" % root)

    # if repo_ctx.attr.root == None:
    cmd = ["./bin/" + opambin,
           "init",
         "--root={}".format(root),
         "--bare",
         "--no-setup", # don't update shell stuff
         "--no-opamrc",
         "--no" # answer no to q about modifying shell rcfiles
         ]
    if repo_ctx.attr.debug > 0:
        print("Initializing opam root:\n%s" % cmd)
    repo_ctx.report_progress("Initializing opam root: {}".format(
        root
    ))
    res = repo_ctx.execute(cmd,
                           quiet = repo_ctx.attr.verbosity < 1)
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

    cmd = ["./bin/" + opambin,
           "switch",
           "create",
           repo_ctx.attr.ocaml_version,
           "--root={}".format(root),
           # "--verbose"
           ]
    if repo_ctx.attr.debug > 0:
        print("Creating switch:\n%s" % cmd)
    repo_ctx.report_progress("Creating opam switch %s" % repo_ctx.attr.ocaml_version)
    res = repo_ctx.execute(cmd, quiet = repo_ctx.attr.verbosity < 1)
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure: %s" % cmd)

    repo_ctx.file(
        "MODULE.bazel",
        content = """
module(
    name = "{repo_name}",
    version = "1.0.0",
    compatibility_level = 1,
)
""".format(repo_name = repo_ctx.name
           # repo_name
           )
    )

#     repo_ctx.file(
#         "var/BUILD.bzl",
#         content = """
# def _impl(ctx):
#     args = ctx.actions.args()
#     args.add("var")
#     args.add("--root=./root")
#     ctx.run(
#         executable = ctx.file._opam,
#         arguments = args,
#         outputs = outs
#     )
#     ## print to stdout???

# run_opam = rule(
#     implementation = _impl,
#     attrs = dict(
#         _opam = attr.label(default = "//bin:opam",
#             allow_single_file = True,
#             executable = True
#         )
# )
#         """
#     )

    repo_ctx.file(
        "var/BUILD.bazel",
        content = """
sh_binary(
    name = "var",
    srcs = ["//:var.sh"],
    data = ["//:opam"]
)
        """
    )

    repo_ctx.file(
        "var.sh",
        content = """
#!/bin/sh
set +x
./opam var --root=./root
        """,
        executable = True
    )

#     repo_ctx.file(
#         "var/BUILD.bazel",
#         content = """
# genrule(
#     name = "var",
#     cmd  = "
# )
#         """
#     )

#     repo_ctx.file(
#         "bin/BUILD.bazel",
#         content = """
# alias(
#     name = "opam",
#     actual = "//:opam",
#     visibility = ["//visibility:public"]
# )
#         """
# )

#     repo_ctx.file(
#         "BUILD.bazel",
#         content = """
# exports_files(["opam", "var.sh"])
# # alias(
# #     name = "opam",
# #     actual = "{opambin}",
# #     visibility = ["//visibility:public"]
# # )

# # load("@cc_config//:MACROS.bzl", "repo_paths")

# # PROD_REPOS = [
# # ]

# # repo_paths(
# #     name = "repo_paths",
# #     repos = PROD_REPOS
# # )

# # repo_paths(
# #     name = "test_repo_paths",
# #     repos = PROD_REPOS + [
# #     ]
# # )

###############################
x_opam_repo = repository_rule(
    implementation = _opam_repo_impl,
    attrs = {
        "opam_version": attr.string(),
        "ocaml_version": attr.string(),
        "debug": attr.int(default=0),
        "verbosity": attr.int(default=0)
        # "tool": attr.label(
        #     allow_single_file = True,
        #     # default = "//new:coswitch",
        #     executable = True,
        #     cfg = "exec"
        # )
        # "generating_repository": attr.string(default = "maven"),
        # "target_name": attr.string(),
    },
)

################
def download_and_config_toolchain(mctx,
                                  opam_version,
                                  ocaml_version,
                                  debug,
                                  verbosity):
    # print("download_and_config_toolchain")

    x_opam_repo(name = "opam",
                opam_version  = opam_version,
                ocaml_version = ocaml_version,
                debug = debug,
                verbosity = verbosity)

