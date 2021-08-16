# workspaces

Best practice: always name the workspace: workspace(name="foo")

This makes load statements work when the repo is used as an external
repo by a client.  Use 'load("@foo//bar:vars.bzl")' instead of
'load("//bar:vars.bzl")'.  This will work both locally and as an
external.  Question: what if the http_archive rule uses a different
name?  Is there a requirement that the name on the http_archive rule
must match the workspace name?

For examples see:
* libsnark/gadgetlib1/gadgets/verifiers/BUILD.bazel
* libsnark/gadgetlib1/gadgets/cpu_checkers/fooram/examples/BUILD.bazel

This also allows us to use the repo in e.g. deps:

    deps = [
        "@libsnark//libsnark/common",
        "@libsnark//libsnark/gadgetlib1/gadgets/hashes",
        "@libff//libff"
    ]

This too will work both locally and as a remote, so long as the
external repo rule uses the same name as the workspace. Whether or not
it is a good idea, I dunno.  I guess it would force the client to use
the correct repo name, since it would not build if it does not match
the ws name.

## transitive repos

Example: ate-pairing

# ate-pairing already includes libgmp, but Bazel does not yet support
# transitive external deps, so we need to repeat the dep here.
# Specifically, ate-pairing build rules refer to @libgmp, which bazel
# resolves to the root repo, which is determined by this WORKSPACE
# file, rather than the one in the ate-pairing repo. So we need to
# copy ate-pairing's repo rule into this file.  The build logic will
# come from ate-pairing; we just need to make the repo available here,
# in the root repo.  Build rules for this repo can then refer to
# libgmp using '@libgmp//:libgmp' (here) or '@ate-pairing//:libgmp' (a
# target that indirectly refers to the repo defined here).  We use the
# latter, in case Bazel ever gets around to supporting transitive
# workspaces.
