# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name       = "bigarray",
    version    = "[distributed with OCaml]",
    sigs       = glob(["*.cmi"], allow_empty=True),
    archive    =  select({
        "@rules_ocaml//platform/emitter:vm": "bigarray.cma",
        "@rules_ocaml//platform/emitter:sys": "bigarray.cmxa",
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
    all        = glob(["bigarray.*"], allow_empty=True),

    deps       = ["@opam.ocamlsdk//lib/unix"],
    visibility = ["//visibility:public"],
)

ocaml_import(
    name       = "plugin",
    version    = "[distributed with OCaml]",
    plugin     =  select({
        "@rules_ocaml//platform/emitter:vm": "bigarray.cma",
        "//conditions:default":         "bigarray.cmxs",
    }),
    deps       = ["@opam.ocamlsdk//lib/unix"],
    visibility = ["//visibility:public"],
);
