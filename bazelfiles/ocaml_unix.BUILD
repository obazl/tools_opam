# generated file - DO NOT EDIT

load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "unix",
    version = "[distributed with OCaml]",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [":unix.cma"],
        "@rules_ocaml//build/mode:native"  : [
            ":unix.cmxa",
            ":unix.a"
        ],
     }),
    all = glob(["unix*.*"]),
    visibility = ["//visibility:public"],
)

ocaml_import(
    name = "plugin",
    version = "[distributed with OCaml]",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [":unix.cma"],
        "@rules_ocaml//build/mode:native"  : [":unix.cmxs"],
     }),
    all = glob(["unix*.*"]),
    visibility = ["//visibility:public"],
);
