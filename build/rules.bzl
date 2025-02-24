"""Public definitions for OPAM rules.

All public OPAM rules imported and re-exported in this file.

Definitions outside this file are private unless otherwise noted, and
may change without notice.
"""

load("//build/_rules:opam_install.bzl",
     _opam_install = "opam_install")
# load("//build/_rules:opam_proxy.bzl", _opam_proxy = "opam_proxy")

opam_install    = _opam_install
# opam_proxy = _opam_proxy
