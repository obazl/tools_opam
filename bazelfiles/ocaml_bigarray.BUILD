# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "bigarray",
    version = "[distributed with OCaml]",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmt = glob(["*.cmt"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx"]),
    cmxa = glob(["*.cmxa"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    # archive = select({
    #     "@rules_ocaml//build/mode:bytecode": [":bigarray.cma"],
    #     "@rules_ocaml//build/mode:native"  : [
    #         ":bigarray.cmxa",
    #         ":bigarray.a"
    #     ],
    #  }),
    all = glob(["bigarray.*"]),
    deps = ["@ocaml//unix"],
    visibility = ["//visibility:public"],
)

ocaml_import(
    name = "plugin",
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
    #     "@rules_ocaml//build/mode:bytecode": [":bigarray.cma"],
    #     "@rules_ocaml//build/mode:native"  : [":bigarray.cmxs"],
    #  }),
    all = glob(["bigarray.*"]),
    deps = ["@ocaml//unix"],
    visibility = ["//visibility:public"],
);
