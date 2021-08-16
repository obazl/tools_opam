# design

## two lists v. dict

ppx = [], ppx_args = []
vs.
ppx = { ppx: [args] }

Example:

PPX_EXE = "//ppx:foo"
PPX_ARGS = [
    # do not sort (buildifier)
    "-cookie", "library-name=\"snarky_intf\"",
    "-corrected-suffix", ".ppx-corrected",
]
## A:
ocaml_module(
    name = "vector.cm_",
    impl = "vector.ml",
    ppx = PPX_EXE,
    ppx_args = PPX_ARGS,
    visibility = ["//visibility:public"],
    deps = ["@opam//pkg:core"]
)
## B:
ocaml_module(
    name = "vector.cm_",
    impl = "vector.ml",
    ppx = { PPX_EXE: " ".join(PPX_ARGS) },
    visibility = ["//visibility:public"],
    deps = ["@opam//pkg:core"]
)
## C:
ocaml_module(
    name = "vector.cm_",
    impl = "vector.ml",
    ppx = {
        "//ppx:foo": "-cookie 'library-name="snarky_intf"' -corrected-suffix .ppx-corrected"
    }
    visibility = ["//visibility:public"],
    deps = ["@opam//pkg:core"]
)
