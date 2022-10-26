load("@bazel_skylib//lib:unittest.bzl", "asserts", "analysistest")

load(":opam_import.bzl", "opam_import")

load("@rules_ocaml//ocaml:providers.bzl",
     "OcamlProvider",
     "OcamlImportMarker")

load("@rules_ocaml//ppx:providers.bzl",
     "PpxCodepsProvider",
     "PpxExecutableMarker",
)

load("@rules_ocaml//ocaml/_rules:impl_common.bzl",
     "dsorder", "opam_lib_prefix")

load("@rules_ocaml//ocaml/_rules:impl_ccdeps.bzl",
     "cc_shared_lib_to_ccinfo",
     "filter_ccinfo",
     "extract_cclibs", "dump_CcInfo",
     "ccinfo_to_string"
     )

load("@rules_ocaml//ocaml/_debug:colors.bzl",
     "CCRED", "CCDER", "CCMAG", "CCRESET")

# "The basic principle is to define a testing rule that DEPENDS ON THE
# RULE-UNDER-TEST. This gives the testing rule access to the
# rule-under-test's providers."

# Actually it looks like the testing rule depends on the TARGET under
# test.

# ==== Check the provider contents ====

def _provider_contents_test_impl(ctx):
    # tc = ctx.toolchains["@rules_ocaml//toolchain/type:std"]

    env = analysistest.begin(ctx)

    target_under_test = analysistest.target_under_test(env)

    [
        static_cc_deps, dynamic_cc_deps
    ] = extract_cclibs(ctx,
                       None, ## tc.linkmode,
                       None, ## args
                       target_under_test[CcInfo])
    print("test: static cc: %s" % static_cc_deps)
    static_names = [cc.basename for cc in static_cc_deps]

    asserts.true(env, "libbase_stubs.a" in static_names)
    asserts.true(env, "libbase_internalhash_types_stubs.a"
                 in static_names)

    jsoo = []
    for f in target_under_test[OcamlProvider].jsoo_runtimes.to_list():
        jsoo.append(f.path)

    asserts.true(env, "external/opam_base/lib/base/runtime.js" in jsoo)

    return analysistest.end(env)

# Create the testing rule to wrap the test logic.
provider_contents_test = analysistest.make(_provider_contents_test_impl)

################################################################
def _test_provider_contents():
    opam_import(
        name = "provider_contents_subject",
        tags = ["manual"],
        version = """v0.15.0""",
        cma    = "base.cma",
        cmxa   = "base.cmxa",
        # cmi      = glob(["*.cmi"]),
        # cmo      = glob(["*.cmo"]),
        # cmx      = glob(["*.cmx"]),
        # ofiles   = glob(["*.o"]),
        # afiles   = glob(["*.a"], exclude=["*_stubs.a"]),
        # cmt      = glob(["*.cmt"]),
        # cmti     = glob(["*.cmti"]),
        # vmlibs   = glob(["dll*.so"]),
        # srcs     = glob(["*.ml", "*.mli"]),
        jsoo_runtime = "runtime.js",
        cc_deps   = [":_libbase_stubs.a"],
        deps = [
            "@opam_base//lib/base_internalhash_types",
            "@opam_base//lib/caml",
            "@opam_base//lib/shadow_stdlib",
            "@opam_sexplib0//lib/sexplib0",
        ],
    )

    # Testing rule.
    provider_contents_test(name = "provider_contents_test",
                           target_under_test = ":provider_contents_subject")
    # Note the target_under_test attribute is how the test rule depends on
    # the real rule target.

# Entry point from the BUILD file; macro for running each test case's macro and
# declaring a test suite that wraps them together.
def opam_import_test_suite(name):
    # Call all test functions and wrap their targets in a suite.
    _test_provider_contents()
    # ...

    native.test_suite(
        name = name,
        tests = [
            ":provider_contents_test",
            # ...
        ],
    )

################################################################
# put this in @opam_base//lib/base/BUILD.bazel:

# load("@opam//build:rules.bzl", "opam_import", "opam_import_test_suite")
# opam_import_test_suite(name = "base_opam_import_test")
