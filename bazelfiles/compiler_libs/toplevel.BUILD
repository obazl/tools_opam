load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "toplevel",
    doc = """Toplevel interactions""",
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
    #         ":ocamltoplevel.cma",
    #     ],
    # }),
    all = glob(["*.cmx", "*.cmi"]),
    deps = [":bytecomp"],
    visibility = ["//visibility:public"]
)
