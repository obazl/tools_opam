# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "bytecomp",
    doc = """Common compiler routines""",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx"]),
    ofiles = glob(["*.o"]),
    cmxa = glob(["ocamlbytecomp.cmxa"]),
    arfiles = glob(["ocamlbytecomp.a"]),
    cma  = glob(["ocamlbytecomp.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    all = glob(["*"]),
    visibility = ["//visibility:public"]
)

ocaml_import(
    name = "common",
    doc = """Common compiler routines""",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx"]),
    ofiles = glob(["*.o"]),
    cmxa = glob(["ocamlcommon.cmxa"]),
    arfiles = glob(["ocamlcommon.a"]),
    cma  = glob(["ocamlcommon.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    all = glob(["*"]),
    visibility = ["//visibility:public"]
)

ocaml_import(
    name = "optcomp",
    doc = """optcomp compiler routines""",
    cmi  = glob(["*.cmi"]),
    cmti = glob(["*.cmti"]),
    cmo  = glob(["*.cmo"]),
    cmx  = glob(["*.cmx"]),
    ofiles = glob(["*.o"]),
    cmxa = glob(["ocamloptcomp.cmxa"]),
    arfiles = glob(["ocamloptcomp.a"]),
    cma  = glob(["ocamloptcomp.cma"]),
    cmxs = glob(["*.cmxs"]),
    srcs = glob(["*.ml", "*.mli"]),
    all = glob(["*"]),
    visibility = ["//visibility:public"]
)

exports_files([
    "ocamlcommon.cma",
    "ocamlcommon.cmxa",
    "ocamlcommon.a",

    "ocamlbytecomp.cma",
    "ocamlbytecomp.cmxa",
    "ocamlbytecomp.a",

    "ocamloptcomp.cma",
    "ocamloptcomp.cmxa",
    "ocamloptcomp.a",

    "ocamlmiddleend.cma",
    "ocamlmiddleend.cmxa",
    "ocamlmiddleend.a",

    "ocamltoplevel.cma",
    "ocamltoplevel.cmxa",
    "ocamltoplevel.a",
])

# null import - other stuff depends on this, even though
# META does not put anything into it...

## HOWEVER, the official docs indirectly suggest the base dep should
## be compiler-libs.common, not compiler-libs
# ocaml_import(
#     name = "compiler-libs",
#     doc = """Common compiler routines""",
#     archive = select({
#         "@rules_ocaml//build/mode:bytecode": [
#         ],
#         "@rules_ocaml//build/mode:native": [
#         ],
#     }),
#     visibility = ["//visibility:public"]
# )

filegroup(
    name = "all",
    srcs = glob([
        "*.cmx", "*.o", "*.cmo",
        "*.cmt", "*.cmti",
        "*.mli", "*.cmi",
    ]),
    visibility = ["@ocaml//compiler-libs:__subpackages__"]
)
