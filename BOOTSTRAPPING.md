# opam bootstrapping

This app, to be run by a Bazel repo rule, constructs an `@opam` repo
by crawling the current OPAM switch and generating `BUILD.bazel` files
(in `@opam//lib`) for all findlib `META` files. The `@opam` repo
contains soft links to the switch (in the local filesystem), and the
targets (`ocaml_import` rules) in the generated `BUILD.bazel` files
refer to resources via the soft links. So for example
`@opam//lib/yojson` will provide built files from
`~/.opam/<current_switch>/lib/yojson`.

## building and running

The bootstrapper is designed to be built and run by the
`opam_configuration` repository rule defined in
`opam/_bootstrap/opam.bzl`. To build it, the rule executes `build.sh`;
to run it, it executes `opam_bootstrap` with no arguments. This will
happen when you run `bazel build` in a project that depends on this
repo.

### testing

For testing, you can build it using the `Makefile` and run it directly
on the command line: `$ ./opam_bootstrap`. Edit the makefile to export
the desired debug macro definitions and then run `make`. Note that
this command contains no dependency rules; it just sets some environment
variables and runs `build.sh`. So it will always rebuild everything.
(We do this so that the production build will depend only on `sh`, not
`make`.) Output will be written to `$(pwd)/tmp/opam`. `make clean`
will delete the output as well as any generated files.

**WARNING** The repository rule writes some files that will not be
written when you build using the makefile and run `opam_bootstrap`
directly at the command line. But that should not be a problem,
because the point of running it directly is to check the BUILD.bazel
outputs, not to use/evaluate them.

## packages and paths

`findlib` packages are analogous to Bazel packages; they are
determined by a `META` file, just as Bazel packages are determined by
a `BUILD.bazel` file. The format is documented at
[META](http://projects.camlcity.org/projects/dl/findlib-1.9.1/doc/ref-html/r759.html).

The analogy ends there, however:

* the package file may be named `META.<pkg>`, in which case the name
  of the packages is the `<pkg>` suffix. By default it is the name of
  the containing directory, just as for Bazel. In practice this does
  not seem to be the case in OPAM; all packages I've encountered use a
  `META` file.
* The `META` file may define subpackages.
* The resources (archive files, plugins, etc.) delivered by a package
  may be located in a different directory; this is controlled by the
  'directory' property. See below for more information.
* Packages have "properties" analogous to Bazel attributes. Note the
  "properties" is our term; the `META` documentation calls them
  "variables".
* Properties may be parameterized by "predicates"; these correspond to
  command-line options and are used to select which properties are
  effective.

### paths

Below we use `OPAM_SWITCH_PREFIX` to refer to the path of the current
OPAM switch; it will be something like e.g. `~/.opam/4.10` for a
switch named `4.10`. Run `opam switch` to see which switch is
"current".

The bootstrapper runs the following shell commands to discover information about the current OPAM switch:

* `opam var switch` - returns the switch name, e.g. '4.11.2'

* `opam var bin` - returns the path of `bin` directory of the current switch, e.g. `~/.opam/4.11.2/bin`

* `opam var lib` - returns the path of `lib` directory of the current switch, e.g. `~/.opam/4.11.2/lib`

For each `findlib` package we have:

* the `findlib` package name, which is formed by concatenating
  subpackage names using dotted syntax; e.g. `ptime.clock.os` is
  defined in `$(OPAM_SWITCH_PREFIX)/lib/ptime/META`.
* a corresponding `bzlpkg_path`, which is the directory in which we
  will write a `BUILD.bazel` file. For top-level findlib packages this
  is the findlib package name relative to `$OPAM_SWITCH_PREFIX`. For
  subpackages, it is constructed by concatenating subpackage names
  after the top-level package name using `/` instead of `.`. For
  example, the `bzlpkg_path` for findlib package `ptime.clock.os` is
  `ptime/clock/os`. For this we will generate `@opam//ptime/clock/os`,
  which will map to `$(OPAM_BZL)/lib/ptime/clock/os/BUILD.bazel`,
  where `$(OPAM_BZL)` is `external/opam` (within Bazel's work area).
* a `rsrc_path`, which is the directory containing the resources
  (archive files etc.) delivered by the package. This path will be
  used to construct labels for the resources imported by the
  `ocaml_import` targets we generate. It does **not** necessarily
  match `bzlpkg_path`.

The `rsrc_path` is constructed from the `directory` properties of
findlib packages and subpackages. **IMPORTANT**: a subpackage may omit
this property, in which case its `rsrc_path` will be the same as its
parent package. From the `rsrc_path` we construct a corresponding
Bazel label; the first segment of this label will be `:_lib`, which
references a symlink within our `@opam` repo that points to
`$OPAM_SWITCH_PREFIX/lib`. So for example
`@opam//:_lib/ptime/ptime.cmxa` will resolve to
`$OPAM_SWITCH_PREFIX/lib/ptime/ptime.cmxa`

Example: `$(OPAM_SWITCH_PREFIX)/lib/ptime/META` defines package
`ptime.clock.os`, which delivers files in
`$(OPAM_SWITCH_PREFIX)/lib/ptime/os` (note that there is no `clock`
subdirectory in the switch). From this we will generate
`@opam//lib/ptime/clock/os`, i.e.
`$(OPAM_BZL)/lib/ptime/clock/os/BUILD.bazel`. This will contain an
`ocaml_import` target `:os`, whose `archive` attribute references
targets (files) in package `@opam//:_lib/ptime/os`; for example,
`@opam//:_lib/ptime/os/ptime_clock.cma`.

### "variable definition" in META

same "variable" can be RHS of multiple assign stmts, distinguished by "predicates"

Example: ctypes
```
  archive(byte) = "ctypes.cma"
  archive(byte, plugin) = "ctypes.cma"
  archive(byte, toploop) = "ctypes.cma ctypes-top.cma"
  archive(native) = "ctypes.cmxa"
  archive(native, plugin) = "ctypes.cmxs"
```

In addition the assignment has an op: set ("=") or update ("+=")

## Implementation notes

We use `UT_string` and `UT_array` from
[uthash](https://troydhanson.github.io/uthash/). The former makes it
easy to construct strings.

`char bzlroot[PATH_MAX]`: the output root dir
* prod: `$(bazel info output_base)/external/opam`. The bootstrapper
  will be run by a repository rule that defines the `@opam`
  repository. Evidently a side effect of running such a repository
  rule is to change directory to the corresponding location. So we can
  use relative paths to write files to the `@opam` repo.
* test: `$(pwd)/tmp/opam`

Our repository rule writes some symlinks and build files to `@opam`
before it runs the bootstrapper:

Symlinks to expose the OPAM resources we need:

* `switch_bin` : `$(OPAM_SWITCH_PREFIX)/bin` = `$ opam var bin`
* `switch_lib` : `$(OPAM_SWITCH_PREFIX)/lib` = `$ opam var lib`
* `bzl_bin`   : `$(bzlroot)/_bin`
* `bzl_lib`   : `$(bzlroot)/_lib`

the links:
* `bzl_bin` -> `switch_bin`
* `bzl_lib` -> `switch_lib`

We also write `(CWD)/BUILD.bazel`; among other things, this contains
`exports_files` rules exporting everything under `bzl_bin` and
`bzl_root`. Now our generated rules can refer to anything in these
switch directories (bin, lib) using labels like
`//:_lib/yojson/yojson.cmxa`.  Users can also use such labels, but
they are intended to be private, used only by the build files we
generate.


   pkg_prefix: "lib" - opam always puts pkgs under $OPAM_SWITCH_PREFIX/lib

   pkg_path: dir for the BUILD.bazel file we're emitting, relative to
             pkg_prefix

   rsrc_dir: contains the resources (archives, plugins) we're importing

   top-level pkg_path name always matches findlib pkg name, but the
   rsrc_dir can be anywhere (see below, special chars).

   subpkgs may have a 'directory' property, in which case it is
   concatenate to the rsrc_dir of its parent pkg. or the 'directory'
   property may be null (omitted), in which case the subpkg's rsrc_dir
   is the same as it's parent's.

   to complicate matters, the value of 'directory' may contain special
   characeters:

       ^ - ocaml std lib = lib/ocaml
       +path - subdir of ocaml std lib; e.g. +threads == lib/ocaml/threads
       /path - absolute path
       path - path relative to parent (or the dir containing the META
              file for top-level packages)



## ppx

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