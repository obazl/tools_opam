"""Public definitions for OCaml rules.

All public OCaml rules imported and re-exported in this file.

Definitions outside this file are private unless otherwise noted, and
may change without notice.
"""

load("//build/rules:opam_import.bzl",
     _opam_import = "opam_import")
load("//build/rules:opam_import_test.bzl",
     _opam_import_test_suite = "opam_import_test_suite")

opam_import = _opam_import
opam_import_test_suite = _opam_import_test_suite
