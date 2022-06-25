# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name       = "core",
    version    = "[distributed with OCaml]",
    cma        = "nums.cma",
    cmxa       = "nums.cmxa",
    cmi        = glob(["*.cmi"]),
    cmo        = glob(["*.cmo"]),
    cmx        = glob(["*.cmx"]),
    ofiles     = glob(["*.o"]),
    afiles     = glob(["*.a"]),
    cmt        = glob(["*.cmt"]),
    cmti       = glob(["*.cmti"]),
    srcs       = glob(["*.ml", "*.mli"]),
    all        = glob(["*.*"]),
    visibility = ["//visibility:public"],
)

ocaml_import(
    name       = "plugin",
    version    = "[distributed with OCaml]",
    cmxs       = "nums.cmxs",
    visibility = ["//visibility:public"],
);
