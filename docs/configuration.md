# configuration


## general

Boolean flags: follow autoconf convention:  enable_FEATURE, with_PKG.  E.g.

* enable_debug
* with_libgmp

Parameterized flags: foo=bar

## transitive and intransitive configurations

Sometimes you want to be able to set different configurations for
different deps, e.g. -DEBUG.  In other cases, you want the same setting
to be propagated to deps. An example of this is libff and ate-pairing,
where we want to set e.g. CURVE=BN128 for the former, and have it
automatically set for the latter.  We can of course set each
independently, but it is clearly preferable to support _transitive
configurations_.

The trick is to use absolute anonymous-ws labels (e.g. "@//foo:bar") for
transitive configuration, and absolute named-ws labels
(e.g. "@foo//bar:baz") for intransitive configuration.

[Terminology: ":foo" and "foo/bar:baz" are a relative labels (relative
to the 'current package'); "//foo/bar:baz" is a "fully-qualified
relative" label (relative to the local workspace root).
"@//foo/bar:baz" is an absolute anonymous-ws label, and
"@x//foo/bar:baz" is an absolute named-ws label.  Absolute named-ws
labels are also local.  Absolute anonymous-ws labels are always
resolved to the current project root workspace, even if they are in
imported repos (workspaces).

A reference to "@//bzl/config:foo" will always be resolved to the
main/current/root/project workspace rather than an external workspace.
By contrast, a reference to "@x//bzl/config:foo" will be resolved to
the @x namespace, which could be the (named) root/project workspace,
or it could be an external workspace (i.e. external/foo).  So if the
importing workspace defines a flg //bzl/config:foo, it will be picked
up by any imported deps that refer to @//bzl/config:foo.

An imported dep named x that refers to @x//bzl/config:debug will
always pick up that value from its own //bzl/config/BUILD.bazel file,
not from the one in the importing workspace.

To make this work, the vars.bzl file, which uses select on config
settings to set global vars, should always be loaded using its
absolute local label, e.g. load("@x//bzl:vars.bzl"). That ensures that
the repo will load its own vars.bzl file, and not the one in the
importing workspace.

It's also critical that each workspace be explicitly named, e.g. put
'workspace(name = "libfoo")' first in the WORKSPACE file.  That is
what ensures that absolute named-ws refs will work in stand-alone mode.
