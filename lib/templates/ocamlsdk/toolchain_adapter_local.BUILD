# generated file - DO NOT EDIT

exports_files(["BUILD.bazel"])

load("@rules_ocaml//toolchain:BUILD.bzl", "ocaml_toolchain_adapter")

########################
ocaml_toolchain_adapter(
    name                   = "ocamlopt.opt",
    host                   = "sys",
    target                 = "sys",
    repl                   = "@ocamlsdk//bin:ocaml",
    compiler               = "@ocamlsdk//bin:ocamlopt.opt",
    version              = "//version",
    profiling_compiler   = "@ocamlsdk//bin:ocamloptp",
    ocamllex             = "@ocamlsdk//bin:ocamllex.opt",
    ocamlyacc            = "@ocamlsdk//bin:ocamlyacc",
    linkmode             = "dynamic",
    # stdlib             = "@ocamlsdk//runtime:stdlib.cma",
    # std_exit           = "@ocamlsdk//runtime:std_exit.cmo",

    default_runtime      = "@ocamlsdk//runtime:libasmrun.a",
    std_runtime          = "@ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@ocamlsdk//runtime:libasmrun_shared.so",
    vmlibs               = "@stublibs//lib/stublibs",
    vmlibs_path          = "@stublibs//lib/stublibs:path",

    ## omit vm stuff for >sys toolchains?
    # vmruntime            = "@ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@ocamlsdk//bin:ocamlruni",
)

ocaml_toolchain_adapter(
    ## debug syssys tc
    name                   = "ocamlopt.opt.d",
    host                   = "sys",
    target                 = "sys",
    repl                   = "@ocamlsdk//bin:ocaml",
    compiler               = "@ocamlsdk//bin:ocamlopt.opt",
    version                = "//version",
    profiling_compiler     = "@ocamlsdk//bin:ocamloptp",
    ocamllex               = "@ocamlsdk//bin:ocamllex.opt",
    ocamlyacc              = "@ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@ocamlsdk//runtime:libasmrund.a",
    std_runtime          = "@ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@ocamlsdk//runtime:libasmrun_shared.so",
    vmlibs               = "@stublibs//lib/stublibs",
    vmlibs_path          = "@stublibs//lib/stublibs:path",

    ## omit vm stuff for >sys toolchains?
    # vmruntime              = "@ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlc.opt",
    host                   = "sys",
    target                 = "vm",
    repl                   = "@ocamlsdk//bin:ocaml",
    compiler               = "@ocamlsdk//bin:ocamlc.opt",
    version                = "//version",
    profiling_compiler     = "@ocamlsdk//bin:ocamlcp",
    ocamllex               = "@ocamlsdk//bin:ocamllex.opt",
    ocamlyacc              = "@ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@ocamlsdk//runtime:libcamlrun.a",
    std_runtime          = "@ocamlsdk//runtime:libcamlrun.a",
    dbg_runtime          = "@ocamlsdk//runtime:libcamlrund.a",
    instrumented_runtime = "@ocamlsdk//runtime:libcamlruni.a",
    pic_runtime          = "@ocamlsdk//runtime:libcamlrun_pic.a",
    shared_runtime       = "@ocamlsdk//runtime:libcamlrun_shared.so",
    vmlibs               = "@stublibs//lib/stublibs",
    vmlibs_path          = "@stublibs//lib/stublibs:path",

    # vmruntime              = "@ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlc.byte",
    host                   = "vm",
    target                 = "vm",
    repl                   = "@ocamlsdk//bin:ocaml",
    compiler               = "@ocamlsdk//bin:ocamlc.byte",
    version                = "//version",
    profiling_compiler     = "@ocamlsdk//bin:ocamlcp",
    ocamllex               = "@ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@ocamlsdk//runtime:libcamlrun.a",
    std_runtime          = "@ocamlsdk//runtime:libcamlrun.a",
    dbg_runtime          = "@ocamlsdk//runtime:libcamlrund.a",
    instrumented_runtime = "@ocamlsdk//runtime:libcamlruni.a",
    pic_runtime          = "@ocamlsdk//runtime:libcamlrun_pic.a",
    shared_runtime       = "@ocamlsdk//runtime:libcamlrun_shared.so",
    vmlibs               = "@stublibs//lib/stublibs",
    vmlibs_path          = "@stublibs//lib/stublibs:path",

    # vmruntime              = "@ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlopt.byte",
    host                   = "vm",
    target                 = "sys",
    repl                   = "@ocamlsdk//bin:ocaml",
    compiler               = "@ocamlsdk//bin:ocamlopt.byte",
    version                = "//version",
    profiling_compiler     = "@ocamlsdk//bin:ocamloptp",
    ocamllex               = "@ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@ocamlsdk//runtime:libasmrun.a",
    std_runtime          = "@ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@ocamlsdk//runtime:libasmrun_shared.so",
    vmlibs               = "@stublibs//lib/stublibs",
    vmlibs_path          = "@stublibs//lib/stublibs:path",

    # vmruntime              = "@ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@ocamlsdk//bin:ocamlruni",
)

ocaml_toolchain_adapter(
    name                   = "ocamlopt.byte.d",
    host                   = "vm",
    target                 = "sys",
    repl                   = "@ocamlsdk//bin:ocaml",
    compiler               = "@ocamlsdk//bin:ocamlopt.byte",
    version                = "//version",
    profiling_compiler     = "@ocamlsdk//bin:ocamloptp",
    ocamllex               = "@ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@ocamlsdk//runtime:libasmrund.a",
    std_runtime          = "@ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@ocamlsdk//runtime:libasmrun_shared.so",
    vmlibs               = "@stublibs//lib/stublibs",
    vmlibs_path          = "@stublibs//lib/stublibs:path",

    # vmruntime              = "@ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@ocamlsdk//bin:ocamlruni",
)
