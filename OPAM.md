# opam

Several ways to present opam resources

* using ocamlfind. opam rules produce the string used by ocamlfind
* using ocaml_import with a distinguished `@_opam` repo. The repo
  exposes all the files in opam, the import rules refer to them

Then we have the option of using a hermetic build vs. a localhost build.

So there are several options for placement.

* using local opam installation:
  * generate build files from opam META files ahead of time. Put them
  in some place like `${HOME}/.local/share/opam`, and use
  `local_repository` to import it.
  * generate build files from META files during bootstrap and put them
    in e.g. `${HOME}/.cache/opam`, to avoid polluting the local OPAM
    installation.
  * bootstrap an `@opam` repo with a `//lib/` or `//pkg` package
  containing ocamlfind opam rules. this is the current implementation.
* hermetic build:
  * bootstrap `@opam` and populate it with tree of bazel packages
  generated during bootstrap by parsing opam META files.
  * for a hermetic config, after opam resources have been downloaded
    and installed, generate the build files and place them directly in
    the opam tree, which is private to the build.
