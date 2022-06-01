load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "core",
    version = "[distributed with OCaml]",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [":nums.cma"],
        "@rules_ocaml//build/mode:native"  : [
            ":nums.cmxa",
            ":nums.a"
        ],
     }),
    # all = glob(["nums.*"]),
    visibility = ["//visibility:public"],
);

ocaml_import(
    name = "plugin",
    version = "[distributed with OCaml]",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [":nums.cma"],
        "@rules_ocaml//build/mode:native"  : [":nums.cmxs"],
     }),
    # all = glob(["nums.*"]),
    visibility = ["//visibility:public"],
);
