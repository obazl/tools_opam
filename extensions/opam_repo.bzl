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
bazel_dep(name = "rules_cc", version = "0.1.1")
        """.format(rctx.attr.opam_version)
    )

    rctx.file(
        "opam_runner.c",
        content = """
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define YEL "\033[0;33m"
#define RED "\033[0;31m"
#define CRESET "\033[0m"
int main(int argc, char *argv[])
{{
    if (argc > 2) {{
        if ((strncmp(argv[2], "init", 4) == 0)
            || (strncmp(argv[2], "admin", 5) == 0)
            || (strncmp(argv[2], "repository", 10) == 0)) {{
            printf("runningX\\n");
            fprintf(stderr, RED "ERROR: " CRESET
            "opam operation not supported by OBazl: %s\\n", argv[2]);
            return EXIT_FAILURE;
        }}
    }}
    fprintf(stdout, YEL "Root module" CRESET
        "  : %s\\n", "{root_module}");
    fprintf(stdout, YEL "  opam bin" CRESET
        "   : %s\\n", getenv("OPAMBIN"));
    fprintf(stdout, YEL "  OPAMROOT" CRESET
        "   : %s\\n", getenv("OPAMROOT"));
    fprintf(stdout, YEL "  OPAMSWITCH" CRESET
        " : %s\\n", getenv("OPAMSWITCH"));
    fprintf(stdout, "\\n");
    int ct = 1; /* for terminating \0 */
    for (int i = 1; i < argc; i++) {{
        ct += strlen(argv[i]) + 1;
    }}
    char *cmd = malloc(ct);
    int offset = 0;
    int written = 0;
    int available_space = 0;
    for (int i = 1; i < argc; i++) {{
        available_space = ct - offset;
        if (available_space <= 0) {{
            break; // Prevent buffer overflow
        }}
        written = snprintf(cmd + offset, available_space,
                           "%s ", argv[i]);
        if (written < 0 || written >= available_space) {{
            cmd[offset] = '\0'; // Ensure null termination
            break; // Stop on encoding error or not enough space
        }}
        offset += written;
    }}
    system(cmd); //argv[1]);
    free(cmd);
}}
        """.format(root_module = rctx.attr.root_module.name)
    )

    rctx.file(
        "BUILD.bzl",
        content = """
def _opam_runner_impl(ctx):
    # args = ctx.actions.args()
    # args.add_all(ctx.attr.args)
    bin = ctx.actions.declare_file("opam")
    ctx.actions.symlink(
        output = bin,
        target_file = ctx.file.bin,
        is_executable = True)
    print("SYMLINK %s" % bin)
    # ctx.actions.run(
    #     executable = ctx.file.bin,
    #     arguments = [args],
    #     inputs  = [],
    #     outputs = [],
    #     tools = [ctx.file.bin],
    #     mnemonic = "opam",
    #     progress_message = "running opam",
    # )
    return DefaultInfo(
        executable=bin
    )

opam_runner = rule(
    implementation = _opam_runner_impl,
    doc = "Runs opam.",
    executable = True,
    attrs = dict(
        bin = attr.label(
            doc = "Bazel label of opam executable.",
            mandatory = False,
            allow_single_file = True,
            executable = True,
            default = ":opam.exe",
            cfg = "exec"
        ),
        # args = attr.string_list()
    )
)
        """.format(rctx.attr.opam_version)
    )

    rctx.file("BUILD.bazel",
        content = """
load("@rules_cc//cc:defs.bzl", "cc_binary")
ARGS = ["--root", "{root}", "--switch", "{switch}"]
ENV = {{"OPAMBIN": "{bin}", "OPAMROOT": "{root}", "OPAMSWITCH": "{switch}"}}

alias(name = "opam", actual=":opam_runner")

cc_binary(
    name = "opam_runner",
    srcs = ["opam_runner.c"],
    copts = ["-Wno-null-character"],
    args = ["{bin}"],
    env  = ENV,
    # data = ["{bin}"]
)
        """.format(root   = rctx.attr.opam_root,
                   switch = rctx.attr.switch_id,
                   bin    = rctx.attr.opam_bin)
              )

    rctx.file("reconfig/reconfig.c",
        content = """
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define YEL "\033[0;33m"
#define RED "\033[0;31m"
#define CRESET "\033[0m"
int main(int argc, char *argv[])
{{
    if (argc > 2) {{
        fprintf(stderr, RED "ERROR: " CRESET
        "no arguments allowed for %s\\n", argv[2]);
            return EXIT_FAILURE;
    }}
    fprintf(stdout, YEL "Root module" CRESET
        "  : %s\\n", "{root_module}");
    fprintf(stdout, YEL "  opam bin" CRESET
        "   : %s\\n", getenv("OPAMBIN"));
    fprintf(stdout, YEL "  OPAMROOT" CRESET
        "   : %s\\n", getenv("OPAMROOT"));
    fprintf(stdout, YEL "  OPAMSWITCH" CRESET
        " : %s\\n", getenv("OPAMSWITCH"));
    fprintf(stdout, "\\n");

    fprintf(stdout, RED "WARNING: " CRESET
        "reconfiguring opam installation to support Bazel operations.\\n");
    fprintf(stdout, "Bazel builds require network access.\\n");

    char *cmd = "opam init --reinit "
        "--no "
        "--bypass-checks "
        "--no-opamrc "
        "--quiet "
        "--config=../tools_opam+/extensions/{config}";
    printf("cwd: %s\\n", getcwd(NULL,0));
    fprintf(stdout, YEL "cmd" CRESET ": %s\\n\\n", cmd);
    system(cmd);
}}
        """.format(
            root_module = rctx.attr.root_module.name,
            config = rctx.attr.config_file.name)
              )

    rctx.file("reconfig/BUILD.bazel",
        content = """
load("@rules_cc//cc:defs.bzl", "cc_binary")
ARGS = ["--root", "{root}", "--switch", "{switch}"]
ENV = {{"OPAMBIN": "{bin}", "OPAMROOT": "{root}", "OPAMSWITCH": "{switch}"}}

cc_binary(
    name = "reconfig",
    srcs = ["reconfig.c"],
    copts = ["-Wno-null-character"],
    args = ["{bin}"],
    env  = ENV,
    data = ["{config_file}"]
)
        """.format(root   = rctx.attr.opam_root,
                   switch = rctx.attr.switch_id,
                   bin    = rctx.attr.opam_bin,
                   config_file = rctx.attr.config_file)
              )

    #### REINIT ####
    rctx.file("reinit/reinit.c",
        content = """
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define YEL "\033[0;33m"
#define RED "\033[0;31m"
#define CRESET "\033[0m"
int main(int argc, char *argv[])
{{
    if (argc > 2) {{
        fprintf(stderr, RED "ERROR: " CRESET
        "no arguments allowed for %s\\n", argv[2]);
            return EXIT_FAILURE;
    }}
    fprintf(stdout, YEL "Root module" CRESET
        "  : %s\\n", "{root_module}");
    fprintf(stdout, YEL "  opam bin" CRESET
        "   : %s\\n", getenv("OPAMBIN"));
    fprintf(stdout, YEL "  OPAMROOT" CRESET
        "   : %s\\n", getenv("OPAMROOT"));
    fprintf(stdout, YEL "  OPAMSWITCH" CRESET
        " : %s\\n", getenv("OPAMSWITCH"));
    fprintf(stdout, "\\n");

    fprintf(stdout, YEL "INFO: " CRESET
        "reinitializing opam installation.\\n");
    fprintf(stdout, "Bazel support (network access) will be removed.\\n");

    char *cmd = "opam init --reinit --quiet ";
    system(cmd);
}}
        """.format(
            root_module = rctx.attr.root_module.name,
            config = rctx.attr.config_file.name)
              )

    rctx.file("reinit/BUILD.bazel",
        content = """
load("@rules_cc//cc:defs.bzl", "cc_binary")
ARGS = ["--root", "{root}", "--switch", "{switch}"]
ENV = {{"OPAMBIN": "{bin}", "OPAMROOT": "{root}", "OPAMSWITCH": "{switch}"}}

cc_binary(
    name = "reinit",
    srcs = ["reinit.c"],
    copts = ["-Wno-null-character"],
    args = ["{bin}"],
    env  = ENV
)
        """.format(root   = rctx.attr.opam_root,
                   switch = rctx.attr.switch_id,
                   bin    = rctx.attr.opam_bin)
              )

    ################

    root = rctx.path("./.opam")
    rctx.symlink(rctx.attr.opam_bin, "opam.exe")

    if rctx.attr.debug > 1:
        print_cwd(rctx)
        print_tree(rctx, root, 1)

###############################
opam_repo = repository_rule(
    implementation = _opam_repo_impl,
    attrs = {
        "root_module": attr.label(),
        "opam_bin": attr.string(),
        "opam_version": attr.string(),
        "opam_root": attr.string(),
        "config_file": attr.label(),
        "switch_id": attr.string(),
        # "ocaml_version": attr.string(),
        # "pkgs": attr.string_list(
        #     mandatory = True,
        #     allow_empty = False
        # ),
        "debug": attr.int(default=0),
        "verbosity": attr.int(default=0)
    },
)
