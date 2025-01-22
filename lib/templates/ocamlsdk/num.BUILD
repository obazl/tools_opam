# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name       = "core",
    version    = "[distributed with OCaml]",
    sigs       = glob(["*.cmi"], allow_empty=True),
    archive    =  select({
        "@rules_ocaml//platform/emitter:vm" : "nums.cma",
        "@rules_ocaml//platform/emitter:sys": "nums.cmxa",
    }, no_match_error="Bad platform"),
    afiles   = select({
        "@rules_ocaml//platform/emitter:vm" : [],
        "@rules_ocaml//platform/emitter:sys": glob(["*.a"],
                                                   allow_empty=True,
                                                   exclude=["*_stubs.a"])
    }, no_match_error="Bad platform"),
    astructs = select({
        "@rules_ocaml//platform/emitter:vm" : [],
        "@rules_ocaml//platform/emitter:sys": glob(["*.cmx"], allow_empty=True)
    }, no_match_error="Bad platform"),
    ofiles   = select({
        "@rules_ocaml//platform/emitter:vm" : [],
        "@rules_ocaml//platform/emitter:sys": glob(["*.o"], allow_empty=True)
    }, no_match_error="Bad platform"),
    cmts       = glob(["*.cmt"], allow_empty=True),
    cmtis      = glob(["*.cmti"], allow_empty=True),
    srcs       = glob(["*.ml", "*.mli"], allow_empty=True),
    all        = glob(["*.*"], allow_empty=True),
    visibility = ["//visibility:public"],
)

ocaml_import(
    name       = "plugin",
    version    = "[distributed with OCaml]",
    plugin     =  select({
        "@rules_ocaml//platform/emitter:vm": "nums.cma",
        "//conditions:default":         "nums.cmxs",
    }),
    # cmxs       = "nums.cmxs",
    # cma        = "nums.cma",
    visibility = ["//visibility:public"],
);
