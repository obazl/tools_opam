# dependencies

## circular deps

Legacy packages may include circular or mutual dependencies. Bazel
disallows such dependencies.

Example: libfqfft/evaluation_domain/domains and libfqfft/polynomial_arithmetic are mutually dependent.

In this case listing each in the deps attribute of the other will
fail, since Bazel will detect the dependency cycle.

The "proper" way to address this is to refactor the packages to
eliminate the cycles.  But we cannot do that with legacy code we do
not control.

The workaround seems to be to use 'include_prefix' ...  This will make
compilation work, at the cost of removing at least one of the
dependencies, so a change in one will not force a recompile of the
other.  [Why does this work?]

## misc

WARNING: dune dependencies are expressed as *libraries*, or
*packages*.  In the latter case they are package-manager entitities.
Bazel deps may be individual modules, libraries, archives, etc. but
they are always build targets, not package-manager entities.

I.e. with dune one says "this build depends on that package", which
really means that it depends on compiled entities contained in the
package.  With bazel one says "this build depends on that built
entity".  That does not always imply something compiled by bazel; in
fact, with obazl, the targets in the @opam//pkg package produce
command line parameters for using OPAM packages.

But in the case of local or immediate deps, there is an ambiguity.

Example: src/lib/syncable_ledger.  The dunefile lists a library with
name "syncable_ledger". The directory contains a file named
"syncable_ledger.ml".  So if we depend on "syncable_ledger", what is
the dep, exactly?

The problem is that we use (by convention) target label ":foo" for
"foo.ml".  But if the lib name is also "foo" then we want to use
":foo" for the library, not the module.

Option: use ":foo_cm" for individual modules/files.  The disadvantage
of this is in label aesthetics.

Label concepts and aesthetics: the idea is that we treat each pkg as a
conceptual unit, and the directory name as the concept name. So we get
labels like "foo/bar:bar", which abbreviates to "foo/bar".  The
package may have additional targets, e.g. "foo/bar:baz", but the core
concept of the package is captured by the name "foo/bar".

Normally (?) a package will correspond to a library/archive containing
multiple modules.  The tricky bit, with dune, is that the library name
may match a module name.  This requires some renaming and module
aliasing.

Another option: use a naming convention for libs and archives,
e.g. "foo_lib" and "foo_archive".  But this prevents naming as above,
i.e. we would have foo/bar:bar_lib instead of just foo/bar.

Rule of thumb: use same name for directory and for library/archive
target, which should be the concept name.  For modules in the
lib/archive (i.e. source files in the dir), use :filename_cm (filename
without extension).

Then only lib/archive targets are exposed as local deps.

WARNING: ocamldep lists deps as module names. It doesn't have any idea
of library or package.

Builing this target with dune yields no fewer than 880+ build steps...


