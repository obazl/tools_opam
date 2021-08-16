load("@obazl_rules_ocaml//ocaml:build.bzl",
     "ocaml_submodule_rename",  # macro
     "ocaml_redirector_gen",  # macro
     "ocaml_binary",
     "ocaml_library",
     "ocaml_ppx_binary",
     "ocaml_ppx_library",
     "ocaml_ppx_module",
     "ocaml_ppx_archive")

SRCS = [
    "src/token.ml",
    "src/cparser.ml",
    "src/interpreter.ml",
    "src/ppx_optcomp.ml",
]
SUBMODULE_SRCS = [
    "ppx_optcomp__Token.ml",
    "ppx_optcomp__Cparser.ml",
    "ppx_optcomp__Interpreter.ml",
    "ppx_optcomp__Ppx_optcomp.ml",
]

SUBMODULE_DEPS = [
    "@opam//pkg:compiler-libs.common",
    "@opam//pkg:base",
    "@opam//pkg:ppxlib",
    "@opam//pkg:stdio",
]
SUBMODULE_COPTS = [
    "-linkall", "-linkpkg", "-c", "-predicates", "ppx_deriving",
    ## REQUIRED for submodules with redirector aliasing
    "-open", "Ppx_optcomp", "-no-alias-deps", "-opaque",
    "-verbose"]

## Generate the module redirector containing module aliases.
ocaml_redirector_gen(name="redirector_gen",
                     redirector="ppx_optcomp",
                     srcs = SRCS)

## Compile the redirector module.
ocaml_ppx_module(
    name = "redirector",
    msg  = "Compiling redirector module...",
    impl = ":redirector_gen",
    opts = [
        # dune:
        ## -w @1..3@5..28@30..39@43@46..47@49..57@61..62-40 -strict-sequence -strict-formats -short-paths -keep-locs -w -49 -nopervasives -nostdlib -g -bin-annot -I src/external/ppx_optcomp/src/.ppx_optcomp.objs/byte -no-alias-deps -opaque
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

## Rename the submodules. No preprocessing.
ocaml_submodule_rename(name="rename_submodules", prefix="ppx_optcomp", srcs=SRCS)

# Archive redirector and submods to ppx module.
# NB: ocaml_ppx_archive, not ocaml_archive
ocaml_ppx_archive(
    name = "ppx_optcomp",
    message = "Compiling ppx_optcomp...",
    # redirector = [":redirector"],
    # kind  = "deriver",
    linkall = True,
    opts = [
        # "-linkall",
        "-predicates", "ppx_deriver",
        "-open", "Ppx_optcomp__",
        # "-a",  # always on for ocaml_ppx_archive
        "-verbose"
    ],
    srcs = [":rename_submodules"], # SUBMODULE_SRCS,
    deps = SUBMODULE_DEPS + [":redirector"],
    visibility = ["//visibility:public"]
)

################################################################
####  TEST

## dune generates redirector modules for src/*ml and test/*ml

# [2], [4]: dune evaluates all tuareg dunefiles first. The
# coda_digestif and config targets are not relevant to this pkg, but
# the test subdir contains a tuareg dunefile.

# [2] build src/coda_digestif - ignore
# [3] run ocaml on test/dune.ml - generates test/dune
# [4] build src/config - ignore

## now build the generated test/dune:
    # (copy_files import_relativity/**.mlh)

    # (rule
    #  (targets ghost)
    #  (mode fallback)
    #  (deps ...files in test/import_relativity...)
    #  (action (bash "touch ghost")))

    # (library (name ppx_optcomp_test)
    #  (inline_tests)
    #  (preprocessor_deps ghost)
    #  (preprocess (pps ppx_optcomp ppx_inline_test ppx_expect)))"

# dune implicit: generate test redirector (ppx_optcomp_test.ml-gen)
## ppx_optcomp_test.ml-gen =
## (** @canonical Ppx_optcomp_test.Import_relativity *)
## module Import_relativity = Ppx_optcomp_test__Import_relativity
## (** @canonical Ppx_optcomp_test.Injection *)
## module Injection = Ppx_optcomp_test__Injection
## Generate the module redirector containing module aliases.

TEST_SRCS = depset(
    direct = [
        "test/injection.ml",
        "test/import_relativity.ml",
    ]
)

# TEST_SUBMODS = depset(
#     direct = ["ppx_optcomp_test__Import_relativity.ml",
#               "ppx_optcomp_test__Injection.ml"]
# )

ocaml_redirector_gen(name="test_redirector_gen",
                     redirector="test/ppx_optcomp_test",
                     srcs = TEST_SRCS)

# [5] bytecode compile test/ppx_optcomp_test.ml-gen -> ppx_optcomp_test.cmo
## we're not doing bytecode output yet - see step [11] below

# [6] ocamldep src/ppx_optcomp.ml - ignore

## dune then compiles the lib srcs - we've already done that
# [7] -c compile src/ppx_optcomp__.ml-gen -> ppx_optcomp__.cmo
## ppx_optcomp_.ml-gen:
## (** @canonical Ppx_optcomp.Cparser *)
## module Cparser = Ppx_optcomp__Cparser
## (** @canonical Ppx_optcomp.Interpreter *)
## module Interpreter = Ppx_optcomp__Interpreter
## (** @canonical Ppx_optcomp.Token *)
## module Token = Ppx_optcomp__Token

# [8] - [10] run ocamldep on src/*.ml

# [11] native compile test/ppx_optcomp_test.ml-gen -o ppx_optcomp_test.cmx
ocaml_ppx_module(
    name = "test_redirector",
    msg  = "Compiling redirector module Ppx_optcomp_test...",
    impl = ":test_redirector_gen",
    opts = [
        "-w", "@1..3@5..28@30..39@43@46..47@49..57@61..62-40",
        "-w", "-49",
        "-strict-sequence", "-strict-formats",
        "-short-paths",
        "-keep-locs",
        "-nopervasives",
        "-nostdlib",
        "-g",
        # "-bin-annot",  # bytecode only
        "-no-alias-deps",
        "-opaque",
        "-verbose",
        # "-linkall",
        # "-linkpkg",
        "-c",
    ],
)

# [12] in test/ $ bash -e -u -o pipefail -c 'touch ghost'
# genrule(
#     name = "ghost",
#     message = "touching ghost file",
#     outs = ["test/ghost"],
#     cmd = "touch $@"
# )

# back to lib srcs; already done, ignore
# [13] in src/: compile ppx_optcomp__.ml-gen

# in test/.ppx_optcomp_test.inline-tests
# [14] use ocamldep to generate run.ml-gen.d (run.ml-gen compiled in [34])

# [15] = [23] compiling lib submodules - we've already done, ignore
## opts:
# -w @1..3@5..28@30..39@43@46..47@49..57@61..62-40 -strict-sequence -strict-formats -short-paths -keep-locs -g -bin-annot -I ...  -no-alias-deps -opaque -open Ppx_optcomp__
# first bytecode compile:
# [15] -c compile src/interpreter.ml -> ppx_optcomp__Interpreter.cmo
# [16] -c compile src/token.ml -> ppx_optcomp__Token.cmo
# [17] -c compile src/cparser.ml -> ppx_optcomp__Cparser.cmo
# now native compile, opts:
# -w @1..3@5..28@30..39@43@46..47@49..57@61..62-40 -strict-sequence -strict-formats -short-paths -keep-locs -g -I ...  -intf-suffix .ml -no-alias-deps -opaque -open Ppx_optcomp__ 

# [18] -c compile src/token.ml -> ppx_optcomp__Token.cmx
# [19] -c compile src/interpreter.ml -> ppx_optcomp__Interpreter.cmx

# back to bytecode for main module:
# [20] -c compile src/ppx_optcomp.ml -> ppx_optcomp__ppx_optcomp.cmo

# back to native:
# [21] -c compile src/cparser.ml -> ppx_optcomp__Cparser.cmx
# [22] -c compile src/ppx_optcomp.ml -> ppx_optcomp__ppx_optcomp.cmx

# [23] now archive:
# -w @1..3@5..28@30..39@43@46..47@49..57@61..62-40 -strict-sequence -strict-formats -short-paths -keep-locs -g -a -o ppx_optcomp.cmxa -linkall ppx_optcomp__.cmx ppx_optcomp__Token.cmx ppx_optcomp__Cparser.cmx ppx_optcomp__Interpreter.cmx ppx_optcomp.cmx

# [24] build ppx with shim, linking in src lib, to use on test files:
# (preprocess (pps ppx_optcomp ppx_inline_test ppx_expect))
# -w -24 _ppx.ml = let () = Ppxlib.Driver.standalone ()
# last linked lib is ppx_optcomp.cmxa, built in step [23].
ocaml_ppx_binary(
    name = "test_ppx",
    # srcs = ["@//ocaml/ppxlib:driver_standalone_shim"],
    # srcs = ["@opam//ppxlib:driver_standalone_runner"], # let () = Ppxlib.Driver.standalone ()
    opts = ["-w", "-24",
            "-linkpkg",
            # "-linkall",
             "-predicates", "ppx_driver",
            # "-predicates", "ppx_deriving",
            # "-ppx",
            # "-ppx",
            "-open", "Ppx_driver_runner",
             "-verbose",
    ],
    deps = [
        "@opam//pkg:ppxlib",
        "@opam//pkg:ppx_inline_test",
        # "@opam//pkg:ppx_inline_test.runner",
        "@opam//pkg:ppx_expect",
        ":ppx_optcomp",
        # "@opam//pkg:ppxlib.runner",
        # "@opam//pkg:ppx_expect.evaluator",
        "@opam//ppxlib:driver_standalone_runner",
    ]
)

## now prepare test code.
## use ppx to preprocess test srcs
# [25] preprocess: test/injection.ml -> test/injection.pp.ml
# ppx.exe --cookie 'library-name="ppx_optcomp_test"' -o test/injection.pp.ml --impl test/injection.ml -corrected-suffix .ppx-corrected -diff-cmd - -dump-ast
# [26] ditto for test/import_relativity.ml -> test/import_relativity.pp.ml

## FIXME: make this depend on the include files as well
## dune hack uses ghost to do this
## IMPORTANT: --cookie 'library-name="foo"' is passed to ppx_expect?
TEST_SRCS_PPed = depset(
    direct = [
        "_tmp_/ppx_optcomp_test__Injection.ml",
        "_tmp_/ppx_optcomp_test__Import_relativity.ml",
    ]
)

genrule(
    name = "test_preproc",
    message = "Preprocessing test sources...",
    # hack: putting *.mlh in tools establishes dependency;
    # putting them in srcs complicates the cmd
    tools = glob(["test/import_relativity/**/*.mlh"])
    + [":test_ppx"],
    srcs = ["test/import_relativity.ml", "test/injection.ml"],
    outs = TEST_SRCS_PPed,
    cmd = "for f in $(SRCS);"
    + "do"
    + "    echo $$f;"
    + "    BNAME=`basename $$f`;"
    + "    HD=`expr \"$$BNAME\" : '\(.\).*'`;"
    + "    HD=`echo $$HD | tr [a-z] [A-Z]`;"
    + "    TL=`expr \"$$BNAME\" : '.\(.*\)'`;"
    + "    $(location :test_ppx)"
    + "    --cookie 'library-name=\"ppx_optcomp_test\"'"
    + "    -o $(@D)/_tmp_/ppx_optcomp_test__$$HD$$TL"
    + "    --impl $$f"
    + "    -corrected-suffix .ppx-corrected"
    + "    -diff-cmd - "
    + "    --dump-ast;"
    + " done"
)

# [27], [28] ignore: run ocamldep against import_relativity.pp.ml, injection.pp.ml

# -c compile test files. Options:

# first bytecode
# [29] -c compile test/import_relativity.pp.ml -> ppx_optcomp_test__Import_relativity.cmo
# [30] -c compile test/injection.pp.ml -> ppx_optcomp_test__Injection.cmo

# then native, opts: same, without --bin-annot, with  -intf-suffix .mln
# [31] -c compile test/import_relativity.pp.ml -> ppx_optcomp_test__Import_relativity.cmx
# [32] -c compile test/injection.pp.ml -> ppx_optcomp_test__Injection.cmx

## for now, we compile preprocessed test files together, linking with
## ppx_optcomp etc.:

TEST_SUBMODULE_DEPS = [
    "@opam//pkg:ppx_inline_test",
    "@opam//pkg:ppx_expect",
]
TEST_SUBMODULE_COPTS = [
    # "-bin-annot",
    "-no-alias-deps",
    "-opaque",
    # "-predicates", "ppx_driver",
    # "-open", "Ppx_optcomp_test__",
    "-c",
    # "-linkall",
    # "-linkpkg",
    "-verbose"
]

ocaml_ppx_library(
    name = "ppx_optcomp_test_cmx",
    visibility = ["//visibility:public"],
    message = "Compiling...",
    srcs = [":test_preproc"],
    opts = TEST_SUBMODULE_COPTS,
    deps = TEST_SUBMODULE_DEPS + [":test_redirector"]
)

# now archive test files to test/ppx_optcomp_test.cmxa:
# [33] -w @1..3@5..28@30..39@43@46..47@49..57@61..62-40 -strict-sequence -strict-formats -short-paths -keep-locs -g -a -o test/ppx_optcomp_test.cmxa ppx_optcomp_test.cmx ppx_optcomp_test__Injection.cmx test/ppx_optcomp_test__Import_relativity.cmx
# ocaml_library(
ocaml_ppx_archive(
    name = "ppx_optcomp_test_cmxa",
    archive_name = "ppx_optcomp_test",
    visibility = ["//visibility:public"],
    message = "Compiling...",
    # srcs = [":test_preproc"],
    opts = [
        # "-linkall",
        "-a",
        "-verbose"
    ],
    deps = [
        "@opam//pkg:ppx_inline_test",
        "@opam//pkg:ppx_expect",
        ":test_redirector",
        ":ppx_optcomp_test_cmx"]
)

# test shim, run.ml-gen:
# let () = Ppx_inline_test_lib.Runtime.exit ();;
# actually this is the test run finalizer, see https://github.com/janestreet/ppx_inline_test

# now -c compile the test shim:
# bytecode first
# [34] ocamlc.opt -w -24 -g -bin-annot -I ... -no-alias-deps -o dune__exe__Run.cmo -c -impl test/.ppx_optcomp_test.inline-tests/run.ml-gen
# then native
# [35] ocamlopt.opt -w -24 -g -I ...  -intf-suffix .ml-gen -nodynlink -no-alias-deps -o dune__exe__Run.cmx -c -impl test/.ppx_optcomp_test.inline-tests/run.ml-gen
ocaml_ppx_module(
    name = "ppx_inline_test_shim",
    msg  = "Compiling ppx_inline_test_shim...",
    impl = "@//ocaml/ppx:ppx_inline_test_shim",
    opts = [
        "-w", "-24",
        "-g",
        "-nodynlink",
        "-no-alias-deps",
        "-verbose",
        "-c",
    ],
    deps = [
        # WARNING: used with ocamlfind, linking with ppx_inline_test
        # has the side-effect of adding to the cmd line:
        "@opam//pkg:ppx_inline_test",
        # "@opam//pkg:ppx_inline_test.runner",
    ]
)

# build the test runner executable, linking against ppx_optcomp_test.cmxa
# [36] ocamlopt.opt -w -24 -g -o test/.ppx_optcomp_test.inline-tests/run.exe -linkall ...opam cmxa files and -I ... dune__exe__Run.cmx
ocaml_ppx_binary(
    name = "ppx_optcomp_test_runner.exe",
    srcs = ["@//ocaml/ppx:ppx_inline_test_shim"],
    opts = ["-w", "-24",
            "-g",
            # "-nodynlink",
            # "-no-alias-deps",
            # "-c",
             "-linkall",
            "-linkpkg",
            # "-predicates", "ppx_driver",
            # "-predicates", "ppx_deriving",
             "-verbose",
    ],
    deps = [
        ":ppx_optcomp_test_cmxa",
        # dune secretly injects the following, which
        # injects ppx_inline_test/runner/lib etc.
        "@opam//pkg:ppx_inline_test",
        "@opam//pkg:ppx_inline_test.runner",
        "@opam//pkg:ppx_expect",
        # ":ppx_inline_test_shim",
    ]
)

# finally run the test:
# [37] test/run.exe inline-test-runner ppx_optcomp_test -source-tree-root ../../../.. -diff-cmd -
# ocaml_test(
#     name = "ppx_optcomp_tester"
#     runner = ":ppx_optcomp_test_runner"
# )
