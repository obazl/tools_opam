load("@bazel_skylib//lib:paths.bzl", "paths")

load("@rules_ocaml//ocaml:providers.bzl",
     "OcamlProvider",
     "OcamlImportMarker")

load("@rules_ocaml//ppx:providers.bzl",
     "PpxCodepsProvider",
     "PpxExecutableMarker",
)

load("@rules_ocaml//ocaml/_rules:impl_common.bzl",
     "dsorder", "opam_lib_prefix")

load("@rules_ocaml//ocaml/_rules:impl_ccdeps.bzl", "dump_CcInfo")

load("@rules_ocaml//ocaml/_functions:deps.bzl",
     "aggregate_deps",
     "aggregate_codeps",
     "OCamlInfo",
     "DepsAggregator")

load("@rules_ocaml//ocaml/_debug:colors.bzl",
     "CCRED", "CCDER", "CCMAG", "CCRESET")

##################################################
######## RULE DECL:  OCAML_IMPORT  #########
##################################################
def _opam_import_impl(ctx):

    """Import OCaml resources."""

    debug                = False
    debug_cc             = False
    debug_deps           = False
    debug_jsoo           = False
    debug_primary_deps   = False
    debug_secondary_deps = False
    debug_ppx            = False
    debug_tc             = False

    if debug: print("opam_import: %s" % ctx.label)

    # WARNING: some pkgs have a "dummy" target with no attribs, e.g
    # seq, byte; but others have only archives, e.g. @ocaml//num/core
    # print("attr hasattr cmi? %s" % hasattr(ctx.attr, "cmi"))
    # print("file hasattr cmi? %s" % hasattr(ctx.file, "cmi"))
    # print("files hasattr cmi? %s" % hasattr(ctx.files, "cmi"))
    if (len(ctx.files.cmi) < 1):
        if not ctx.file.cma:
            if debug: print("Skipping dummy target: %s" % ctx.label)
            return [
                OcamlImportMarker()
            ]

    tc = ctx.toolchains["@rules_ocaml//toolchain/type:std"]

    ################################################################
                ####    PRIMARY DEPENDENCIES    ####
    ################################################################

    ## A note on ppx_codep dependencies - primary and secondary: ##

    # Ppx_codeps are resources for which a ppx module injects a
    # dependency when run as part of a ppx executable. So if this
    # import has a primary ppx_codep, we can infer that it must be a
    # ppx_module that injects the dependency. On the other hand, this
    # import may have an "ordinary" secondary dep (listed in 'deps')
    # that in turn has a primary ppx_codep. That would count as a
    # secondary ppx_codep. And so on, so we could have tertiary etc.
    # ppx_codeps.

    # Task: deliver PpxCodepsProvider merging all ppx_codeps for this
    # import, both primary (listed in ppx_codeps) and secondary
    # (found as a dependency listed in deps).

    # For example, @ppx_log//:ppx_log has ppx_codep
    # @ppx_sexp_conv//runtime-lib. But it also lists @ppx_log//kernel
    # as a primary dep, and @ppx_log//kernel in turn has its own
    # ppx_codep, @ppx_log//types. So in this case we would list both
    # ppx_codeps in the PpxCodepsProvider for @ppx_log//:ppx_log.

    # Only ppx modules and execs can have ppx_codeps, which will be
    # injected when the ppx executable runs.


    ################ PRIMARY OCAML DEPENDENCIES ################
    ## Primary deps: cm* attributes, which are label_lists.
    sigs_primary             = []
    structs_primary          = []
    ofiles_primary           = []  # .o files
    archives_primary         = []
    afiles_primary           = []  # .a files
    astructs_primary         = []
    cmts_primary             = []
    paths_primary            = []

    jsoo_runtimes_primary     = []
    ccinfos_primary             = []  ## file list

    sigs_primary.extend(ctx.files.cmi)

    ## we add both cma and cmxa to archives.
    ## one may be needed to link a ppx, and the other
    ## to build a file transformed by the ppx
    ## (since a transition may cause a different toolchain to be used
    ## in each situation).
    archives_primary.append(ctx.file.cma)
    archives_primary.append(ctx.file.cmxa)
    # if tc.target == "vm":
    #     # archives_primary.append(ctx.file.cma)
    #     if ctx.file.cma:
    #         astructs_primary.extend(ctx.files.cmo)
    #     else:
    #         structs_primary.extend(ctx.files.cmo)
    # else:
    # if tc.target != "vm":
    if hasattr(ctx.attr, "cmxa"):
        # archives_primary.append(ctx.file.cmxa)
        if ctx.file.cmxa:
            astructs_primary.extend(ctx.files.cmx)
        else:
            structs_primary.extend(ctx.files.cmx)

    ofiles_primary.extend(ctx.files.ofiles)
    afiles_primary.extend(ctx.files.afiles)

    cmts_primary.extend(ctx.files.cmti)
    cmts_primary.extend(ctx.files.cmt)

    if len(ctx.files.cmi) > 0:
        paths_primary.append(ctx.files.cmi[0].dirname)

    for dep in ctx.attr.cc_deps:
        ccinfos_primary.append(dep[CcInfo])

    if hasattr(ctx.attr, "jsoo_runtime"):
        if ctx.file.jsoo_runtime:
            jsoo_runtimes_primary.append(ctx.file.jsoo_runtime)

    if debug_primary_deps:
        print("PRIMARY DEPS for %s" % ctx.label)
        print("sigs_primary: %s" % sigs_primary)
        print("structs_primary: %s" % structs_primary)
        print("archives_primary: %s" % archives_primary)
        print("ofiles_primary: %s" % ofiles_primary)
        print("afiles_primary: %s" % afiles_primary)
        print("astructs_primary: %s" % astructs_primary)
        print("cmts_primary: %s" % cmts_primary)

        print("jsoo_runtimes_primary: %s" % jsoo_runtimes_primary)

        print("paths_primary: %s" % paths_primary)

    depsets = DepsAggregator(
        deps = OCamlInfo(
            sigs = [],
            structs = [],
            ofiles  = [],
            archives = [],
            afiles = [],
            astructs = [], # archived cmx structs, for linking
            cmts = [],
            paths  = [],
            jsoo_runtimes = [], # runtime.js files
        ),
        codeps = OCamlInfo(
            sigs = [],
            structs = [],
            ofiles = [],
            archives = [],
            afiles = [],
            astructs = [],
            cmts = [],
            paths = [],
            jsoo_runtimes = [],
        ),
        ccinfos = []
    )

    if debug_deps: print("ctx.attr.deps: %s" % ctx.attr.deps)
    for dep in ctx.attr.deps:
        depsets = aggregate_deps(ctx, dep, depsets)

    if debug_deps: print("ctx.attr.ppx_codeps: %s" % ctx.attr.ppx_codeps)
    if hasattr(ctx.attr, "ppx_codeps"):
        for codep in ctx.attr.ppx_codeps:
            depsets = aggregate_codeps(ctx, codep, depsets)

    ################################################################
    ######  IMPORT ACTION ######
    ################################################################
    # Null action - don't really need this but it marks the divide
    # between merging deps and creating providers.
    ctx.actions.do_nothing(
        mnemonic = "opam_import",
        inputs = depset()
    )

    ################################################################
    ##  PROVIDERS ##
    ################################################################
    providers = []

    ## we don't produce anything by action, so default is empty:
    providers.append(DefaultInfo())

    ##################
    ppxCodepsProvider = PpxCodepsProvider(
        sigs       = depset(order=dsorder,
                            transitive = depsets.codeps.sigs),
        structs    = depset(order=dsorder,
                            transitive = depsets.codeps.structs),
        ofiles     = depset(order=dsorder,
                            transitive = depsets.codeps.ofiles),
        archives   = depset(order=dsorder,
                            transitive = depsets.codeps.archives),
        afiles     = depset(order=dsorder,
                            transitive = depsets.codeps.afiles),
        astructs   = depset(order=dsorder,
                            transitive = depsets.codeps.astructs),
        cmts       = depset(order=dsorder,
                            transitive = depsets.codeps.cmts),
        paths       = depset(order=dsorder,
                             transitive = depsets.codeps.paths),
        jsoo_runtimes = depset(order=dsorder,
                               transitive = depsets.codeps.jsoo_runtimes),
    )
    providers.append(ppxCodepsProvider)

    #### Std OcamlProvider
    _ocamlProvider = OcamlProvider(
        struct = depset(direct = [ctx.file.cma]),
        sigs    = depset(order="postorder",
                         direct=sigs_primary,
                         transitive = depsets.deps.sigs),
        # transitive=sigs_secondary),
        structs = depset(order="postorder",
                         direct=structs_primary,
                         transitive = depsets.deps.structs),
        # transitive=structs_secondary),
        ofiles   = depset(order="postorder",
                          direct=ofiles_primary,
                          transitive = depsets.deps.ofiles),
        # transitive=ofiles_secondary),
        archives = depset(order="postorder",
                          direct=archives_primary,
                          transitive = depsets.deps.archives),
        # transitive=archives_secondary),
        afiles   = depset(order="postorder",
                          direct=afiles_primary,
                          transitive = depsets.deps.afiles),
        # transitive=afiles_secondary),
        astructs = depset(order="postorder",
                          direct=astructs_primary,
                          transitive = depsets.deps.astructs),
        # transitive=astructs_secondary),
        cmts     = depset(order="postorder",
                          direct=cmts_primary,
                          transitive = depsets.deps.cmts),
        # transitive=cmts_secondary),
        paths    = depset(order="postorder",
                          direct=paths_primary,
                          transitive = depsets.deps.paths),
        # transitive=paths_secondary),

        jsoo_runtimes = depset(order="postorder",
                               direct=jsoo_runtimes_primary,
                               transitive = depsets.deps.jsoo_runtimes),
        # transitive=jsoo_runtimes_secondary),
    )
    providers.append(_ocamlProvider)

    ccInfo = cc_common.merge_cc_infos(
        direct_cc_infos = ccinfos_primary,
        cc_infos = depsets.ccinfos
    )
    providers.append(ccInfo)

    providers.append(OcamlImportMarker(marker = "OcamlImport"))

    ## --output_groups only prints generated stuff, so there's no
    ## --point in providing OutputGroupInfo for opam_import

    if ctx.label.name == "ppx_expect":
        # print("import ccinfos depsets: %s" % depsets.ccinfos)
        if debug_cc: dump_CcInfo(ctx, ccInfo)
        if debug_jsoo:
            print("import jsoo: %s" % _ocamlProvider.jsoo_runtimes)
            if hasattr(ctx.attr, "ppx_codeps"):
                print("import codeps jsoo: %s" % ppxCodepsProvider.jsoo_runtimes)
        # fail("xxxxXXXXXXXXXXXXXXXX: %s" % )

    return providers

################################################################
opam_import = rule(
    implementation = _opam_import_impl,
    doc = """Imports pre-compiled OCaml files. [User Guide](../ug/opam_import.md).

    """,
    attrs = dict(
        # _mode       = attr.label(
        #     default = "@rules_ocaml//build/mode",
        # ),

        cma  = attr.label(allow_single_file = True),
        cmxa = attr.label(allow_single_file = True),
        cmxs = attr.label(allow_single_file = True),
        cmi  = attr.label_list(allow_files = True),
        cmo  = attr.label_list(allow_files = True),
        cmx  = attr.label_list(allow_files = True),
        ofiles =  attr.label_list(
            doc = "list of .o files that go with .cmx files",
            allow_files = True
        ),
        afiles =  attr.label_list(
            doc = "list of .a files that go with .cmxa files",
            allow_files = True
        ),
        cc_deps = attr.label_list(
            doc = "C archive files (.a) for integrating OCaml and C libs",
            allow_files = True,
            providers = [CcInfo],
        ),
        vmlibs   = attr.label_list(
            doc = "Dynamically-loadable, for ocamlrun. Standard naming is 'dll<name>_stubs.so' or 'dll<name>.so'.",
            allow_files = [".so"]
        ),
        cmt  = attr.label_list(allow_files = True),
        cmti = attr.label_list(allow_files = True),
        jsoo_runtime = attr.label(allow_single_file = True),
        srcs = attr.label_list(allow_files = True),

        all = attr.label_list(
            doc = "Glob all cm* files except for 'archive' or 'plugin' so theey can be added to action ldeps (rather than cmd line). I.e. the (transitive) deps of an archive, which must be accessible to the compiler (via search path, not command line), and so must be added to the action ldeps.",
            allow_files = True
        ),

        modules = attr.label_list(
            allow_files = True
        ),
        signature = attr.label_list(
            allow_files = True
        ),
        ppx = attr.label(
            doc = "precompiled ppx executable",
            allow_single_file = True,
            executable = True,
            cfg        = "exec"
        ),
        plugin = attr.label_list(
            allow_files = True
        ),
        # opam_import can only depend on other opam_imports
        deps = attr.label_list(
            allow_files = True,
            providers = [[OcamlImportMarker],[CcInfo]],
        ),
        ppx_codeps = attr.label_list(
            allow_files = True,
            providers = [[OcamlImportMarker]],
            # cfg = _ppx_codeps_transition,
        ),
        # _allowlist_function_transition = attr.label(
        #     default = "@bazel_tools//tools/allowlists/function_transition_allowlist"
        # ),

        version = attr.string(),
        opam_version = attr.string(),
        doc = attr.string(),
        _rule = attr.string( default = "opam_import" ),
    ),
    provides = [OcamlImportMarker],
    executable = False,
    toolchains = ["@rules_ocaml//toolchain/type:std"],
)
