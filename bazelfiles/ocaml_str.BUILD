load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "str",
    version = """[distributed with Ocaml]""",
    doc = """Regular expressions and string processing""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [":str.cma"],
        "@rules_ocaml//build/mode:native": [
            ":str.cmxa",
            ":str.a",
        ],
    }),
    all = glob(["str.*"]),
    visibility = ["//visibility:public"]
)

ocaml_import(
    name = "plugin",
    plugin = select({
        "@rules_ocaml//build/mode:bytecode": [":str.cma"],
        "@rules_ocaml//build/mode:native": [":str.cmxs"],
    }),
    visibility = ["//visibility:public"]
);
