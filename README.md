# rules_opam
Bazel rules for OPAM support

summary:

* `bazel run @opam//init` - creates project-local OPAM installation
  with `--root .opam` and `--switch obazl`

* `bazel run @opam//init -- --import <file>` - same as `@opam//init`
  but then imports packages using <file>, which must 1) be located in
  `.opam.d` and 2) have the form of the files produced by `opam switch
  export`

* `bazel run @opam//install -- <pkg>` - installs one OPAM package in
  the project-local switch (root .opam, switch 'obazl')

* `bazel run @opam//status` - prints info about the effective OPAM switch.

* `bazel run @opam//ingest` - generates BUILD.bazel files from
  effective OPAM installation (either sys install or project-local),
  writes them to `.opam.d/buildfiles`, and `.opam.d/opam_repos.bzl`
  containing repo rules.


## opam commands

Useful commands:

* `opam var` - prints global opam vars, config vars from current
  switch, and a list of package variables that you can get by running:
* `opam var PKG:VAR` - print value of variable VAR for package PKG, e.g.
  * `opam var ocaml:lib` - prints path to the ocaml std lib, something like `~/
4.12.0/lib/ocaml`. So this will show you where to find the META file for any OPAM package.
    You can get the list of available variables with:
* `opam config list PKG` - print variables for package PKG; for
  example:
  * `opam config list ocaml` (NB: opam treats `ocaml` as a package)
* `opam info PKG` - prints a bunch of metadata, like versions, maintainer, etc.
* `opam show PKG` - same as `opam info PKG`?

* `opam install --download-only` (opam v. 2.1.?)

These commands have lots of options, use `--help` to see them.

## "core" libraries/archives/packages

**WARNING** In the literature "standard library" is sometimes used to
refer to `lib/ocaml`. Not to be confused with the module
[Stdlib](https://ocaml.org/api/Stdlib.html), "The OCaml Standard
Library", which is a namespaced archive (`stdlib.cmxa`) installed in
`lib/ocaml`.

The standard compiler distribution contains a set of resources
(archives, plugins) that the `findlib` system describes in "special"
META files. The packages desribed in these META files refer to files
in `lib/ocaml`; the directory holding the META file contains no Ocaml
files. In contrast, most (all?) other META package specs refer to
files in the same directory or one of its subdirectories.

Since these are standard, their BUILD.bazel files are predefined by
OBazl. That is, they are not generated from the META files by the
bootstrapper.

Furthermore we adapt findlib names to make them more idiomatic in
Bazel. Where findlib has `compiler-libs.X`, we have
`@ocaml//compiler-libs:X`:

* `compiler-libs` => `@ocaml//compiler-libs`
* `compiler-libs.common` => `@ocaml//compiler-libs/common`
* `compiler-libs.bytecomp` => `@ocaml//compiler-libs/bytecomp`
* `compiler-libs.optcomp` => `@ocaml//compiler-libs/optcomp`
* `compiler-libs.toplevel` => `@ocaml//compiler-libs/toplevel`
* `compiler-libs.native-toplevel` => `@ocaml//compiler-libs/native-toplevel`

The others we also put in the `@ocaml` namespace:

* `bigarray` => `@ocaml//bigarray`
* `dynlink`  =>  `@ocaml//dynlink`
* `str` => `@ocaml//str`
* `unix` => `@ocaml//unix`

The `stdlib` module is always included (and `open`ed) by the compiler,
so there is no build target for it.

Threads require special treatment because threading support has
changed over the years. `ocamlfind` has command-line switchs (`-mt`,
`-mt_vm`, `-mt_posix`) for threading, but there are no corresponding
options for the compilers; building with thread support just requires
depending the the threading lib. Support for virtual threads was
removed in version X, but threads/META still exposes `threads.vm`.

OBazl does not support the `ocamlfind` threading options, eliminates
the distinction between posix and vm threads.

* `threads.posix` => `@ocaml//threads`
* `threads.vm`    =>  `@ocaml//threads`

The list:

* lib/bigarray
* lib/compiler-libs
* lib/dynlink
* [lib/stdlib]?
* lib/str
* lib/threads
* lib/unix

Note that these correspond to archives or subdirs in `/lib/ocaml`.

Creating a fresh `4.13.0` switch installs the following in
`${SWITCHPREFIX}/lib`.  No findlib `META` files are installed.

* `ocaml` - the "core" archives and related files (cmx, cmi, etc.):
  * `bigarray.cmxa`
  * `dynlink.cmxa`
  * `stdlib.cmxa`
  * `str.cmxa`
  * `unix.cmxa`

  "core" plugins:
  * `bigarray.cmxs`
  * `str.cmxs`
  * `unix.cmxs`

  `lib/ocaml` also has the following subdirectories:

  * `stublibs` - libs used by `ocamlrun` to deal with C dependencies.: `dllcamlstr.so`, `dllthreads.so`, `dllunix.so`
  * `caml` - C headers
  * `compiler-libs` - contains archives, exported by /lib/compiler-libs:
    * `ocamlbytecomp.cmxa`
    * `ocamlcommon.cmxa`
    * `ocamlmiddleend.cmxa`
    * `ocamloptcomp.cmxa`
  * `threads` - `threads.cmxa`, exported by `lib/threads`
  * `ocamldoc`

* `stublibs` - empty!

* `toplevel` - empty!

## special opam packages

### compiler libs

* pkg "compiler-libs" - provided by OCaml core? Not found as a
  separate package in opam-repository, so evidently it gets installed
  when a switch is created(?). The META file in an opam installation says
  "distributed with Ocaml", and defines subpackages whose files come
  from `lib/ocaml/compiler-libs" ( directory = "+compiler-libs" ).

* pkg [ocaml-compiler-libraries](https://github.com/janestreet/ocaml-compiler-libs) - dune-based repackaging of compiler-libraries.  Essentially partitions the libraries into namespaces.
