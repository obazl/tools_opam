load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "toplevel",
    doc = """Toplevel interactions""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [
            ":ocamltoplevel.cma",
        ],
    }),
    all = glob(["*.cmx", "*.cmi"]),
    deps = [":bytecomp"],
    visibility = ["//visibility:public"]
)
