"""Public Providers for obazl_rules_opam."""

OpamConfig = provider(
    doc = "OPAM configuration structure.",
    fields = {
        "version"  : "OPAM version",
        "switches" : "List of [OpamSwitch](#opamswitch) structures"
    }
)

OpamSwitch = provider(
    doc = """OPAM switch configuration.

    Package specification format (by example):

    - `"alcotest": ["1.1.0"]`
    - `"ppx_deriving_yojson": ["3.5.2", ["ppx_deriving_yojson.runtime"]]`
    - `"ppx_deriving": ["4.4.1", ["ppx_deriving.api", "ppx_deriving.enum"]]`
    - `"async_kernel": ["v0.12.0", "src/external/async_kernel"]` # pin pkg to path

    """,

    fields = {
        "default"  : "Must be True for exactly one switch configuration. Default: False",
        "compiler" : "OCaml compiler version",
        "packages" : "Dict of required OPAM packages. Keys: package name strings. Values: package spec."
    }
)
