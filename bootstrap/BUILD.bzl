load("@bazel_skylib//lib:paths.bzl", "paths")

CMD_FLAGS = [
    "-std=c11",
    "-pedantic-errors",
]

BOOTSTRAP_INCLUDES = [
    "-Ibootstrap",
    "-Iexternal/opam/bootstrap",

    "-I$(GENDIR)/bootstrap",
    "-I$(GENDIR)/external/opam/bootstrap",
]

def _lemon_impl(ctx):

    (bn, ext) = paths.split_extension(ctx.file.yy.basename)

    out_c = ctx.actions.declare_file(bn + ".c")
    out_out = ctx.actions.declare_file(bn + ".out")

    exe = ctx.file._tool.path

    if (len(ctx.attr.defines) > 0):
        defs = "-D" + " -D".join(ctx.attr.defines)
    else:
        defs = ""

    if not ctx.attr.compress:
        compress = "-c"
    else:
        compress = ""

    cmd = "{lemon} -m {yy} {compress} {defines} -T{template} -d{outdir}".format(
        lemon=exe, yy=ctx.file.yy.path,
        compress=compress,
        defines=defs,
        template=ctx.file.template.path,
        outdir=out_c.dirname
    )

    ctx.actions.run_shell(
        inputs = [ctx.file.yy, ctx.file.template],
        outputs = ctx.outputs.outs,
        tools = [ctx.file._tool],
        command = cmd
    )

    return [DefaultInfo(files = depset(ctx.outputs.outs))]

lemon = rule(
    implementation = _lemon_impl,
    attrs = {
        "yy": attr.label(
            allow_single_file = True,
            default = "syntaxis.y"
        ),
        "outs": attr.output_list( ),
        "compress": attr.bool(
            doc = "False: pass -c, do not compress action tables",
            default = True
        ),
        "defines": attr.string_list(
        ),
        "template": attr.label(
            allow_single_file =  True,
            default = "//bootstrap:lempar.c"
        ),
        "_tool": attr.label(
            allow_single_file = True,
            default = "//bootstrap:lemon",
            executable = True,
            cfg = "host"
        )
    }
)

################################################################
debug_defs = select({
    "//:debug-ast": ["DEBUG_AST"],
    "//conditions:default":   []
}) + select({
    "//:debug-bindings": ["DEBUG_BINDINGS"],
    "//conditions:default":   []
}) + select({
    "//:debug-ctors": ["DEBUG_CTORS"],
    "//conditions:default":   []
}) + select({
    "//:debug-filters": ["DEBUG_FILTERS"],
    "//conditions:default":   []
}) + select({
    "//:debug-format": ["DEBUG_FORMAT"],
    "//conditions:default":   []
}) + select({
    "//:debug-load": ["DEBUG_LOAD"],
    "//conditions:default":   []
}) + select({
    "//:debug-loads": ["DEBUG_LOADS"],
    "//conditions:default":   []
}) + select({
    "//:debug-mem": ["DEBUG_MEM"],
    "//conditions:default":   []
}) + select({
    "//:debug-mutators": ["DEBUG_MUTATORS"],
    "//conditions:default":   []
}) + select({
    "//:debug-paths": ["DEBUG_PATHS"],
    "//conditions:default":   []
}) + select({
    "//:debug-preds": ["DEBUG_PREDICATES"],
    "//conditions:default":   []
}) + select({
    "//:debug-properties": ["DEBUG_PROPERTIES"],
    "//conditions:default":   []
}) + select({
    "//:debug-queries": ["DEBUG_QUERY"],
    "//conditions:default":   []
}) + select({
    "//:debug-s7-api": ["DEBUG_S7_API"],
    "//conditions:default":   []
}) + select({
    "//:debug-set": ["DEBUG_SET"],
    "//conditions:default":   []
}) + select({
    "//:debug-serializers": ["DEBUG_SERIALIZERS"],
    "//conditions:default":   []
}) + select({
    "//:debug-targets": ["DEBUG_TARGETS"],
    "//conditions:default":   []
}) + select({
    "//:debug-trace": ["DEBUG_TRACE"],
    "//conditions:default":   []
}) + select({
    "//:debug-utarrays": ["DEBUG_UTARRAYS"],
    "//conditions:default":   []
}) + select({
    "//:debug-vectors": ["DEBUG_VECTORS"],
    "//conditions:default":   []
}) + select({
    "//:debug-yytrace": ["DEBUG_YYTRACE"],
    "//conditions:default":   []
})

