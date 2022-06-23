load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "bytecomp",
    doc = """Bytecode compiler""",
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
    #         "@ocaml//compiler-libs:ocamlbytecomp.cma",
    #     ],
    #     "@rules_ocaml//build/mode:native": [
    #         "@ocaml//compiler-libs:ocamlbytecomp.cmxa",
    #         "@ocaml//compiler-libs:ocamlbytecomp.a",
    #     ],
    # }),
    all = ["@ocaml//compiler-libs:all"],
    deps = ["@ocaml//compiler-libs/common"],
    visibility = ["//visibility:public"]
)
