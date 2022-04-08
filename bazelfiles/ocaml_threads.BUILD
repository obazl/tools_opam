load("@rules_ocaml//build:rules.bzl", "ocaml_import")

ocaml_import(
    name = "threads",
    version = "[distributed with OCaml]",
    archive = select({
        "@rules_ocaml//build/mode:bytecode": [":threads.cma"],
        "@rules_ocaml//build/mode:native"  : [
            ":threads.cmxa",
            ":threads.a"
        ],
     }),
    all = glob(["*.cm*", "*.o", "*.a"]),
    deps = [
        ## "@unix//:unix"
        "//unix"
    ],
    visibility = ["//visibility:public"],
);
