load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

OBAZL_BRANCH = "dev"

################################################################
def fetch_repos():

    maybe(
        http_archive,
        name = "bazel_skylib",
        sha256 = "b8a1527901774180afc798aeb28c4634bdccf19c4d98e7bdd1ce79d1fe9aaad7",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.4.1/bazel-skylib-1.4.1.tar.gz",
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.4.1/bazel-skylib-1.4.1.tar.gz",
        ],
    )

    maybe(
        git_repository,
        name = "rules_ocaml",
        remote = "https://github.com/obazl/rules_ocaml",
        branch = OBAZL_BRANCH
    )

    maybe(
        git_repository,
        name = "libs7",
        remote = "https://github.com/obazl/libs7",
        branch = OBAZL_BRANCH
    )


    # maybe(
    #     http_archive,
    #     name = "rules_foreign_cc",
    #     sha256 = "6041f1374ff32ba711564374ad8e007aef77f71561a7ce784123b9b4b88614fc",
    #     strip_prefix = "rules_foreign_cc-0.8.0",
    #     url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.8.0.tar.gz",
    # )

    # maybe(
    #     http_archive,
    #     name = "opam_re2c",
    #     urls = [
    #         "https://github.com/skvadrik/re2c/archive/2.0.3.zip"
    #     ],
    #     strip_prefix = "re2c-2.0.3",
    #     sha256 = "8f74163d02b4ce371d69876af1610177b45055b387656d0fb22c3eab131ccbf9",
    #     workspace_file_content = "workspace( name = \"opam-re2c\" )",
    #     build_file = "@opam//external/re2c:BUILD.bazel"
    #     # build_file_content = "\n".join([
    #     #     "filegroup(name = \"all\",",
    #     #     "srcs = glob([\"**\"]),",
    #     #     "visibility = [\"//visibility:public\"])",
    #     # ]),
    # )

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
