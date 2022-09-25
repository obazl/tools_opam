# generated file - DO NOT EDIT

load("@opam//build:rules.bzl", "opam_import")

opam_import(
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

opam_import(
    name       = "plugin",
    version    = "[distributed with OCaml]",
    cmxs       = ["bigarray.cmxs"],
    deps       = ["@ocaml//unix"],
    visibility = ["//visibility:public"],
);
