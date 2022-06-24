# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "str",
    version = """[distributed with Ocaml]""",
    doc = """Regular expressions and string processing""",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx"]),
    ofiles = glob(["*.o"]),
    cmxa = glob(["*.cmxa"]),
    arfiles = glob(["*.a"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    # archive = select({
    #     "@rules_ocaml//build/mode:bytecode": [":str.cma"],
    #     "@rules_ocaml//build/mode:native": [
    #         ":str.cmxa",
    #         ":str.a",
    #     ],
    # }),
    all = glob(["str.*"]),
    visibility = ["//visibility:public"]
)

ocaml_import(
    name = "plugin",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx", "*.o"]),
    cmxa = glob(["*.cmxa", "*.a"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    # plugin = select({
    #     "@rules_ocaml//build/mode:bytecode": [":str.cma"],
    #     "@rules_ocaml//build/mode:native": [":str.cmxs"],
    # }),
    visibility = ["//visibility:public"]
);
