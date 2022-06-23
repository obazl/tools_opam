# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "core",
    version = "[distributed with OCaml]",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx", "*.o"]),
    cmxa = glob(["*.cmxa", "*.a"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    # archive = select({
    #     "@rules_ocaml//build/mode:bytecode": [":nums.cma"],
    #     "@rules_ocaml//build/mode:native"  : [
    #         ":nums.cmxa",
    #         ":nums.a"
    #     ],
    #  }),
    # all = glob(["nums.*"]),
    visibility = ["//visibility:public"],
);

ocaml_import(
    name = "plugin",
    version = "[distributed with OCaml]",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx", "*.o"]),
    cmxa = glob(["*.cmxa", "*.a"]),
    cma  = glob(["*.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    # archive = select({
    #     "@rules_ocaml//build/mode:bytecode": [":nums.cma"],
    #     "@rules_ocaml//build/mode:native"  : [":nums.cmxs"],
    #  }),
    # all = glob(["nums.*"]),
    visibility = ["//visibility:public"],
);
