# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "dynlink",
    version = "[distributed with OCaml]",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx"]),
    cmxa = glob(["*.cmxa"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    # archive = select({
    #     "@rules_ocaml//build/mode:bytecode": [":dynlink.cma"],
    #     "@rules_ocaml//build/mode:native"  : [
    #         ":dynlink.cmxa",
    #         ":dynlink.a"
    #     ],
    #  }),
    all = glob(["dyn*.*"]),
    visibility = ["//visibility:public"],
);

