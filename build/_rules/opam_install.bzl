load("@bazel_skylib//lib:paths.bzl", "paths")

load("@rules_ocaml//build:providers.bzl",
     "OcamlArchiveMarker",
     "OCamlDepsProvider",
     "OcamlExecutableMarker",
     "OCamlImportProvider",
     "OCamlLibraryProvider",
     "OCamlNsResolverProvider",
     "OCamlModuleProvider",
     "OcamlNsMarker",
     "OcamlNsSubmoduleMarker",
     "OCamlSignatureProvider",
)
# load("//build:providers.bzl",
#      "PpxExecutableMarker",
# )
################################################
def _in_transition_impl(settings, attr):

    if attr.tc == "ocamlopt":
        host = "@rules_ocaml//platform:ocamlopt.opt"
        tgt  = "@rules_ocaml//platform:sys>any"
    elif attr.tc == "ocamlc":
        host = "@rules_ocaml//platform:ocamlc.opt"
        tgt  = "@rules_ocaml//platform:vm>any"
    else:
        fail("Illegal tc: {}; options are ocamlopt, ocamlc".format(attr.tc))

    return {
        "@rules_ocaml//toolchain": attr.tc,
        "//command_line_option:host_platform": host,
        "//command_line_option:platforms": tgt
    }

################
_in_transition = transition(
    implementation = _in_transition_impl,
    inputs = [
        "@rules_ocaml//toolchain",
        "//command_line_option:host_platform",
        "//command_line_option:platforms",
    ],
    outputs = [
        "@rules_ocaml//toolchain",
        "//command_line_option:host_platform",
        "//command_line_option:platforms",
    ]
)

###############
def _libs(ctx):
    libs = set()
    for dep in ctx.attr.lib:
        provider = dep[OCamlDepsProvider]
        for arch in provider.archives.to_list():
            libs.add('"{}"'.format(arch.path))
        for sig in provider.sigs.to_list():
            libs.add('"{}"'.format(sig.path))
        for xstruct in provider.structs.to_list():
            libs.add('"{}"'.format(xstruct.path))
        for astruct in provider.astructs.to_list():
            libs.add('"{}"'.format(astruct.path))
        for afile in provider.afiles.to_list():
            libs.add('"{}"'.format(afile.path))
        # print("dep: %s" % dep)
        # print("provider: %s" % provider)
        for src in provider.srcs.to_list():
            libs.add('"{}"'.format(src.path))
        # print("OFILES: %s" % provider.ofiles)
        # print("CMTS: %s" % provider.cmts)
        # print("CMTIS: %s" % provider.cmtis)

    libs = sorted(libs)
    text = ""
    for lib in libs:
        text = text + "  " + lib + "\n"
    return text

##################
def _sublibs(ctx):
    sublibs = dict()
    for dep, sublib in ctx.attr.sublibs.items():
        pkg = dep.label.package
        sublibs[sublib] = []
        provider = dep[OCamlDepsProvider]
        for item in provider.archives.to_list():
            if item.short_path.startswith(pkg):
                sublibs[sublib].append('"{}"'.format(item.path))
        for item in provider.sigs.to_list():
            if item.short_path.startswith(pkg):
                sublibs[sublib].append('"{}"'.format(item.path))
        for item in provider.structs.to_list():
            if item.short_path.startswith(pkg):
                sublibs[sublib].append('"{}"'.format(item.path))
        for item in provider.astructs.to_list():
            if item.short_path.startswith(pkg):
                sublibs[sublib].append('"{}"'.format(item.path))
        for item in provider.afiles.to_list():
            if item.short_path.startswith(pkg):
                sublibs[sublib].append('"{}"'.format(item.path))
        for item in provider.srcs.to_list():
            if item.short_path.startswith(pkg):
                sublibs[sublib].append('"{}"'.format(item.path))
        for item in provider.cmts.to_list():
            if item.short_path.startswith(pkg):
                sublibs[sublib].append('"{}"'.format(item.path))
        for item in provider.cmtis.to_list():
            if item.short_path.startswith(pkg):
                sublibs[sublib].append('"{}"'.format(item.path))
        # print("OFILES: %s" % provider.ofiles)

    # fail("SUBLIBS %s" % sublibs)

    text = ""
    for (pkg, files) in sublibs.items():
        # print("\n{}: {}\n".format(pkg, files))
        for f in files:
            text = text + "  " + f + " {{ \"{}/{} }}\n".format(
                pkg, paths.basename(f))
    return text

############################
def _opam_install_impl(ctx):

    # print("opam_install rule: %s" % ctx.label)
    tc = ctx.toolchains["@rules_ocaml//toolchain/type:std"]

    lib_files = _libs(ctx)
    # print("LIBS:\n%s" % text)
    sublib_files = _sublibs(ctx)

    text = "\n" + lib_files + sublib_files

    ctx.actions.run_shell(
        inputs = ctx.files.lib + ctx.files.sublibs,
        outputs = [ctx.outputs.out],
        progress_message = "Generating %s" % ctx.outputs.out.short_path,
        command = "echo '{}' > '{}'".format(
            text, ctx.outputs.out.path)
    )

    return DefaultInfo(files = depset([ctx.outputs.out]))

##########################
opam_install = rule(
    implementation = _opam_install_impl,
    doc = """Rule for installing to OPAM.""",
    executable = False,
    attrs = dict(
        out = attr.output(
            mandatory = True
        ),
        tc  = attr.string(
            values = ["ocamlopt", "ocamlc"],
            default = "ocamlopt"
        ),
        pkg = attr.string(mandatory=True),
        bin = attr.label_list(
            providers = [OCamlDepsProvider]
        ),
        lib = attr.label_list(
            providers = [OCamlDepsProvider]
        ),
        sublibs = attr.label_keyed_string_dict(
            providers = [OCamlDepsProvider]
        ),
    ),
    cfg = _in_transition,
    toolchains = ["@rules_ocaml//toolchain/type:std"],
)
