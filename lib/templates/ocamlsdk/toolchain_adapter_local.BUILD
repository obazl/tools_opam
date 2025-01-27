# generated file - DO NOT EDIT

exports_files(["BUILD.bazel"])

load("@rules_ocaml//toolchain:BUILD.bzl", "ocaml_toolchain_adapter")

########################
ocaml_toolchain_adapter(
    name                   = "ocamlopt.opt",
    host                   = "sys",
    target                 = "sys",
    repl                   = "@opam.ocamlsdk//bin:ocaml",
    compiler               = "@opam.ocamlsdk//bin:ocamlopt.opt",
    version              = "//version",
    profiling_compiler   = "@opam.ocamlsdk//bin:ocamloptp",
    ocamllex             = "@opam.ocamlsdk//bin:ocamllex.opt",
    ocamlyacc            = "@opam.ocamlsdk//bin:ocamlyacc",
    linkmode             = "dynamic",
    # stdlib             = "@opam.ocamlsdk//runtime:stdlib.cma",
    # std_exit           = "@opam.ocamlsdk//runtime:std_exit.cmo",

    default_runtime      = "@opam.ocamlsdk//runtime:libasmrun.a",
    std_runtime          = "@opam.ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@opam.ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@opam.ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@opam.ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@opam.ocamlsdk//runtime:libasmrun_shared.so",
    vmlibs               = "@opam.stublibs//lib/stublibs",
    vmlibs_path          = "@opam.stublibs//lib/stublibs:path",

    ## omit vm stuff for >sys toolchains?
    # vmruntime            = "@opam.ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@opam.ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@opam.ocamlsdk//bin:ocamlruni",
)

ocaml_toolchain_adapter(
    ## debug syssys tc
    name                   = "ocamlopt.opt.d",
    host                   = "sys",
    target                 = "sys",
    repl                   = "@opam.ocamlsdk//bin:ocaml",
    compiler               = "@opam.ocamlsdk//bin:ocamlopt.opt",
    version                = "//version",
    profiling_compiler     = "@opam.ocamlsdk//bin:ocamloptp",
    ocamllex               = "@opam.ocamlsdk//bin:ocamllex.opt",
    ocamlyacc              = "@opam.ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@opam.ocamlsdk//runtime:libasmrund.a",
    std_runtime          = "@opam.ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@opam.ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@opam.ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@opam.ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@opam.ocamlsdk//runtime:libasmrun_shared.so",
    vmlibs               = "@opam.stublibs//lib/stublibs",
    vmlibs_path          = "@opam.stublibs//lib/stublibs:path",

    ## omit vm stuff for >sys toolchains?
    # vmruntime              = "@opam.ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@opam.ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@opam.ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlc.opt",
    host                   = "sys",
    target                 = "vm",
    repl                   = "@opam.ocamlsdk//bin:ocaml",
    compiler               = "@opam.ocamlsdk//bin:ocamlc.opt",
    version                = "//version",
    profiling_compiler     = "@opam.ocamlsdk//bin:ocamlcp",
    ocamllex               = "@opam.ocamlsdk//bin:ocamllex.opt",
    ocamlyacc              = "@opam.ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@opam.ocamlsdk//runtime:libcamlrun.a",
    std_runtime          = "@opam.ocamlsdk//runtime:libcamlrun.a",
    dbg_runtime          = "@opam.ocamlsdk//runtime:libcamlrund.a",
    instrumented_runtime = "@opam.ocamlsdk//runtime:libcamlruni.a",
    pic_runtime          = "@opam.ocamlsdk//runtime:libcamlrun_pic.a",
    shared_runtime       = "@opam.ocamlsdk//runtime:libcamlrun_shared.so",
    vmlibs               = "@opam.stublibs//lib/stublibs",
    vmlibs_path          = "@opam.stublibs//lib/stublibs:path",

    # vmruntime              = "@opam.ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@opam.ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@opam.ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlc.byte",
    host                   = "vm",
    target                 = "vm",
    repl                   = "@opam.ocamlsdk//bin:ocaml",
    compiler               = "@opam.ocamlsdk//bin:ocamlc.byte",
    version                = "//version",
    profiling_compiler     = "@opam.ocamlsdk//bin:ocamlcp",
    ocamllex               = "@opam.ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@opam.ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@opam.ocamlsdk//runtime:libcamlrun.a",
    std_runtime          = "@opam.ocamlsdk//runtime:libcamlrun.a",
    dbg_runtime          = "@opam.ocamlsdk//runtime:libcamlrund.a",
    instrumented_runtime = "@opam.ocamlsdk//runtime:libcamlruni.a",
    pic_runtime          = "@opam.ocamlsdk//runtime:libcamlrun_pic.a",
    shared_runtime       = "@opam.ocamlsdk//runtime:libcamlrun_shared.so",
    vmlibs               = "@opam.stublibs//lib/stublibs",
    vmlibs_path          = "@opam.stublibs//lib/stublibs:path",

    # vmruntime              = "@opam.ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@opam.ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@opam.ocamlsdk//bin:ocamlruni",
)

########################
ocaml_toolchain_adapter(
    name                   = "ocamlopt.byte",
    host                   = "vm",
    target                 = "sys",
    repl                   = "@opam.ocamlsdk//bin:ocaml",
    compiler               = "@opam.ocamlsdk//bin:ocamlopt.byte",
    version                = "//version",
    profiling_compiler     = "@opam.ocamlsdk//bin:ocamloptp",
    ocamllex               = "@opam.ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@opam.ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@opam.ocamlsdk//runtime:libasmrun.a",
    std_runtime          = "@opam.ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@opam.ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@opam.ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@opam.ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@opam.ocamlsdk//runtime:libasmrun_shared.so",
    vmlibs               = "@opam.stublibs//lib/stublibs",
    vmlibs_path          = "@opam.stublibs//lib/stublibs:path",

    # vmruntime              = "@opam.ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@opam.ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@opam.ocamlsdk//bin:ocamlruni",
)

ocaml_toolchain_adapter(
    name                   = "ocamlopt.byte.d",
    host                   = "vm",
    target                 = "sys",
    repl                   = "@opam.ocamlsdk//bin:ocaml",
    compiler               = "@opam.ocamlsdk//bin:ocamlopt.byte",
    version                = "//version",
    profiling_compiler     = "@opam.ocamlsdk//bin:ocamloptp",
    ocamllex               = "@opam.ocamlsdk//bin:ocamllex.byte",
    ocamlyacc              = "@opam.ocamlsdk//bin:ocamlyacc",
    linkmode               = "dynamic",

    default_runtime      = "@opam.ocamlsdk//runtime:libasmrund.a",
    std_runtime          = "@opam.ocamlsdk//runtime:libasmrun.a",
    dbg_runtime          = "@opam.ocamlsdk//runtime:libasmrund.a",
    instrumented_runtime = "@opam.ocamlsdk//runtime:libasmruni.a",
    pic_runtime          = "@opam.ocamlsdk//runtime:libasmrun_pic.a",
    shared_runtime       = "@opam.ocamlsdk//runtime:libasmrun_shared.so",
    vmlibs               = "@opam.stublibs//lib/stublibs",
    vmlibs_path          = "@opam.stublibs//lib/stublibs:path",

    # vmruntime              = "@opam.ocamlsdk//bin:ocamlrun",
    # vmruntime_debug        = "@opam.ocamlsdk//bin:ocamlrund",
    # vmruntime_instrumented = "@opam.ocamlsdk//bin:ocamlruni",
)
