load("@obazl//ocaml:build.bzl",
     "ocaml_binary",
     "ocaml_library",
     "ocaml_ppx_binary",
     "ocaml_ppx_library")

# dune: generates _ppx.ml == let () = Ppxlib.Driver.standalone ()
# but ppxlib.runner == Ppxlib.Driver.standalone ()
# either seems to work...
genrule(
    name = "ppxlib_driver",
    visibility = ["//visibility:public"],
    message = "Generating ppxlib driver...",
    outs = [
        "ppxlib_driver.ml",
    ],
    cmd = "echo let \(\) = Ppxlib.Driver.standalone \(\) > $@;"
)

# compile ppx pipeline
ocaml_ppx_binary(
    name = "ppx",
    message = "Compiling preprocessor pipeline...",
    visibility = ["//visibility:public"],
    # srcs = [":ppxlib_driver"],
    copts = ["-linkall",
             "-linkpkg",
             "-predicates", "ppx_driver",
             # "-verbose"
    ],
    deps = [
        "@opam//pkg:core_kernel",
        "@opam//pkg:ppxlib.metaquot",
        "@opam//pkg:ppx_sexp_conv",
        "@opam//pkg:ppxlib.runner",
    ]
)

# preprocess sources
genrule(
    name = "preproc",
    visibility = ["//visibility:public"],
    message = "Preprocessing sources...",
    tools = [":ppx"],
    srcs = [
        "snarkydef.ml",
        "snarky_module.ml",
        "ppx_snarky.ml",
    ],
    outs = [
        "Ppx_snarky__Snarkydef.ml",
        "Ppx_snarky__Snarky_module.ml",
        "Ppx_snarky__Ppx_snarky.ml",
    ],
    cmd = "for f in $(SRCS);"
    + "do"
    + "    BNAME=`basename $$f`;"
    + "    PGM=$(location :ppx);"
    + "    $(location :ppx)"
    # + "    --dump-ast"
    + "    $$f > $(@D)/Ppx_snarky__$$BNAME;"
    + " done"
)

## compile the transformed srcs to a ppx lib
# for the ppx, dune injects _ppx.ml == let () = Ppxlib.Driver.standalone ()
# dune generates a wrapper, ppx_snarky__.ml-gen, containing
## module Snarky_module = Ppx_snarky__Snarky_module
## module Snarkydef = Ppx_snarky__Snarkydef
## ppx_snarky__.ml-gen compiled to ppx_snarky__.cmx
genrule(
    name = "wrapper_gen",
    visibility = ["//visibility:public"],
    message = "Generating ppxlib driver...",
    outs = [
        "Ppx_snarky__.ml",
    ],
    cmd = "echo module Snarky_module = Ppx_snarky__Snarky_module > $@;"
    + " echo module Snarkydef = Ppx_snarky__Snarkydef >> $@;"
)

# now compile the wrapper
ocaml_library(
    name = "wrapper",
    # message = "Compiling preprocessed sources...",
    srcs = [
        ":wrapper_gen",
    ],
    copts = ["-no-alias-deps",
             "-opaque",
             "-c",
             "-linkall",
             # "-verbose",
             # "-ccopt", "-v"
    ],
    # deps = [
    #     "@opam//pkg:ppxlib",
    #     "@opam//pkg:ppx_tools",
    #     "@opam//pkg:core_kernel",
    #     # "@opam//pkg:ppxlib.metaquot",
    #     # "@opam//pkg:ppx_sexp_conv",
    #     # "@opam//pkg:ppxlib.runner"
    # ]
)

ocaml_library(
    name = "ppx_snarky",
    # message = "Compiling preprocessed sources...",
    srcs = [
        ":preproc",
    ],
    copts = ["-linkall",
             "-predicates", "ppx_driver",
             "-no-alias-deps",
             "-opaque",

             ## to produce a set of cmi/cmx/o files: -c -linkpkg
             "-c",
             "-linkpkg",

             ## to produce a cmxa: -a
             # "-a",

             "-open", "Ppx_snarky__",
             # "-verbose"
    ],
    deps = [
        ":wrapper",
        "@opam//pkg:ppxlib",
        "@opam//pkg:ppx_tools",
        "@opam//pkg:core_kernel",
        # "@opam//pkg:ppxlib.metaquot",
        # "@opam//pkg:ppx_sexp_conv",
        # "@opam//pkg:ppxlib.runner"
    ]
)

# compile new ppx pipeline
ocaml_ppx_binary(
    name = "ppx_snarky_exe",
    message = "Compiling preprocessor pipeline...",
    visibility = ["//visibility:public"],
    copts = ["-linkall",
             "-linkpkg",
             "-predicates", "ppx_driver",
             "-verbose"
    ],
    deps = [
        "@opam//pkg:core_kernel",
        "@opam//pkg:ppxlib",
        ":ppx_snarky",
        "@opam//pkg:ppxlib.runner",
    ]
)
