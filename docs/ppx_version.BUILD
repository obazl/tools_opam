load("@obazl_rules_ocaml//ocaml:build.bzl",
     "ocaml_ppx_binary",
     "ocaml_ppx_library",
     "ocaml_ppx_archive",
     "ocaml_ppx_module",
     "ocaml_ppx_pipeline",
     "ocaml_ppx_test")

SRCS = [
    "bin_io_unversioned.ml",
    "lint_version_syntax.ml",
    # "print_binable_functors.ml",
    # "print_versioned_types.ml",
    "versioned_util.ml",
    "versioned_module.ml",
    "versioned_type.ml",
]

## Generate the module redirector containing module aliases.
genrule(
    name = "redirector_gen",
    message = "genrule: ppx_version.ml",
    srcs = glob(["*.ml"],
                exclude=["print_binable_functors.ml",
                         "print_versioned_types.ml"]),
    outs = ["ppx_version.ml"],
    ## NOTE: genrule runs bash, but we use portable sh code anyway.
    cmd = "for f in $(SRCS);"
    + " do"
    + "     BNAME=`basename $$f`;"
    + "     BNAME=`expr \"$$BNAME\" : '\(.*\)...'`;"
    + "     HD=`expr \"$$BNAME\" : '\(.\).*'`;"
    + "     HD=`echo $$HD | tr [a-z] [A-Z]`;"
    + "     TL=`expr \"$$BNAME\" : '.\(.*\)'`;"
    + "     echo module $$HD$$TL = Ppx_version__$$HD$$TL >> \"$@\";"
    + " done"
)

## Compile the redirector module.
ocaml_ppx_module(
    name = "redirector",
    message = "Compiling redirector module ppx_version...",
    impl = ":redirector_gen",
    copts = [
        "-verbose",
        # Error (warning 49): no cmi file was found in path for module <m>
        # Disable for redirector generation:
        "-w", "-49",
        "-linkall",
        # "-linkpkg",
        "-c",
        # "-predicates", "ppx_deriving",
        "-no-alias-deps",  ## REQUIRED for redirectors with module aliases
        "-opaque",
        # "-nopervasives",
        # "-nostdlib"
        # "-a"
        # "-I", "src/lib/ppx_version"
    ],
)

## Next, preprocess and rename the submodules.
## Use //ppx:metaquot to preproces
## Rename result: foo_bar.ml => Ppx_version__Foo_bar.ml
genrule(
    name = "preproc_and_rename",
    message = "genrule: preprocess and rename submodules...",
    tools = ["//ocaml/ppx:metaquot"],
    srcs = SRCS,
    ## WARNING: order matters here. Dune can evidently do this automatically,
    ## but here it's on the user.
    outs = [
        "ppx_version__Lint_version_syntax.ml",
        "ppx_version__Bin_io_unversioned.ml",
        "ppx_version__Versioned_module.ml",
        "ppx_version__Versioned_type.ml",
        "ppx_version__Versioned_util.ml",
    ],
    cmd = "for f in $(SRCS);"
    + " do"
    ## transform the file name
    + "    BNAME=`basename $$f`;"
    + "    HD=`expr \"$$BNAME\" : '\(.\).*'`;"
    + "    HD=`echo $$HD | tr [a-z] [A-Z]`;"
    + "    TL=`expr \"$$BNAME\" : '.\(.*\)'`;"
    + "    MODULE=ppx_version__$$HD$$TL;"
    ## preprocess and write to new name
    + "    $(location //ocaml/ppx:metaquot) $$f > $(@D)/$$MODULE;"
    + " done"
)

## Compile each submodule separately.
#(libraries compiler-libs.common ppxlib ppx_deriving.api ppx_bin_prot core_kernel)
SUBMODULE_DEPS = [
    "@opam//pkg:compiler-libs.common",
    "@opam//pkg:ppxlib",
    "@opam//pkg:ppx_deriving.api",
    "@opam//pkg:ppx_bin_prot",
    "@opam//pkg:core_kernel",
    ":redirector",
]
SUBMODULE_OPTS = [
    "-linkall", "-linkpkg", "-c", "-predicates", "ppx_deriving",
    ## REQUIRED for submodules with redirector aliasing
    "-open", "Ppx_version", "-no-alias-deps", "-opaque",
    "-verbose"]
## NOTE: to enforce ordered compilation, we need to add explicit
## submodule deps here (e.g. versioned_module depends on :versioned_util)
ocaml_ppx_module( name = "lint_version_syntax",
                  ns   = "ppx_version",
                  impl = "lint_version_syntax.ml",
                  ppx  = "//ocaml/ppx:metaquot",
                  opts = SUBMODULE_OPTS,
                  deps = SUBMODULE_DEPS)


# ocaml_ppx_module( name = "lint_version_syntax",
#                   impl = "ppx_version__Lint_version_syntax.ml",
#                   opts = SUBMODULE_OPTS,
#                   deps = SUBMODULE_DEPS)
ocaml_ppx_module( name = "versioned_util",
                  impl = "ppx_version__Versioned_util.ml",
                  opts = SUBMODULE_OPTS,
                  deps = SUBMODULE_DEPS)
ocaml_ppx_module( name = "versioned_module",
                  impl = "ppx_version__Versioned_module.ml",
                  opts = SUBMODULE_OPTS,
                  deps =  SUBMODULE_DEPS + [":versioned_util"])
ocaml_ppx_module( name = "versioned_type",
                  impl = "ppx_version__Versioned_type.ml",
                  opts = SUBMODULE_OPTS,
                  deps = SUBMODULE_DEPS + [":versioned_util"])
ocaml_ppx_module( name = "bin_io_unversioned",
                  impl = "ppx_version__Bin_io_unversioned.ml",
                  opts = SUBMODULE_OPTS,
                  deps = SUBMODULE_DEPS + [":versioned_module"])

PPX_ARCHIVE_OPTS = [
    "-strict-sequence",
    "-strict-formats",
    "-short-paths",
    "-keep-locs",
    "-g",
    "-linkall",
    "-no-alias-deps",
    "-opaque",
]

## Finally, link into archive.
ocaml_ppx_archive(
    name = "ppx_version",
    message = "Compiling ppx_version...",
    opts = PPX_ARCHIVE_OPTS
    + ["-open", "Ppx_version",
       "-predicates", "ppx_deriving"],
    # redirector = "Ppx_version",
    # kind = "deriver",
    deps = [
        "@opam//pkg:compiler-libs.common",
        "@opam//pkg:ppxlib",
        "@opam//pkg:ppx_deriving.api",
        "@opam//pkg:ppx_bin_prot",
        "@opam//pkg:core_kernel",
        ":redirector",
        ":versioned_util",
        ":lint_version_syntax",
        ":bin_io_unversioned",
        ":versioned_type",
        ":versioned_module",
    ],
    visibility = ["//visibility:public"],
)

################################################################
####  EXECUTABLES  ####

## These don't build under dune, so we ignore for now.

################################################################
#### TESTS ####

## WARNING: the test ppx depends on ppx_coda, which depends on
## ppx_version, which puts us in a vicious circle: we cannot test
## ppx_version until we've tested ppx_coda, which depends on
## ppx_version...

# dune tests all use the same ppx:
# (preprocess (pps ppx_jane ppx_deriving_yojson ppx_coda ppx_version))
ocaml_ppx_pipeline(
    name = "test_ppx",
    srcs = ["//ocaml/ppxlib:driver_standalone_shim"],
    opts = ["-strict-sequence",
            "-strict-formats",
            "-short-paths",
            "-keep-locs",
            "-g",
            "-predicates", "ppx_deriving",
            "-predicates", "ppx_driver",
            "-linkpkg",
            "-verbose",
    ],
    deps = [
        "@opam//pkg:core_kernel",
        "@opam//pkg:ppxlib",
        "@opam//pkg:ppx_inline_test",
        "@opam//pkg:ppx_jane",
        "@opam//pkg:ppx_deriving_yojson",
        "//src/lib/ppx_coda",
        ":ppx_version",
    ]
)

ocaml_ppx_test(
    name = "ppx_version_test",
    message = "running ppx_version tests",
    ppx = ":test_ppx",
    srcs = [
        "tests/bad_version_syntax_bin_io_in_functor.ml",
        "tests/bad_version_syntax_extension.ml",
        "tests/bad_version_syntax_missing_versioned.ml",
        "tests/bad_version_syntax_multiple_errors.ml",
        "tests/bad_version_syntax_version_in_functor.ml",
        "tests/bad_versioned_in_functor.ml",
        "tests/bad_versioned_in_nested_functor.ml",
        "tests/versioned_bad_arrow_type.ml",
        "tests/versioned_bad_contained_types.ml",
        "tests/versioned_bad_module_name.ml",
        "tests/versioned_bad_module_structure.ml",
        "tests/versioned_bad_option.ml",
        "tests/versioned_bad_type_name.ml",
        "tests/versioned_bad_unnumbered.ml",
        "tests/versioned_bad_version_name.ml",
        "tests/versioned_bad_wrapped_module_structure.ml",
        "tests/versioned_module_bad_missing_to_latest.ml",
        "tests/versioned_module_bad_missing_type.ml",
        "tests/versioned_module_bad_stable_name.ml",
        "tests/versioned_module_bad_version_name.ml",
        "tests/versioned_module_bad_version_order.ml",

        "tests/good_version_syntax.ml",
        "tests/versioned_good.ml",
        "tests/versioned_module_good.ml",
        "tests/versioned_sig_good.ml",
    ],
    deps = [":test_ppx"]
)

ocaml_ppx_test(
    name = "ppx_version_test1",
    message = "running ppx_version tests",
    ppx = ":test_ppx",
    srcs = [
        "tests/bad_version_syntax_version_in_functor.ml",
    ],
    deps = [":test_ppx"]
)
