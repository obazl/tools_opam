load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "bytecomp",
    doc = """Bytecode compiler""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [
            "@ocaml//compiler-libs:ocamlbytecomp.cma",
        ],
        "@rules_ocaml//build/mode:native": [
            "@ocaml//compiler-libs:ocamlbytecomp.cmxa",
            "@ocaml//compiler-libs:ocamlbytecomp.a",
        ],
    }),
    all = ["@ocaml//compiler-libs:all"],
    deps = ["@ocaml//compiler-libs/common"],
    visibility = ["//visibility:public"]
)
