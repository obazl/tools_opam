# generated file - DO NOT EDIT

exports_files(["BUILD.bazel"])

load("@rules_ocaml//toolchain:BUILD.bzl", "ocaml_toolchain_adapter")

# load("@toolchains_ocaml//build:rules.bzl",
#      "ocaml_toolchain_adapter")

########################
ocaml_toolchain_adapter(
    name                   = "ocamlopt.opt",
    host                   = "sys",
    target                 = "sys",
    repl                   = "@{{pfx}}ocamlsdk//bin:ocaml",
    compiler               = "@{{pfx}}ocamlsdk//bin:ocamlopt.opt",
    version              = "//version",
    profiling_compiler   = "@{{pfx}}ocamlsdk//bin:ocamloptp",
    ocamllex             = "@{{pfx}}ocamlsdk//bin:ocamllex.opt",
    ocamlyacc            = "@{{pfx}}ocamlsdk//bin:ocamlyacc",
    linkmode             = "dynamic",
    # stdlib             = "@{{pfx}}ocamlsdk//runtime:stdlib.cma",
    # std_exit           = "@{{pfx}}ocamlsdk//runtime:std_exit.cmo",

    default_runtime      = "@{{pfx}}ocamlsdk//runtime:libasmrun.a",
    std_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@{{pfx}}ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@{{pfx}}ocamlsdk//runtime:libasmrun_shared.so",
    dllibs               = "@{{pfx}}stublibs//lib/stublibs",
    dllibs_path          = "@{{pfx}}stublibs//lib/stublibs:path",

    ## omit vm stuff for >sys toolchains?
    # vmruntime            = "@{{pfx}}ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@{{pfx}}ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@{{pfx}}ocamlsdk//bin:ocamlruni",
)

ocaml_toolchain_adapter(
    ## debug syssys tc
    name                   = "ocamlopt.opt.d",
    host                   = "sys",
    target                 = "sys",
    repl                   = "@{{pfx}}ocamlsdk//bin:ocaml",
    compiler               = "@{{pfx}}ocamlsdk//bin:ocamlopt.opt",
    version                = "//version",
    profiling_compiler     = "@{{pfx}}ocamlsdk//bin:ocamloptp",
    ocamllex               = "@{{pfx}}ocamlsdk//bin:ocamllex.opt",
    ocamlyacc              = "@{{pfx}}ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@{{pfx}}ocamlsdk//runtime:libasmrund.a",
    std_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@{{pfx}}ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@{{pfx}}ocamlsdk//runtime:libasmrun_shared.so",
    dllibs               = "@{{pfx}}stublibs//lib/stublibs",
    dllibs_path          = "@{{pfx}}stublibs//lib/stublibs:path",

    ## omit vm stuff for >sys toolchains?
    # vmruntime              = "@{{pfx}}ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@{{pfx}}ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@{{pfx}}ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlc.opt",
    host                   = "sys",
    target                 = "vm",
    repl                   = "@{{pfx}}ocamlsdk//bin:ocaml",
    compiler               = "@{{pfx}}ocamlsdk//bin:ocamlc.opt",
    version                = "//version",
    profiling_compiler     = "@{{pfx}}ocamlsdk//bin:ocamlcp",
    ocamllex               = "@{{pfx}}ocamlsdk//bin:ocamllex.opt",
    ocamlyacc              = "@{{pfx}}ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@{{pfx}}ocamlsdk//runtime:libcamlrun.a",
    std_runtime          = "@{{pfx}}ocamlsdk//runtime:libcamlrun.a",
    dbg_runtime          = "@{{pfx}}ocamlsdk//runtime:libcamlrund.a",
    instrumented_runtime = "@{{pfx}}ocamlsdk//runtime:libcamlruni.a",
    pic_runtime          = "@{{pfx}}ocamlsdk//runtime:libcamlrun_pic.a",
    shared_runtime       = "@{{pfx}}ocamlsdk//runtime:libcamlrun_shared.so",
    dllibs               = "@{{pfx}}stublibs//lib/stublibs",
    # dllibs               = "@{{pfx}}stublibs//lib/stublibs",
    dllibs_path          = "@{{pfx}}stublibs//lib/stublibs:path",

    # vmruntime              = "@{{pfx}}ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@{{pfx}}ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@{{pfx}}ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlc.byte",
    host                   = "vm",
    target                 = "vm",
    repl                   = "@{{pfx}}ocamlsdk//bin:ocaml",
    compiler               = "@{{pfx}}ocamlsdk//bin:ocamlc.byte",
    version                = "//version",
    profiling_compiler     = "@{{pfx}}ocamlsdk//bin:ocamlcp",
    ocamllex               = "@{{pfx}}ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@{{pfx}}ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@{{pfx}}ocamlsdk//runtime:libcamlrun.a",
    std_runtime          = "@{{pfx}}ocamlsdk//runtime:libcamlrun.a",
    dbg_runtime          = "@{{pfx}}ocamlsdk//runtime:libcamlrund.a",
    instrumented_runtime = "@{{pfx}}ocamlsdk//runtime:libcamlruni.a",
    pic_runtime          = "@{{pfx}}ocamlsdk//runtime:libcamlrun_pic.a",
    shared_runtime       = "@{{pfx}}ocamlsdk//runtime:libcamlrun_shared.so",
    dllibs               = "@{{pfx}}stublibs//lib/stublibs",
    dllibs_path          = "@{{pfx}}stublibs//lib/stublibs:path",

    # vmruntime              = "@{{pfx}}ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@{{pfx}}ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@{{pfx}}ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlopt.byte",
    host                   = "vm",
    target                 = "sys",
    repl                   = "@{{pfx}}ocamlsdk//bin:ocaml",
    compiler               = "@{{pfx}}ocamlsdk//bin:ocamlopt.byte",
    version                = "//version",
    profiling_compiler     = "@{{pfx}}ocamlsdk//bin:ocamloptp",
    ocamllex               = "@{{pfx}}ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@{{pfx}}ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@{{pfx}}ocamlsdk//runtime:libasmrun.a",
    std_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@{{pfx}}ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@{{pfx}}ocamlsdk//runtime:libasmrun_shared.so",
    dllibs               = "@{{pfx}}stublibs//lib/stublibs",
    dllibs_path          = "@{{pfx}}stublibs//lib/stublibs:path",

    # vmruntime              = "@{{pfx}}ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@{{pfx}}ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@{{pfx}}ocamlsdk//bin:ocamlruni",
)

ocaml_toolchain_adapter(
    name                   = "ocamlopt.byte.d",
    host                   = "vm",
    target                 = "sys",
    repl                   = "@{{pfx}}ocamlsdk//bin:ocaml",
    compiler               = "@{{pfx}}ocamlsdk//bin:ocamlopt.byte",
    version                = "//version",
    profiling_compiler     = "@{{pfx}}ocamlsdk//bin:ocamloptp",
    ocamllex               = "@{{pfx}}ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@{{pfx}}ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@{{pfx}}ocamlsdk//runtime:libasmrund.a",
    std_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@{{pfx}}ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@{{pfx}}ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@{{pfx}}ocamlsdk//runtime:libasmrun_shared.so",
    dllibs               = "@{{pfx}}stublibs//lib/stublibs",
    dllibs_path          = "@{{pfx}}stublibs//lib/stublibs:path",

    # vmruntime              = "@{{pfx}}ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@{{pfx}}ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@{{pfx}}ocamlsdk//bin:ocamlruni",
)
