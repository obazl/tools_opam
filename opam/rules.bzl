"""Public definitions for tools_opam rules.

All public tools_opam rules imported and re-exported in this file.

Definitions outside this file are private unless otherwise noted, and
may change without notice.
"""

load("//opam/_rules:repo_rules.bzl",
    _new_local_opam_repository = "new_local_opam_repository")

new_local_opam_repository = _new_local_opam_repository
