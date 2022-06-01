load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "native-toplevel",
    doc = """Toplevel interactions""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [
            ":ocamltoplevel.cmxa",
        ],
    }),
    all = glob(["*.cmx", "*.cmi"]),
    deps = [
        ":optcomp",
        "@ocaml//dynlink"
    ],
    visibility = ["//visibility:public"]
);

