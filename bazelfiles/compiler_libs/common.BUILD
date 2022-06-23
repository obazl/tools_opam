load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "common",
    doc = """Common compiler routines""",
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
    #         "@ocaml//compiler-libs:ocamlcommon.cma",
    #     ],
    #     "@rules_ocaml//build/mode:native": [
    #         "@ocaml//compiler-libs:ocamlcommon.cmxa",
    #         "@ocaml//compiler-libs:ocamlcommon.a",
    #     ],
    # }),
    all = ["@ocaml//compiler-libs:all"],
    visibility = ["//visibility:public"]
)

