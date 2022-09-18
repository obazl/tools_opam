load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

################################################################
def opam_fetch_repos():

    # maybe(
    #     git_repository,
    #     name = "rules_cc",
    #     remote = "https://github.com/bazelbuild/rules_cc",
    #     commit = "b1c40e1de81913a3c40e5948f78719c28152486d",
    #     shallow_since = "1605101351 -0800"
    #     # branch = "master"
    # )

    # maybe(
    #     http_archive,
    #     name = "rules_foreign_cc",
    #     sha256 = "1df78c7d7eed2dc21b8b325a2853c31933a81e7b780f9a59a5d078be9008b13a",
    #     strip_prefix = "rules_foreign_cc-0.7.0",
    #     url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.7.0.tar.gz",
    # )

    maybe(
        http_archive,
        name = "rules_foreign_cc",
        sha256 = "6041f1374ff32ba711564374ad8e007aef77f71561a7ce784123b9b4b88614fc",
        strip_prefix = "rules_foreign_cc-0.8.0",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.8.0.tar.gz",
    )

    maybe(
        git_repository,
        name = "libs7",
        remote = "https://github.com/obazl/libs7",
        branch = "dev"
    )

    maybe(
        http_archive,
        name = "opam_re2c",
        urls = [
            "https://github.com/skvadrik/re2c/archive/2.0.3.zip"
        ],
        strip_prefix = "re2c-2.0.3",
        sha256 = "8f74163d02b4ce371d69876af1610177b45055b387656d0fb22c3eab131ccbf9",
        workspace_file_content = "workspace( name = \"opam-re2c\" )",
        build_file = "@opam//external/re2c:BUILD.bazel"
        # build_file_content = "\n".join([
        #     "filegroup(name = \"all\",",
        #     "srcs = glob([\"**\"]),",
        #     "visibility = [\"//visibility:public\"])",
        # ]),
    )

    ######
    maybe(
        http_archive,
        name = "libinih",
        # build_file_content = "exports_files(['ini.c', 'ini.h'])",
    build_file_content = """
filegroup(name = "srcs", srcs = ["ini.c", "ini.h"], visibility = ["//visibility:public"])
filegroup(name = "hdrs", srcs = ["ini.h"], visibility = ["//visibility:public"])""",
        urls = [
            "https://github.com/benhoyt/inih/archive/cb55f57d87ae840bd0f65dbe6bd22fa021a873a7.tar.gz"
        ],
        strip_prefix = "inih-cb55f57d87ae840bd0f65dbe6bd22fa021a873a7",
        sha256 = "26d05999033eef9e3abca2d4dbf3dc2e4a24335df51231b6faa093be06bb19d7"
    )

    # maybe(
    #     git_repository,
    #     name = "obazl_tools_obazl",
    #     remote = "https://github.com/obazl/tools_obazl",
    #     branch = "dev",
    # )

    # maybe(
    #     new_git_repository,
    #     name = "ck",
    #     remote = "https://github.com/concurrencykit/ck.git",
    #     branch = "master",
    #     # strip_prefix = "re2c-2.0.3",
    #     # sha256 = "269c266c1fc12d1e73682ed1a05296588e8482d188e6d56408a29de447ce87d7",
    #     build_file_content = "\n".join([
    #         "filegroup(name = \"all\",",
    #         "srcs = glob([\"**\"]),",
    #         "visibility = [\"//visibility:public\"])",
    #     ]),
    #     workspace_file_content = "workspace( name = \"ck\" )"
    # )

    # maybe(
    #     git_repository,
    #     name = "io_bazel_stardoc",
    #     remote = "https://github.com/bazelbuild/stardoc.git",
    #     tag = "0.4.0",
    # )

################################################################
# def install_kernel_libs():
#     print("INSTALLING kernel libs repos")
#     # path attr: relative to OPAM_SWITCH_PREFIX

#     new_local_pkg_repository(
#         name = "ocaml.compiler-libs",
#         # path = OPAM_SWITCH_PREFIX + "/lib/ocaml/compiler-libs",
#         path = "ocaml/compiler-libs",
#         build_file = "@opam//opam/_templates:ocaml.compiler-libs.REPO"
#     )

#     new_local_pkg_repository(
#         name = "ocaml.ffi",
#         path = "ocaml/caml",
#         build_file = "@opam//opam/_templates:ocaml.ffi.REPO"
#     )

#     new_local_pkg_repository(
#         name = "ocaml.bigarray",
#         path = "ocaml",
#         build_file = "@opam//opam/_templates:ocaml.bigarray.REPO"
#     )

#     new_local_pkg_repository(
#         name = "ocaml.dynlink",
#         path = "ocaml",
#         build_file = "@opam//opam/_templates:ocaml.dynlink.REPO"
#     )

#     new_local_pkg_repository(
#         name = "ocaml.str",
#         path = "ocaml",
#         build_file = "@opam//opam/_templates:ocaml.str.REPO"
#     )

#     new_local_pkg_repository(
#         name = "ocaml.unix",
#         path = "ocaml",
#         build_file = "@opam//opam/_templates:ocaml.unix.REPO"
#     )

#     new_local_pkg_repository(
#         name = "ocaml.threads",
#         path = "ocaml/threads",
#         build_file = "@opam//opam/_templates:ocaml.threads.REPO"
#     )
