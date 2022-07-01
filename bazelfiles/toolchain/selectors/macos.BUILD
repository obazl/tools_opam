# generated file - DO NOT EDIT

load("@rules_ocaml//toolchain:adapter.bzl", "ocaml_toolchain_adapter")

##########
toolchain(
    name           = "macos_x86_64",
    toolchain      = "@ocaml//toolchain/adapters/macos:x86_64",
    toolchain_type = "@rules_ocaml//toolchain:type",
    # exec_compatible_with = [
    #     "@platforms//os:macos",
    #     "@platforms//cpu:x86_64",
    #     "@ocaml//toolchain/platform_constraints/target:local",
    #  ],
    # target_compatible_with = [
    #     "@platforms//os:macos",
    #     "@platforms//cpu:x86_64",
    #  ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    name           = "macos_x86_64__macos_arm",
    toolchain      = "@ocaml//toolchain/adapters/macos:x86_64__arm",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
     ],
    target_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        # "@ocaml//toolchain/adapters/macos:x86_64__macos_arm"
    ],
    visibility             = ["//visibility:public"],
)

##########
# NB: we can select this with
# --platforms=@opam//tc/host/target:linux_x86_64 but CC toolchain
# selection will fail since we do not have a cross-compiling cc
# toolchain.
toolchain(
    name           = "fake_macos_x86_64__linux_x86_64",
    toolchain      =
    "@ocaml//toolchain/adapters/macos:fake_macos_x86_64__linux_x86_64",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    name           = "macos_x86_64__vm",
    toolchain      = "@ocaml//toolchain/adapters/macos:x86_64__vm",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        # "@platforms//os:macos",
        # "@platforms//cpu:x86_64",
        "@ocaml//toolchain/platform_constraints/target:vm",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    name           = "macos_vm_vm",
    toolchain      = "_vm_vm",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        "@ocaml//host/build:vm",
    ],
    target_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        "@ocaml//host/target:vm",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    ## unspecified --host_platform
    name           = "macos_x_vm",
    toolchain      = "_x_vm",
    toolchain_type = "@rules_ocaml//toolchain:type",
    target_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        "@ocaml//host/target:vm",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    ## unspecified --platforms (target host)
    name           = "macos_vm_x",
    toolchain      = "_vm_x",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        "@ocaml//host/build:vm",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    name           = "macos_vm_native",
    toolchain      = "_vm_native",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        "@ocaml//host/build:vm",
    ],
    target_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        "@ocaml//host/target:native",
    ],
    visibility             = ["//visibility:public"],
)

##########
toolchain(
    name           = "default_macos",
    toolchain      = "@ocaml//toolchain/adapters/x86:_macos_default",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
    visibility             = ["//visibility:public"],
)

