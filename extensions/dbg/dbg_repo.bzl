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
        """.format(rctx.attr.dbg_version)
    )
    # bazel_dep(name = "rules_cc", version = "0.1.1")

    rctx.template("dbg_runner.bzl",
                  rctx.attr._dbg_runner)

    rctx.file("BUILD.bazel",
        content = """
load(":dbg_runner.bzl", "ocamldebug_runner")

alias(name = "dbg", actual="dbg_runner")

ocamldebug_runner(
    name = "dbg_runner",
    data = ["@dbg//x"],
    args = ["$(location @dbg//x)"],
    env  = {{
        "OPAMROOT": "{root}",
        "OPAMSWITCH": "{switch}",
        "OCAML_TOPLEVEL_PATH": "",
        }},
)
        """.format(
            # inifile = rctx.attr.ini_file,
            # ld_path = rctx.attr.ld_lib_path,
            root = rctx.attr.opam_root,
            switch = rctx.attr.switch_id
        )
              )
        # "CAML_LD_LIBRARY_PATH": "{ld_path}"


    ## bazel run @dbg --@dbg//x=foo/bar:baz.exe
    rctx.file("x/BUILD.bazel",
        content = """

label_flag(
        name = "x",
        build_setting_default = "@rules_ocaml//cfg:nil",
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
        # "ld_lib_path": attr.string(
        #     mandatory = True
        # ),
        "debug": attr.int(default=0),
        "verbosity": attr.int(default=0)
    },
)
