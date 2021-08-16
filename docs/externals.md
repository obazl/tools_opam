# external repositories


## transitive deps

Example from ate-pairing:

WORKSPACE:

all_content = """filegroup(name = "all", srcs = glob(["**"]), visibility = ["//visibility:public"])"""

http_archive( name="libgmp",
    url="https://gmplib.org/download/gmp/gmp-6.2.0.tar.xz",
    sha256="258e6cd51b3fbdfc185c716d55f82c08aff57df0c6fbd143cf6ed561267a1526",
    strip_prefix = "gmp-6.2.0", build_file_content = all_content )
    IMPORTANT: always use all_content filegroup. Then Bazel will
    generate a BUILD file in the external repo containing only this
    rule, to which clients can refer in their (configure_make) build rules.

We never want to use the (generated) openssl/BUILD file to build,
    since it is in a different repo. we want the build file in this
    repo build_file = "openssl/BUILD.bazel"


in external/libgmp/BUILD.bazel:

# this is what allows downstream repos that depend on this repo and
# use the preceding :libgmp rule to build the lib.  the rule is: the
# build file for libgmp must contain a copy of this rule. for
# downstream deppers, that's all it contains - they set it using
# build_file_content, and Bazel constructs the BUILD file within the
# libgmp repo.  we only provide the build rule in the "base" case,
# here.

filegroup(
    name = "all",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

configure_make(
    name = "libgmp",
    configure_env_vars = select({
        "//bzl/host:macos": {"AR": ""},
        # "//bzl/host:linux_x64": { "LD": "-Wl,-S -Wl,-z,relro,-z,now" },
        "//conditions:default": {}
    }),
    #     # cross-compile:
    #     # "CPPFLAGS": "-P -fPIC -I$COSYSROOT/usr/local/include -I$COSYSROOT/usr/include",
    #     # "LDFLAGS": "-L$COSYSROOT/usr/lib/x86_64-unknown-linux-gnu",
    # ],
    ## workaround: omission of -Dredacted results in:
    ## error: invalid suffix on literal; C++11 requires a space
    ## between literal and identifier [-Wreserved-user-defined-literal]
    configure_options = select({
        "//bzl/host:linux_x64": [
            # "--host=x86_64-unknown-linux-gnu",
            # "--prefix=" + COSYSROOT + "/usr/local",
            # "--with-sysroot=" + COSYSROOT + "/x86_64-unknown-linux-gnu/sysroot",
            "--enable-shared",
            "--enable-static",
            "--with-pic",
        ],
        "//bzl/host:macos": [
            # "--build=x86_64-apple-darwin",
            # "AR=\"\"",
            # # "-Wl,-no_pie" # avoid PIE disabled warning
        ],
        "//conditions:default": [],
    }) + [
        "--enable-cxx",
        "CXXFLAGS=-lstdc++",
    ],


    # IMPORTANT: do not use ":all" for lib_source!
    # Reference to lib sources must go through the repo, because Bazel
    # will generate its BUILD file, within the (external) repo dir.
    # In this case, it will be a copy of this file; in the case of
    # downstream clients of this lib, it will be a build_file_content
    # string.  Using ":all" here relativizes refs to this dir rather
    # than the repo dir, so e.g. the config_command above will fail.
    
    # Once this file is copied to external/openssl/BUILD, the
    # filegroup above will be relative to the contents of
    # external/openssl.  But this file will still be available in the
    # source tree, so  the lib can be built with "//external/openssl".

    # In other words, this file is a kind of hybrid, used locally but
    # also used (by copy) in the external repo.

    lib_source = "@libgmp//:all",
    out_lib_dir = "lib",
    shared_libraries = select({
        "//bzl/host:macos": ["libgmp.dylib"],
        "//bzl/host:linux_x64": ["libgmp.so", "libgmpxx.so"],
        "//conditions:default": ["libgmp.so", "libgmpxx.so"],
    }),
    static_libraries = [
        "libgmp.a",
        "libgmpxx.a",
    ],
    visibility = ["//visibility:public"],
)

Example: libff

WORKSPACE:

# ate-pairing already includes libgmp, but Bazel does not yet support
# transitive external deps, so we need to repeat the dep here.
# Specifically, ate-pairing build rules refer to @libgmp, which bazel
# resolves to the root repo, which is determined by this WORKSPACE
# file, rather than the one in the ate-pairing repo. So we need to
# copy ate-pairing's repo rule into this file.  The build logic will
# come from ate-pairing; we just need to make the repo available here,
# in the root repo.  Build rules for this repo can then refer to
# @ate-pairing//external/libgmp (a target that indirectly refers to
# the repo defined here).
