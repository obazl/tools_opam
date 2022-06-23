# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "unix",
    version = "[distributed with OCaml]",
    # archive = select({
    #     "@rules_ocaml//build/mode:bytecode": [":unix.cma"],
    #     "@rules_ocaml//build/mode:native"  : [
    #         ":unix.cmxa",
    #         ":unix.a"
    #     ],
    #  }),
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx", "*.o"]),
    cmxa = glob(["*.cmxa", "*.a"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    all = glob(["unix*.*"]),
    visibility = ["//visibility:public"],
)

ocaml_import(
    name = "plugin",
    version = "[distributed with OCaml]",
    # archive = select({
    #     "@rules_ocaml//build/mode:bytecode": [":unix.cma"],
    #     "@rules_ocaml//build/mode:native"  : [":unix.cmxs"],
    #  }),
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx", "*.o"]),
    cmxa = glob(["*.cmxa", "*.a"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    all = glob(["unix*.*"]),
    visibility = ["//visibility:public"],
);
