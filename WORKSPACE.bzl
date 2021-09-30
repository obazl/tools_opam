load("@bazel_tools//tools/build_defs/repo:git.bzl",
     "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

all_content = """filegroup(name = "all", srcs = glob(["**"]), visibility = ["//visibility:public"])"""

def opam_fetch_repos():

    maybe(
        git_repository,
        name = "obazl_tools_obazl",
        remote = "https://github.com/obazl/tools_obazl",
        branch = "dev",
    )

    maybe(
        http_archive,
        name = "bazel_skylib",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
        ],
        sha256 = "97e70364e9249702246c0e9444bccdc4b847bed1eb03c5a3ece4f83dfe6abc44",
    )

    maybe(
        git_repository,
        name = "rules_cc",
        remote = "https://github.com/bazelbuild/rules_cc",
        commit = "b1c40e1de81913a3c40e5948f78719c28152486d",
        shallow_since = "1605101351 -0800"
        # branch = "master"
    )

    maybe(
        http_archive,
        name = "rules_foreign_cc",
        strip_prefix = "rules_foreign_cc-0.6.0",
        url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.6.0.tar.gz",
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
        build_file = "@opam//remote/re2c:BUILD.bazel"
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

