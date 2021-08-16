load("@obazl_rules_ocaml//ocaml:build.bzl",
     "ocaml_binary",
     "ocaml_deps",
     "ocaml_library",
     "ocaml_ppx_binary",
     "ocaml_ppx_library",
     "ocaml_ppx_module",
     "ocaml_ppx_archive",
     "ocaml_ppx_test")

SRCS = ["check_ocaml_word_size.ml",
        "define_from_scope.ml",
        "define_locally.ml",
        "expires_after.ml",
        "getenv_ppx.ml"]
# Note the capitalization and two _ in the middle:
SUBMOD_SRCS = ["ppx_coda__Check_ocaml_word_size.ml",
               "ppx_coda__Define_from_scope.ml",
               "ppx_coda__Define_locally.ml",
               "ppx_coda__Expires_after.ml",
               "ppx_coda__Getenv_ppx.ml"]

ocaml_deps(name="deps", srcs=SRCS)


### Manual method.  Replaced by ocaml_ns_module

# First build the ppx pipeline.
# (preprocess (pps ppx_version ppxlib.metaquot ppx_optcomp)))
# (preprocessor_deps ../../config.mlh) - ???
##FIXME: use ocaml_ppx_library
ocaml_ppx_binary(
    name = "ppx",
    message = "Compiling preprocessor pipeline...",
    visibility = ["//visibility:public"],
    opts = ["-strict-sequence",
            "-strict-formats",
            "-short-paths",
            "-keep-locs",
            "-g",
            "-predicates", "ppx_deriving",
            "-predicates", "ppx_driver",
            "-linkall",
            "-linkpkg",
            # "-a",
            # "-open", "Ppx_version", "-no-alias-deps", "-opaque",
    ],
    deps = [
        "@opam//pkg:base",
        "@opam//pkg:core_kernel",
        "@opam//pkg:ppxlib",
        "@opam//pkg:ppx_deriving.api",
        # "@opam//pkg:ppx_deriving",
        "//src/lib/ppx_version",
        "@opam//pkg:ppxlib.metaquot",
        "@ppx_optcomp//:ppx_optcomp",
        "@opam//pkg:ppxlib.runner",
    ]
)

# Now deal with the module naming stuff.

# Example: dune "library" with preprocessing
# 1. generate wrapper module foo_bar.ml
# 2. compile foo_bar.ml to foo_bar.cmx
# 2. preprocess srcs
# 3. rename preprocessed srcs, e.g. a_b.ml -> Foo_bar__A_b.ml
# 4. compile renamed, preprocessed srcs to .cmx, e.g.
#    Foo_bar__A_b.cmx
# 5. link the resulting cmx files to foo_bar.cmxa

## Generate wrapper module containing module aliases.
## Expected output form for aliases in output file foo_bar.ml:
## module Fiz_buz = Foo_bar__Fiz_buz""",
genrule(
    name = "ppx_coda_redirector_gen",
    message = "Generating redirector for ppx_coda...",
    srcs = SRCS,
    outs = ["ppx_coda.ml"],
    cmd = "for f in $(SRCS);"
    + " do"
    + "     BNAME=`basename $$f`;"
    + "     BNAME=`expr \"$$BNAME\" : '\(.*\)...'`;"
    + "     HD=`expr \"$$BNAME\" : '\(.\).*'`;"
    + "     HD=`echo $$HD | tr [a-z] [A-Z]`;"
    + "     TL=`expr \"$$BNAME\" : '.\(.*\)'`;"
    + "     echo module $$HD$$TL = Ppx_coda__$$HD$$TL >> \"$@\";"
    + " done"
)

## Compile generated module to *.cmx file.
ocaml_library( #FIXME: ocaml_module
    name = "redirector",
    message = "Compiling redirector module ppx_coda.ml...",
    srcs = [":ppx_coda_redirector_gen"],
    opts = [
        "-w", "-49",  # Warning 49: no cmi file was found in path for module ...
        # "-linkall",
        "-c",
        "-no-alias-deps",  ## REQUIRED for wrappers with module aliases
        "-opaque",
    ],
)

## Preprocess and rename the modules.
genrule(
    name = "preproc_and_rename",
    message = "Preprocessing and renaming modules...",
    tools = [":ppx"],
    srcs = SRCS,
    outs = SUBMOD_SRCS,
    cmd = "for f in $(SRCS);"
    + " do"
    + "    BNAME=`basename $$f`;"
    + "    HD=`expr \"$$BNAME\" : '\(.\).*'`;"
    + "    HD=`echo $$HD | tr [a-z] [A-Z]`;"
    + "    TL=`expr \"$$BNAME\" : '.\(.*\)'`;"
    + "    MODULE=ppx_coda__$$HD$$TL;"
    + "    $(location :ppx) $$f > $(@D)/$$MODULE;"
    + " done"
)

## Now individually compile the submodules.
SUBMODULE_DEPS = [
    "@opam//pkg:core_kernel",
    "@opam//pkg:compiler-libs.common",
    "@opam//pkg:ppxlib",
    "@opam//pkg:ppx_deriving.api",
    "@opam//pkg:ppx_bin_prot",
    ":redirector",
]
SUBMODULE_COPTS = [
    "-linkall", "-linkpkg", "-c", "-predicates", "ppx_deriving",
    ## REQUIRED for submodules with redirector aliasing
    "-open", "Ppx_coda", "-no-alias-deps", "-opaque",
    "-verbose"]
# ocaml_ppx_module( name = "check_ocaml_word_size_cm",
#                   ns   = "ppx_coda",
#                   ppx  = ":ppx",
#                   impl = "check_ocaml_word_size.ml",
#                   copts = SUBMODULE_COPTS,
#                   deps = [
#                       "@opam//pkg:core_kernel",  # Core_kernel
#                       "@opam//pkg:ppxlib",       # Ppxlib, Driver
#                       "@opam//pkg:????",  # Asttypes
#                   ])

ocaml_ppx_module( name = "check_ocaml_word_size",
                  impl = "ppx_coda__Check_ocaml_word_size.ml",
                  opts = SUBMODULE_COPTS, deps = SUBMODULE_DEPS)
ocaml_ppx_module( name = "define_from_scope",
                  impl = "ppx_coda__Define_from_scope.ml",
                  opts = SUBMODULE_COPTS, deps = SUBMODULE_DEPS)
ocaml_ppx_module( name = "define_locally",
                  impl = "ppx_coda__Define_locally.ml",
                  opts = SUBMODULE_COPTS, deps = SUBMODULE_DEPS)
ocaml_ppx_module( name = "expires_after",
                  impl = "ppx_coda__Expires_after.ml",
                  opts = SUBMODULE_COPTS, deps = SUBMODULE_DEPS)
ocaml_ppx_module( name = "getenv_ppx",
                  impl = "ppx_coda__Getenv_ppx.ml",
                  opts = SUBMODULE_COPTS, deps = SUBMODULE_DEPS)

## Finally, link delegator (redirector, wrapper) and delegates (submodules) into an archive.
ocaml_ppx_archive(
    name = "ppx_coda",
    linkall = True,
    opts = [
        "-predicates", "ppx_deriving",
        "-open", "Ppx_version",
    ],
    # redirector = ":redirector",
    # (kind ppx_deriver)
    # kind = "deriver",
    # (preprocessor_deps ../../config.mlh)
    deps = [
        "@opam//pkg:compiler-libs.common",
        "@opam//pkg:ppxlib",
        "@opam//pkg:ppx_deriving.api",
        "@opam//pkg:ppx_bin_prot",
        "@opam//pkg:core_kernel",
        ":redirector",
        ":check_ocaml_word_size",
        ":define_from_scope",
        ":define_locally",
        ":expires_after",
        ":getenv_ppx"
    ],
    visibility = ["//visibility:public"]
)

################################################################
####  TESTS  ####

# all tests use the same ppx:
# (preprocess (pps ppx_jane ppx_deriving_yojson ppx_coda))
#### tests
# all tests use the same ppx pipeline: ppx_jane ppx_deriving_yojson ppx_coda
ocaml_ppx_binary(
    name = "test_ppx",
    message = "Compiling test preprocessor pipeline...",
    visibility = ["//visibility:public"],
    srcs = ["@//ocaml/ppx:ppxlib_driver_standalone_shim"],
    opts = ["-linkall",
            "-linkpkg",
            "-predicates", "ppx_driver",
            "-I", "src/lib/ppx_coda/tests",
            "-verbose"
    ],
    deps = [
        "@opam//pkg:core_kernel",
        "@opam//pkg:ppx_jane",
        "@opam//pkg:ppx_deriving_yojson",
        "@opam//pkg:ppx_module_timer.runtime",
        ":ppx_coda",
    ]
)

## Run test files through test ppx
ocaml_ppx_test(
    name = "ppx_coda_test",
    message = "testing expired.ml",
    ppx = ":test_ppx",
    srcs = [
        "tests/expired.ml",
        "tests/expiry_in_module.ml",
        "tests/expiry_invalid_date.ml",
        "tests/expiry_invalid_format.ml",
        "tests/unexpired.ml",
        "tests/define_from_scope_good.ml",
        "tests/define_locally_good.ml"
    ],
    deps = [":test_ppx"]
)
