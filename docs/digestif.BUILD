# load("@io_bazel_rules_docker//container:image.bzl", "container_image")

load("@obazl_rules_ocaml//ocaml:build.bzl",
     "ocaml_submodule_rename",  # macro
     "ocaml_redirector_gen",  # macro
     "ocaml_archive",
     "ocaml_binary",
     "ocaml_deps",
     "ocaml_interface",
     "ocaml_library",
     "ocaml_module",
     "ocaml_ppx_binary",
     "ocaml_ppx_module",
     "ocaml_ppx_archive")

################################################################
COMMON_SRCS = [
    "src/digestif_by.ml",
    "src/digestif_bi.ml",
    "src/digestif_conv.ml",
    "src/digestif_eq.ml",
    "src/digestif_hash.ml",
    "src/digestif.mli",
]
COMMON_DEPS = [
    "@opam//pkg:eqaf"
]

# NOTE: for batch compilation, sources must be passed in dependency
# order.  The ordering can be set in the list here, or by passing by a
# dependency file; see the 'depgraph' attribute below.

## WARNING: compiling with depgraph is currently broken.  In a future
# version dependency ordering will be handled automatically.

BAIJIU_SRCS = [
    "src-ocaml/baijiu_sha256.ml",
    "src-ocaml/baijiu_blake2b.ml",
    "src-ocaml/baijiu_blake2s.ml",
    "src-ocaml/baijiu_md5.ml",
    "src-ocaml/baijiu_rmd160.ml",
    "src-ocaml/baijiu_sha1.ml",
    "src-ocaml/baijiu_sha224.ml",
    "src-ocaml/baijiu_sha512.ml",
    "src-ocaml/baijiu_sha384.ml",
    "src-ocaml/baijiu_whirlpool.ml",
    "src-ocaml/xor.ml",
]

OCAML_SRCS = BAIJIU_SRCS + ["src-ocaml/digestif.ml"]

C_SRCS = [
    "src-c/digestif.ml",
    "src-c/digestif_native.ml"
]

SUBMODULE_COPTS = [
    "-linkall", "-linkpkg", "-c", "-predicates", "ppx_deriving",
    ## REQUIRED for submodules with redirector aliasing
    "-open", "Ppx_optcomp", "-no-alias-deps", "-opaque",
    "-verbose"]


ocaml_deps(
    name = "deps",
    srcs =
    COMMON_SRCS
    + OCAML_SRCS
    + C_SRCS
)

ocaml_module(name="digestif_by", impl="src/digestif_by.ml", deps=["@opam//pkg:bigarray"])
ocaml_module(name="digestif_bi", impl="src/digestif_bi.ml", deps=["@opam//pkg:bigarray"])
ocaml_module(name="digestif_conv", impl="src/digestif_conv.ml")
ocaml_module(name="digestif_eq", impl="src/digestif_eq.ml", deps=["@opam//pkg:eqaf"], opts=["-linkpkg", "-linkall"])
ocaml_module(name="digestif_hash", impl="src/digestif_hash.ml")
ocaml_interface(name="digestif_mli", intf="src/digestif.mli")

ocaml_module(name="baijiu_sha256", impl="src-ocaml/baijiu_sha256.ml", deps=[":digestif_bi", ":digestif_by"])
ocaml_module(name="baijiu_blake2b", impl="src-ocaml/baijiu_blake2b.ml", deps=[":digestif_bi", ":digestif_by"])
ocaml_module(name="baijiu_blake2s", impl="src-ocaml/baijiu_blake2s.ml", deps=[":digestif_bi", ":digestif_by"])
ocaml_module(name="baijiu_md5", impl="src-ocaml/baijiu_md5.ml", deps=[":digestif_bi", ":digestif_by"])
ocaml_module(name="baijiu_rmd160", impl="src-ocaml/baijiu_rmd160.ml", deps=[":digestif_bi", ":digestif_by"])
ocaml_module(name="baijiu_sha1", impl="src-ocaml/baijiu_sha1.ml", deps=[":digestif_bi", ":digestif_by"])
ocaml_module(name="baijiu_sha224", impl="src-ocaml/baijiu_sha224.ml",
             deps=[":digestif_bi", ":digestif_by", ":baijiu_sha256"])
ocaml_module(name="baijiu_sha512", impl="src-ocaml/baijiu_sha512.ml", deps=[":digestif_bi", ":digestif_by"])
ocaml_module(name="baijiu_sha384", impl="src-ocaml/baijiu_sha384.ml",
             deps=[":digestif_bi", ":digestif_by", ":baijiu_sha512"])
ocaml_module(name="baijiu_whirlpool", impl="src-ocaml/baijiu_whirlpool.ml", deps=[":digestif_bi", ":digestif_by"])
ocaml_module(name="digestif_ocaml", impl="src-ocaml/digestif.ml", deps=[":baijiu", ":xor"])
ocaml_module(name="xor", impl="src-ocaml/xor.ml", deps=[":digestif_bi", ":digestif_by"])

ocaml_archive(
    name = "common_archive",
    message = "digestif, common",
    visibility = ["//visibility:public"],
    opts = ["-I", "src", "-open", "Digestif_by"],
    deps = [
        ":digestif_by",
        ":digestif_bi",
        ":digestif_conv",
        ":digestif_eq",
        ":digestif_hash",
        ":digestif_mli", # this will be ignored, archives do not understand cmi files
    ]
)

ocaml_library(
    name = "common_lib",
    message = "digestif, common",
    visibility = ["//visibility:public"],
    opts = ["-I", "src", "-open", "Digestif_by"],
    deps = [
        ":digestif_by",
        ":digestif_bi",
        ":digestif_conv",
        ":digestif_eq",
        ":digestif_hash",
        ":digestif_mli",
    ]
)

ocaml_library(
    name = "baijiu",
    message = "digestif baijiu, composite",
    visibility = ["//visibility:public"],
    opts = ["-I", "src", "-open", "Digestif_by"],
    deps = [
        ":common_lib",
        ":baijiu_sha256",
        ":baijiu_blake2b",
        ":baijiu_blake2s",
        ":baijiu_md5",
        ":baijiu_rmd160",
        ":baijiu_sha1",
        ":baijiu_sha224",
        ":baijiu_sha512",
        ":baijiu_sha384",
        ":baijiu_whirlpool",
    ]
)

## Composite build: each source file is built using ocaml_module, and
## the ocaml_library target composes them. This allows Bazel to do the
## scheduling.  Dependencies are listed for each ocaml_module target
## (source file).
ocaml_library(
    name = "ocaml_composite",
    message = "digestif, composite",
    visibility = ["//visibility:public"],
    opts = ["-I", "src", "-open", "Digestif_by"],
    ## Note that the dependencies are build targets, not files - this forces them to build.
    deps = [
        ":common_archive", ## this could also be ":common_lib"
        ":digestif_ocaml",
    ]
)

## Batch build: all sources passed in one batch.  Bazel runs one
## compile job; the compiler runs jobs for each file.
ocaml_library(
    name = "ocaml_batch",
    message = "",
    visibility = ["//visibility:public"],
    opts = ["-verbose",
             "-ccopt", "-v",
             "-w", "@1..3@5..28@30..39@43@46..47@49..57@61..62-40",
             # "-I", "tmp/src",
             "-no-keep-locs",
             "-no-alias-deps",
             "-opaque",
             "-linkall",
             "-linkpkg",
             "-g",
             "-c",
             "-I", "."
    ],
    ## The deps here are source files.
    srcs = COMMON_SRCS + OCAML_SRCS,
    ## A dep graph may be used to order the sources, in which case the
    ## srcs can be in any order. Otherwise, they must be in depencency order.
    # WARNING: depgraph is broken atm. You must list the srcs in dependency order.
    # depgraph = "digestif_ocaml.depends",
    # with batching, we have to provide deps for the group, since the
    # individual source files are not registered with Bazel as build
    # targets with their own deps.
    deps = [
    "@opam//pkg:bigarray",
    "@opam//pkg:eqaf",
    ]
)

################################################################
## C implementation

ocaml_module(name="digestif_c", impl="src-c/digestif.ml", deps=[":common_lib", ":digestif_native"])

ocaml_module(name="digestif_native", impl="src-c/digestif_native.ml",
             opts = ["-c", "-no-alias-deps", "-opaque"],
             deps=[":digestif_bi", ":digestif_by", ":rakia"])

# dune compiles an empty file to produce rakia.cmx. we do not need to do this.
# ocaml_module(name="rakia_cmx", impl="src-c/rakia.ml", opts=["-nopervasives", "-nostdlib"])

# dune uses ocamlmklib to build the c lib, which it calls rakia_stubs - a misnomer, its not a stub file?
# we use the host toolchain, and call the result librakia:
cc_library(
    name = "rakia",
    srcs = [
        "src-c/native/bitfn.h",
        "src-c/native/blake2b.c",
        "src-c/native/blake2b.h",
        "src-c/native/blake2s.c",
        "src-c/native/blake2s.h",
        "src-c/native/digestif.h",
        "src-c/native/md5.c",
        "src-c/native/md5.h",
        "src-c/native/misc.c",
        "src-c/native/ripemd160.c",
        "src-c/native/ripemd160.h",
        "src-c/native/sha1.c",
        "src-c/native/sha1.h",
        "src-c/native/sha256.c",
        "src-c/native/sha256.h",
        "src-c/native/sha512.c",
        "src-c/native/sha512.h",
        "src-c/native/stubs.c",
        "src-c/native/whirlpool.c",
        "src-c/native/whirlpool.h",
    ],
    copts = ["-I", "external/ocaml/csdk"],
    linkstatic = True,
    deps = ["@ocaml//:csdk"]
)

# For demo purposes, build some variants of rakia.
ocaml_archive(
    name = "rakia_archive",
    archive_name = "librakia",
    message = "digestif, rakia_lib",
    visibility = ["//visibility:public"],
    opts = [# "-linkall",
            "-verbose"
    ],
    features = ["-keep-locs"],
    deps = [
        # ":rakia_cmx",
        ":rakia"
    ]
)

ocaml_archive(
    name = "rakia_so",
    archive_name = "librakia",
    message = "digestif, rakia_lib",
    visibility = ["//visibility:public"],
    opts = ["-linkall",
            "-verbose"
    ],
    linkshared = True,
    deps = [
        ":rakia_archive"
    ]
)

## Now integrate librakia and ocaml. Two methods, batched and composite.
ocaml_library(
    name = "c_batch",
    message = "digestif, c batch",
    visibility = ["//visibility:public"],
    srcs = COMMON_SRCS + C_SRCS,
    # WARNING: depgraph is broken atm. You must list the srcs in dependency order.
    # depgraph = "digestif_c.depends",
    opts = [
        "-I", ".",
        "-linkall",
        "-linkpkg",
        "-no-alias-deps",
        "-opaque",
        "-verbose",
        "-c"
    ],
    deps = [
        "@opam//pkg:bigarray",
        "@opam//pkg:eqaf",
        ":rakia",
    ]
)

ocaml_library(
    name = "c_composite",
    message = "digestif, c composite",
    visibility = ["//visibility:public"],
    opts = ["-I", "src", "-open", "Digestif_by"],
    deps = [
        ":common_lib",
        ":digestif_c",
        ":digestif_native",
        ":rakia",
    ]
)

## digestif_c: uses librakia implementation?
# ocamlopt.opt
# -w @1..3@5..28@30..39@43@46..47@49..57@61..62-40
# -strict-sequence
# -strict-formats
# -short-paths
# -keep-locs
# -no-keep-locs
# -g
# -a
# -o src-c/digestif_c.cmxa
# src-c/.digestif_c.objs/native/digestif_bi.cmx
# src-c/.digestif_c.objs/native/digestif_by.cmx
# src-c/.digestif_c.objs/native/digestif_native.cmx
# src-c/.digestif_c.objs/native/digestif_hash.cmx
# src-c/.digestif_c.objs/native/digestif_eq.cmx
# src-c/.digestif_c.objs/native/digestif_conv.cmx
# src-c/.digestif_c.objs/native/digestif.cmx


## digestif_ocaml: uses baijiu ocaml implementation, even though it builds in c/native?
# ocamlopt.opt
# -w @1..3@5..28@30..39@43@46..47@49..57@61..62-40
# -strict-sequence
# -strict-formats
# -short-paths
# -keep-locs
# -no-keep-locs
# -g
# -a
# -o
# src-ocaml/digestif_ocaml.cmxa
# src-ocaml/.digestif_ocaml.objs/native/digestif_bi.cmx
# src-ocaml/.digestif_ocaml.objs/native/digestif_by.cmx
# src-ocaml/.digestif_ocaml.objs/native/xor.cmx
# src-ocaml/.digestif_ocaml.objs/native/digestif_hash.cmx
# src-ocaml/.digestif_ocaml.objs/native/digestif_eq.cmx
# src-ocaml/.digestif_ocaml.objs/native/digestif_conv.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_blake2b.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_blake2s.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_md5.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_rmd160.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_sha1.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_sha256.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_sha224.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_sha512.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_sha384.cmx
# src-ocaml/.digestif_ocaml.objs/native/baijiu_whirlpool.cmx
# src-ocaml/.digestif_ocaml.objs/native/digestif.cmx

################################################################
####  TESTS  ####
## to run:  bazel run instead of bazel build

ocaml_binary(
    name = "ocaml_test",
    srcs = ["test/test.ml"],
    opts = ["-linkall"],
    data = [
        "test/blake2b.test",
        "test/blake2s.test"
    ],
    strip_data_prefixes = True,  # make the data files available at runtime in same dir as binary
    deps = [
        "@opam//pkg:fmt",
        "@opam//pkg:alcotest",
        "//:ocaml_composite"
        # "//:ocaml_batch"   ## not working atm, sorry
    ]
)

ocaml_binary(
    name = "c_test",
    srcs = ["test/test.ml"],
    opts = ["-linkall",
            "-verbose"],
    data = [
        "test/blake2b.test",
        "test/blake2s.test"
    ],
    strip_data_prefixes = True,  # make the data files available at runtime in same dir as binary
    deps = [
        "@opam//pkg:fmt",
        "@opam//pkg:alcotest",
        "//:c_composite",
        # "//:c_batch",  # not working atm, sorry
    ]
)

# WARNING: this test fails under dune, with:
# conv_test: internal error, uncaught exception:
#            Alcotest__Core.Registration_error("Duplicate test name: of_hex")
ocaml_binary(
    name = "conv_test",
    srcs = ["test/conv/test_conv.ml"],
    opts = ["-linkall", "-I", "test",
            "-verbose"],
    deps = [
        "@opam//pkg:fmt",
        "@opam//pkg:alcotest",
        "//:c_composite",
    ]
)

## experimental - this tool will be used to automatically put batched
## sources in dependency order.
ocaml_binary(
    name = "sort_deps",
    srcs = ["@obazl_rules_ocaml//tools:sort_deps"],
    opts = ["-thread"],
    data = ["deps.depends"],
    deps = ["@opam//pkg:core",
            "@opam//pkg:core_kernel"]
)

################################################################
## docker

# container_image(
#     name = "helloworld",
# )
