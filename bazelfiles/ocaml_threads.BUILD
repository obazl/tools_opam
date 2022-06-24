# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "threads",
    version = "[distributed with OCaml]",
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
    #     "@rules_ocaml//build/mode:bytecode": [":threads.cma"],
    #     "@rules_ocaml//build/mode:native"  : [
    #         ":threads.cmxa",
    #         ":threads.a"
    #     ],
    #  }),
    all = glob(["*.cm*", "*.o", "*.a"]),
    deps = [
        ## "@unix//:unix"
        "//unix"
    ],
    visibility = ["//visibility:public"],
);
