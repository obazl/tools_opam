load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "dynlink",
    version = "[distributed with OCaml]",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [":dynlink.cma"],
        "@rules_ocaml//build/mode:native"  : [
            ":dynlink.cmxa",
            ":dynlink.a"
        ],
     }),
    all = glob(["dyn*.*"]),
    visibility = ["//visibility:public"],
);

