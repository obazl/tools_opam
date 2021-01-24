# Copyright 2020 Gregg Reynolds. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

OpamPkgInfo = provider(
    doc = "Provider for OPAM packages.",
    fields = {
        ## clients must write: dep[OpamPkgInfo].pkg.to_list()[0].name
        ## FIXME: make pkg contain just the label? no need for a depset.
        "pkg": "Label depset containing package name string used by ocamlfind.",
        "ppx_driver": "True if ocamlfind would generate -ppx command line arg when this lib is listed as a dep."
    }
)

OpamConfig = provider(
    doc = "OPAM configuration structure.",
    fields = {
        "version"  : "OPAM version",
        "switches" : """Dictionary from switch name strings to [OpamSwitch](#opamswitch) structures.
Example:
```
PACKAGES = {"bin_prot": ["v0.12.0"], ...}

opam = OpamConfig(

    version = "2.0",

    switches  = {

        "mina-0.1.0": OpamSwitch(

            default  = True,

            compiler = "4.07.1",

            packages = PACKAGES

        ),

        "4.07.1": OpamSwitch(

            compiler = "4.07.1",

            packages = PACKAGES

        ),

    }

)
```
"""
    }
)

OpamSwitch = provider(
    doc = """OPAM switch configuration.

Example:

```
OpamSwitch(
    default  = True,
    compiler = "4.07.1",
    packages = {
        "async": ["v0.12.0"],
        "bytes": [],
        "core": ["v0.12.1"],
        "ctypes": ["0.17.1", ["ctypes.foreign", "ctypes.stubs"]],
        "ppx_deriving": ["4.4.1", [
            "ppx_deriving.enum",
            "ppx_deriving.eq",
            "ppx_deriving.show"
        ]],
        "ppx_deriving_yojson": ["3.5.2", ["ppx_deriving_yojson.runtime"]],
        "unix": [],
    }
)
```
    """,

    fields = {
        "default"  : "Must be True for exactly one switch configuration. Default: False",
        "compiler" : "OCaml compiler version",
        "packages" : """List of `<pkg name string>: [<version string>] | [<version string> [<subpkg names>]]`, where:

```
<pkg name string> := name string used for `opam` or `ocamlfind` commands

<version string>  := version string as printed by `opam list`

<subpkg names>    := list of subpackage name strings as used by ocamlfind
```
Subpackage name strings have the form <pkg>.<subpkg>, and may be discovered by running `ocamlfind list`.

**Exception**: for packages that are distributed with the compiler and
  have no version string, use the empty list `[]`; e.g. `"bytes": []`.
"""
    }
)
