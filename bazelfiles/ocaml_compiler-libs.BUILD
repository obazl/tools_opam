load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "compiler-libs",
    ## version = """[distributed with Ocaml]""",
    doc = """Compiler-libs support library""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [],
        "@rules_ocaml//build/mode:native": [],
    }),
    all = glob(["*.cm*", "*.o", "*.a"]),
    visibility = ["//visibility:public"]
)

ocaml_import(
    name = "common",
    doc = """Common compiler routines""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [
            ":ocamlcommon.cma",
        ],
        "@rules_ocaml//build/mode:native": [
            ":ocamlcommon.cmxa",
            ":ocamlcommon.a",
        ],
    }),
    deps = [
        ":compiler-libs",
    ],
    visibility = ["//visibility:public"]
)

ocaml_import(
    name = "bytecomp",
    doc = """Bytecode compiler""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [
            ":ocamlbytecomp.cma",
        ],
        "@rules_ocaml//build/mode:native": [
            ":ocamlbytecomp.cmxa",
            ":ocamlbytecomp.a",
        ],
    }),
    all = glob(["*.cmx", "*.cmi"]),
    deps = [":common"],
    visibility = ["//visibility:public"]
)

ocaml_import(
    name = "optcomp",
    doc = """Native-code compiler""",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [
            ":ocamloptcomp.cma",
        ],
        "@rules_ocaml//build/mode:native": [
            ":ocamloptcomp.cmxa",
            ":ocamloptcomp.a",
        ],
    }),
    all = glob(["*.cmx", "*.cmi"]),
    deps = [":common"],
    visibility = ["//visibility:public"]
)

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

