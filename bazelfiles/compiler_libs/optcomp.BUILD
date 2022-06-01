load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "optcomp",
    doc = """Native-code compiler""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [
            "@ocaml//compiler-libs:ocamloptcomp.cma",
        ],
        "@rules_ocaml//build/mode:native": [
            "@ocaml//compiler-libs:ocamloptcomp.cmxa",
            "@ocaml//compiler-libs:ocamloptcomp.a",
        ],
    }),
    all = ["@ocaml//compiler-libs:all"],
    deps = ["@ocaml//compiler-libs/common"],
    visibility = ["//visibility:public"]
)

