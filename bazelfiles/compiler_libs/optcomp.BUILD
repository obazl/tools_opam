load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "optcomp",
    doc = """Native-code compiler""",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx", "*.o"]),
    cmxa = glob(["*.cmxa", "*.a"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    # archive = select({
    #     "@rules_ocaml//build/mode:bytecode": [
    #         "@ocaml//compiler-libs:ocamloptcomp.cma",
    #     ],
    #     "@rules_ocaml//build/mode:native": [
    #         "@ocaml//compiler-libs:ocamloptcomp.cmxa",
    #         "@ocaml//compiler-libs:ocamloptcomp.a",
    #     ],
    # }),
    all = ["@ocaml//compiler-libs:all"],
    deps = ["@ocaml//compiler-libs/common"],
    visibility = ["//visibility:public"]
)

