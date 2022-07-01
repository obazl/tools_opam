# generated file - DO NOT EDIT

load("@rules_ocaml//toolchain:adapter.bzl", "ocaml_toolchain_adapter")

##########
toolchain(
    name           = "linux_x86_64",
    toolchain      = "@ocaml//toolchain/adapters/linux:x86_64",
    toolchain_type = "@rules_ocaml//toolchain:type",
    # exec_compatible_with = [
    #     "@platforms//os:linux",
    #     "@platforms//cpu:x86_64",
    # ],
    # target_compatible_with = [
    #     "@platforms//os:linux",
    #     "@platforms//cpu:x86_64",
    # ],
    visibility             = ["//visibility:public"],
)

##########
# NB: we can select this with
# --platforms=@opam//tc/host/target:linux_x86_64 but CC toolchain
# selection will fail since we do not have a cross-compiling cc
# toolchain.
toolchain(
    name           = "fake_linux_x86_64__macos_x86_64",
    toolchain      =
    "@ocaml//toolchain/adapters/linux:fake_linux_x86_64__macos_x86_64",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    name           = "linux_x86_64__vm",
    toolchain      = "@ocaml//toolchain/adapters/linux:x86_64__vm",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        # "@platforms//os:linux",
        # "@platforms//cpu:x86_64",
        "@ocaml//toolchain/platform_constraints/target:vm",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    name           = "linux_vm__vm",
    toolchain      = "@ocaml//toolchain/adapters/linux:vm__vm",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
        "@ocaml//toolchain/platform/build:vm",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
        "@ocaml//toolchain/platform/build:vm",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    name           = "linux_vm_x86_64",
    toolchain      = "@ocaml//toolchain/adapters/linux:vm__x86_64",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    visibility             = ["//visibility:public"],
)
