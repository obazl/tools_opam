# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name       = "bigarray",
    version    = "[distributed with OCaml]",
    cma        = "bigarray.cma",
    cmxa       = "bigarray.cmxa",
    cmi        = glob(["*.cmi"]),
    cmo        = glob(["*.cmo"]),
    cmx        = glob(["*.cmx"]),
    ofiles     = glob(["*.o"]),
    afiles     = glob(["*.a"]),
    cmt        = glob(["*.cmt"]),
    cmti       = glob(["*.cmti"]),
    srcs       = glob(["*.ml", "*.mli"]),
    all        = glob(["bigarray.*"]),

    deps       = ["@ocaml//unix"],
    visibility = ["//visibility:public"],
)

ocaml_import(
    name = "plugin",
    version = "[distributed with OCaml]",
    cmxs       = "bigarray.cmxs",
    deps = ["@ocaml//unix"],
    visibility = ["//visibility:public"],
);
