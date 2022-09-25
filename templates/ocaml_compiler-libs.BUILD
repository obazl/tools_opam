# generated file - DO NOT EDIT

load("@opam//build:rules.bzl", "opam_import")

opam_import(
    name       = "common",
    doc        = """Common compiler routines""",
    cma        = "ocamlcommon.cma",
    cmxa       = "ocamlcommon.cmxa",
    cmi        = glob(["*.cmi"]),
    cmo        = glob(["*.cmo"]),
    cmx        = glob(["*.cmx"]),
    ofiles     = glob(["*.o"]),
    afiles     = glob(["*.a"]),
    cmt        = glob(["*.cmt"]),
    cmti       = glob(["*.cmti"]),
    srcs       = glob(["*.ml", "*.mli"]),
    all        = glob(["*"]),
    visibility = ["//visibility:public"]
)

opam_import(
    name       = "bytecomp",
    doc        = """Common compiler routines""",
    cma        = "ocamlbytecomp.cma",
    cmxa       = "ocamlbytecomp.cmxa",
    cmi        = glob(["*.cmi"]),
    cmo        = glob(["*.cmo"]),
    cmx        = glob(["*.cmx"]),
    ofiles     = glob(["*.o"]),
    afiles     = glob(["*.a"]),
    cmt        = glob(["*.cmt"]),
    cmti       = glob(["*.cmti"]),
    srcs       = glob(["*.ml", "*.mli"]),
    all        = glob(["*"]),
    visibility = ["//visibility:public"]
)

opam_import(
    name       = "optcomp",
    doc        = """optcomp compiler routines""",
    cma        = "ocamloptcomp.cma",
    cmxa       = "ocamloptcomp.cmxa",
    cmi        = glob(["*.cmi"]),
    cmo        = glob(["*.cmo"]),
    cmx        = glob(["*.cmx"]),
    ofiles     = glob(["*.o"]),
    afiles     = glob(["*.a"]),
    cmt        = glob(["*.cmt"]),
    cmti       = glob(["*.cmti"]),
    srcs       = glob(["*.ml", "*.mli"]),
    all        = glob(["*"]),
    visibility = ["//visibility:public"]
)

opam_import(
    name = "toplevel",
    doc = """Toplevel interactions""",
    cma  = "ocamltoplevel.cma",
    # cmxa = "ocamltoplevel.cmxa",
    cmi  = glob(["*.cmi"]),
    cmo  = glob(["*.cmo"]),
    # cmx  = glob(["*.cmx"]),
    # ofiles = glob(["*.o"]),
    # afiles = glob(["*.a"]),
    cmti = glob(["*.cmti"]),
    srcs = glob(["*.ml", "*.mli"]),
    all = glob(["*.cmx", "*.cmi"]),
    deps = [":bytecomp"],
    visibility = ["//visibility:public"]
)

# exports_files([
#     "ocamlcommon.cma",
#     "ocamlcommon.cmxa",
#     "ocamlcommon.a",

#     "ocamlbytecomp.cma",
#     "ocamlbytecomp.cmxa",
#     "ocamlbytecomp.a",

#     "ocamloptcomp.cma",
#     "ocamloptcomp.cmxa",
#     "ocamloptcomp.a",

#     "ocamlmiddleend.cma",
#     "ocamlmiddleend.cmxa",
#     "ocamlmiddleend.a",

#     "ocamltoplevel.cma",
#     "ocamltoplevel.cmxa",
#     "ocamltoplevel.a",
# ])

# filegroup(
#     name = "all",
#     srcs = glob([
#         "*.cmx", "*.o", "*.cmo",
#         "*.cmt", "*.cmti",
#         "*.mli", "*.cmi",
#     ]),
#     visibility = ["@ocaml//compiler-libs:__subpackages__"]
# )
