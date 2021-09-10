# opam bootstrapping

This app, to be run by a Bazel repo rule, constructs an `@opam` repo
by crawling the current OPAM switch and generating `BUILD.bazel` files
(in `@opam//lib`) for all findlib `META` files. The `@opam` repo
contains soft links to the switch (in the local filesystem), and the
targets (`ocaml_import` rules) in the generated `BUILD.bazel` files
refer to resources via the soft links. So for example
`@opam//lib/yojson` will provide built files from
`~/.opam/<current_switch>/lib/yojson`.

## note on ppx

[META documentation](http://projects.camlcity.org/projects/dl/findlib-1.9.1/doc/ref-html/r759.html)

findlib is driven by "predicates", which as near as I can tell are
just command-line options, e.g. `-predicates ppx_driver`. Such
predicates are analogous to boolean flags, in that they are negated
with `-`, e.g. `-predicates -ppx_driver`. However, the LEM does not
seem to apply; absence of `ppx_driver` does not imply `-ppx_driver`.
Or in other words, by default they are undefined rather than true or
false. So we have use boolean flags to emulate these defined/undefined
flags.

**IMPORTANT** Do not confuse the `ppx_driver` findlib flag and the
[ppx_driver](https://github.com/janestreet-deprecated/ppx_driver)
library (now deprecated) from Janestreet. Unfortunately, the
`ppx_driver` flag ("predicate") seems to be completely undocumented.

Predicate flags select (or deselect) various things, mostly
dependencies. For example, `ppx_sexp_value/META`:

```
version = "v0.14.0"
description = ""
requires(ppx_driver) = "base ppx_here.expander ppx_sexp_conv.expander ppxlib"
archive(ppx_driver,byte) = "ppx_sexp_value.cma"
archive(ppx_driver,native) = "ppx_sexp_value.cmxa"
plugin(ppx_driver,byte) = "ppx_sexp_value.cma"
plugin(ppx_driver,native) = "ppx_sexp_value.cmxs"
# This is what dune uses to find out the runtime dependencies of
# a preprocessor
ppx_runtime_deps = "ppx_sexp_conv.runtime-lib"
# This line makes things transparent for people mixing preprocessors
# and normal dependencies
requires(-ppx_driver) = "ppx_here.runtime-lib ppx_sexp_conv.runtime-lib"
ppx(-ppx_driver,-custom_ppx) = "./ppx.exe --as-ppx"
library_kind = "ppx_rewriter"
```

If `-predicates ppx_driver` is passed, ocamlfind will add the value of
the `requires(ppx_driver)` key (which findlib calls a "variable") as
dependencies; if instead `-ppx_driver` is passed, the value of
`requires(-ppx_driver)` will be used.

Of course each dep in these deplists will have its own dependencies,
which `findlib` will gather. In this case, the transitive closure of
the `requires(ppx_driver)` deplist will include the deps listed in
`requires(-ppx_driver)`.

The OBazl opam bootstrapper (i.e. this package) generates a
`BUILD.bazel` for each `META` file in the current switch. OPAM
libraries (packages? resources?) thus become available as ordinary
dependency labels, e.g. `@opam//lib/ppx_sexp_value`. Currently, the
default target (e.g. `@opam//lib/ppx_sexp_value:ppx_sexp_value`) is
generated using the `ppx_driver` flag; the target corresponding to
`-ppx_driver` is named "no_ppx_driver".

### undocumented predicates

findlib `META` files may contain undocumented predicates; for example
`-custom_ppx`. Currently we take a stab at supporting these, probably
incorrectly. The working assumption is that these are specific to some
build tools (dune?) that use the `META` files, so they can be ignored
in OBazl. For example: `requires(-ppx_driver,-custom_ppx) +=
"ppx_deriving"`, meaning, apparently, that if these predicates are
negated, then add `ppx_deriving` to the deps list, and raising the
question of what to do if one is passed as positive, e.g. `-predicates
custom_ppx`.

The `-custom_ppx` flag is common, and seems to always be associated
with `-ppx_driver`, as in the example above. To support this we use
Bazel boolean flags on `config_settings`, so that the user can control
things at the command line, e.g. the equivalent of `-predicates
-custom_ppx` is `--no@opam/lib/ppx_sexp_conv:custom_ppx`.

The combinations we see in the `META` files:

* `requires(ppx_driver)`
* `requires(-ppx_driver)`
* `requires(-ppx_driver,-custom_ppx)`

which we support with:

```
load("@bazel_skylib//rules:common_settings.bzl", "bool_flag")
bool_flag( name = "ppx_driver", build_setting_default = True )
bool_flag( name = "custom_ppx", build_setting_default = True )

config_setting(name = "ppx_driver_only",
               flag_values = {":ppx_driver": "True",
                              ":custom_ppx": "True"})
config_setting(name = "no_ppx_driver_only",
               flag_values = {":ppx_driver": "False",
                              ":custom_ppx": "True"})
config_setting(name = "both_disabled",
               flag_values = { ":ppx_driver": "False",
                               ":custom_ppx": "False" })
```

We also see some ppx-related "variables" that are apparently
tool-specific (dune?), which we can (?) ignore:

* `ppxopt(-ppx_driver,-custom_ppx) = "ppx_deriving,package:ppx_sexp_conv"`
* `ppx(-ppx_driver,-custom_ppx) = "./ppx.exe --as-ppx"`

### deprecated predicates

All thread-related predicates: `mt_vm`, `mt_posix`. OBazl's support
for threads does not use any opam resources.

Similar for Dune stuff, which must be handled appropriately by
conversion routines. In general threading in OBazl is supported by
config flags like `--@ocaml//module/threads` rather than rule
arguments. I.e. there is no need to add a `deps` entry for the thread
library; it will be automatically added.

## Vendored

* [lemon](https://www.hwaci.com/sw/lemon/)
* [logc](https://github.com/rxi/log.c)
* [makeheaders](https://www.hwaci.com/sw/mkhdr/)
* [uthash](https://troydhanson.github.io/uthash/)
