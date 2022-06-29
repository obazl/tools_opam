# generated file - DO NOT EDIT

load("@rules_ocaml//toolchain:adapter.bzl", "ocaml_toolchain_adapter")

## toolchain bindings (to be passed to 'register_toolchains' function)

##########
toolchain(
    name           = "default_macos",
    toolchain      = "_opam_macos_default",
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

##########
toolchain(
    name           = "macos_opam_native_native",
    toolchain      = "_opam_native_native",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        "@ocaml//host/build:native",
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
    name           = "macos_opam_native_vm",
    toolchain      = "_opam_native_vm",
    toolchain_type = "@rules_ocaml//toolchain:type",
    exec_compatible_with = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
        "@ocaml//host/build:native",
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
    name           = "macos_opam_vm_vm",
    toolchain      = "_opam_vm_vm",
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
    name           = "macos_opam_x_vm",
    toolchain      = "_opam_x_vm",
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
    name           = "macos_opam_vm_x",
    toolchain      = "_opam_vm_x",
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
    name           = "macos_opam_vm_native",
    toolchain      = "_opam_vm_native",
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
    name           = "linux_opam_native_native",
    toolchain      = "_opam_native_native",
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

##########
toolchain(
    name           = "linux_opam_native_vm",
    toolchain      = "_opam_native_vm",
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

##########
toolchain(
    name           = "linux_opam_vm_vm",
    toolchain      = "_opam_vm_vm",
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

##########
toolchain(
    name           = "linux_opam_vm_native",
    toolchain      = "_opam_vm_native",
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

##########
toolchain(
    name           = "default_linux",
    toolchain      = "_opam_native_native",
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


####################################
    #### toolchain adapters ####
ocaml_toolchain_adapter(
    name                   = "_opam_vm_native",
    host                   = "vm",
    target                 = "native",
    repl                   = "@ocaml//bin:ocaml",
    compiler               = "@ocaml//bin:ocamlopt.byte",
    profiling_compiler     = "@ocaml//bin:ocamloptp.byte",
    ocamllex               = "@ocaml//bin:ocamllex.byte",
    ocamlyacc              = "@ocaml//bin:ocamlyacc",
    linkmode               = "dynamic",
    vmruntime              = "@ocaml//bin:ocamlrun",
    vmruntime_debug        = "@ocaml//bin:ocamlrund",
    vmruntime_instrumented = "@ocaml//bin:ocamlruni",
    vmlibs                 = "@stublibs//:stublibs",
)

ocaml_toolchain_adapter(
    name                   = "_opam_native_native",
    host                   = "native",
    target                 = "native",
    repl                   = "@ocaml//bin:ocaml",
    compiler               = "@ocaml//bin:ocamlopt.opt",
    profiling_compiler     = "@ocaml//bin:ocamloptp.opt",
    ocamllex               = "@ocaml//bin:ocamllex.opt",
    ocamlyacc              = "@ocaml//bin:ocamlyacc",
    linkmode               = "dynamic",
    vmruntime              = "@ocaml//bin:ocamlrun",
    vmruntime_debug        = "@ocaml//bin:ocamlrund",
    vmruntime_instrumented = "@ocaml//bin:ocamlruni",
    vmlibs                 = "@stublibs//:stublibs",
)

ocaml_toolchain_adapter(
    name                   = "_opam_native_vm",
    host                   = "native",
    target                 = "vm",
    repl                   = "@ocaml//bin:ocaml",
    compiler               = "@ocaml//bin:ocamlc.opt",
    profiling_compiler     = "@ocaml//bin:ocamlcp.opt",
    ocamllex               = "@ocaml//bin:ocamllex.opt",
    ocamlyacc              = "@ocaml//bin:ocamlyacc",
    linkmode               = "dynamic",
    vmruntime              = "@ocaml//bin:ocamlrun",
    vmruntime_debug        = "@ocaml//bin:ocamlrund",
    vmruntime_instrumented = "@ocaml//bin:ocamlruni",
    vmlibs                 = "@stublibs//:stublibs",
)

ocaml_toolchain_adapter(
    name                   = "_opam_vm_vm",
    host                   = "vm",
    target                 = "vm",
    repl                   = "@ocaml//bin:ocaml",
    compiler               = "@ocaml//bin:ocamlc.byte",
    profiling_compiler     = "@ocaml//bin:ocamlcp.byte",
    ocamllex               = "@ocaml//bin:ocamllex.byte",
    ocamlyacc              = "@ocaml//bin:ocamlyacc",
    linkmode               = "dynamic",
    vmruntime              = "@ocaml//bin:ocamlrun",
    vmruntime_debug        = "@ocaml//bin:ocamlrund",
    vmruntime_instrumented = "@ocaml//bin:ocamlruni",
    vmlibs                 = "@stublibs//:stublibs",
)

ocaml_toolchain_adapter(
    ## unspecified host defaults to native
    name                   = "_opam_x_vm",
    host                   = "native",
    target                 = "vm",
    repl                   = "@ocaml//bin:ocaml",
    compiler               = "@ocaml//bin:ocamlc.opt",
    profiling_compiler     = "@ocaml//bin:ocamlcp.opt",
    ocamllex               = "@ocaml//bin:ocamllex.opt",
    ocamlyacc              = "@ocaml//bin:ocamlyacc",
    linkmode               = "dynamic",
    vmruntime              = "@ocaml//bin:ocamlrun",
    vmruntime_debug        = "@ocaml//bin:ocamlrund",
    vmruntime_instrumented = "@ocaml//bin:ocamlruni",
    vmlibs                 = "@stublibs//:stublibs",
)

ocaml_toolchain_adapter(
    name                   = "_opam_vm_x",
    host                   = "vm",
    target                 = "native",
    repl                   = "@ocaml//bin:ocaml",
    compiler               = "@ocaml//bin:ocamlopt.byte",
    profiling_compiler     = "@ocaml//bin:ocamloptp.byte",
    ocamllex               = "@ocaml//bin:ocamllex.byte",
    ocamlyacc              = "@ocaml//bin:ocamlyacc",
    linkmode               = "dynamic",
    vmruntime              = "@ocaml//bin:ocamlrun",
    vmruntime_debug        = "@ocaml//bin:ocamlrund",
    vmruntime_instrumented = "@ocaml//bin:ocamlruni",
    vmlibs                 = "@stublibs//:stublibs",
)

ocaml_toolchain_adapter(
    name                   = "_opam_macos_default",
    host                   = "native",
    target                 = "native",
    repl                   = "@ocaml//bin:ocaml",
    compiler               = "@ocaml//bin:ocamlopt.opt",
    profiling_compiler     = "@ocaml//bin:ocamloptp.opt",
    ocamllex               = "@ocaml//bin:ocamllex.opt",
    ocamlyacc              = "@ocaml//bin:ocamlyacc",
    linkmode               = "dynamic",
    vmruntime              = "@ocaml//bin:ocamlrun",
    vmruntime_debug        = "@ocaml//bin:ocamlrund",
    vmruntime_instrumented = "@ocaml//bin:ocamlruni",
    vmlibs                 = "@stublibs//:stublibs",
)

