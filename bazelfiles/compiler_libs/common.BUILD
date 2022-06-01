load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "common",
    doc = """Common compiler routines""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [
            "@ocaml//compiler-libs:ocamlcommon.cma",
        ],
        "@rules_ocaml//build/mode:native": [
            "@ocaml//compiler-libs:ocamlcommon.cmxa",
            "@ocaml//compiler-libs:ocamlcommon.a",
        ],
    }),
    all = ["@ocaml//compiler-libs:all"],
    visibility = ["//visibility:public"]
)

