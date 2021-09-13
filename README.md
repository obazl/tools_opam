# rules_opam
Bazel rules for OPAM support


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

These commands have lots of options, use `--help` to see them.

## dune "virtual" libraries

The concept of a "virtual" module was introduced by Dune; OCaml itself
neither has nor needs such a concept.

A “virtual module” is neither a module nor virtual. It’s just an
interface file. The concept involved is very simple: binding a
signature to a structure. You can do that at compile time or at link
time. Normally, when you have one .ml and the corresponding.mli, it
happens at compile time. Dune just decided to put the focus for
link-time binding on the signature part and call the sigs “virtual
modules”. But we don’t need the concept; it’s not an OCaml concept
(and it's confusing, IMHO). So OBazl will merely point out that you can
bind sigs to structs at link time if you wish, and you can select what
to bind based on config state.

Importantly, we do not need any special build machinery to support
this.

[Virtual libraries](http://rgrinberg.com/posts/virtual-libraries/)
2018 blog post.

NB: a note in the blog post: "As usual, Dune’s philosophy is to sweep
all these low level details under the rug, and provide users with a
high level API." OBazl's philosophy is the diametrical opposite: don't
sweep _anything_ under the rug; on the contrary, make everything
_clear_ and _explicit_, and trust the user to figure out how to use
the tool - including writing higher-level "sugar" that _does_ keep the
nasty details hidden.
