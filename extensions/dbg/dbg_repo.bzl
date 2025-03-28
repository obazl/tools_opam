# load("//extensions:opam_ops.bzl",
#      "print_cwd", "print_tree", "run_cmd")

##############################
def _dbg_repo_impl(rctx):
    # if tc == local: created if needed, symlink
    # elif tc == global, symlink to opam binary & root
    # else: embedded, proceed as follows:

    rctx.file("REPO.bazel", content = "")

    rctx.file(
        "MODULE.bazel",
        content = """
module(
    name = "dbg",
    version = "{}",
)
bazel_dep(name = "rules_cc", version = "0.1.1")
        """.format(rctx.attr.dbg_version)
    )

    rctx.template("ocamldebug_runner.c",
                  rctx.attr._ocamldebug_runner)

    rctx.template("dbg_runner.bzl",
                  rctx.attr._dbg_runner)

    rctx.file("BUILD.bazel",
        content = """
load(":dbg_runner.bzl", "ocamldebug_runner")

alias(name = "dbg", actual="dbg_runner")

cc_binary(
    name = "ocamldebug_runner",
    srcs = ["ocamldebug_runner.c"],
    copts = ["-Wno-null-character"],
    args = [
        "$(location @opam.ocamlsdk//bin:ocamldebug)"
    ],
    data = ["@opam.ocamlsdk//bin:ocamldebug"],
    env  = {{
        "OPAMROOT": "{root}",
        "OPAMSWITCH": "{switch}",
        }}
)

ocamldebug_runner(
    name = "dbg_runner",
    runner = ":ocamldebug_runner",
    bin = "@opam.ocamlsdk//bin:ocamldebug",
    env  = {{
        "OPAMROOT": "{root}",
        "OPAMSWITCH": "{switch}",
        }},
)
        """.format(
            # inifile = rctx.attr.ini_file,
            # ld_path = rctx.attr.ld_lib_path,
            root = rctx.attr.opam_root,
            switch = rctx.attr.switch_id,
        )
              )
        # "CAML_LD_LIBRARY_PATH": "{ld_path}"
#        "CAML_LD_LIBRARY_PATH": "{ld_path}"}},


    ## bazel run @dbg --@dbg//pgm=foo/bar:baz.exe
    rctx.file("pgm/dummy", content = "")

    rctx.file("pgm/BUILD.bazel",
        content = """

    # rctx.file("pgm", content="")

label_flag(
        name = "pgm",
        build_setting_default = "@dbg//pgm:dummy",
        visibility = ["@dbg//:__pkg__"],
)
        """
              )

###############################
dbg_repo = repository_rule(
    implementation = _dbg_repo_impl,
    attrs = {
        "root_module": attr.label(),
        "opam_root": attr.string(),
        "switch_id": attr.string(),
        # "ini_file": attr.label(),
        "dbg_version": attr.string(),
        "_dbg_runner": attr.label(
            default = "dbg_runner.bzl"
        ),
        # "ocamldebug_bin": attr.string(
        #     # mandatory=True
        # ),
        "_ocamldebug_runner": attr.label(
            default = "ocamldebug_runner.c"
        ),
        # "ld_lib_path": attr.string(
        #     mandatory = True
        # ),
        "debug": attr.int(default=0),
        "verbosity": attr.int(default=0)
    },
)
