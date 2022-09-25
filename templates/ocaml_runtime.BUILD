# generated file - DO NOT EDIT

package(default_visibility = ["//visibility:public"])

exports_files(glob(["**"], exclude = ["stdlib.cma", "std_exit.cmo"]))

load("@opam//build:rules.bzl", "opam_import")

opam_import(
    name = "stdlib",
    cma    = "stdlib.cma",
)

# opam_import_cmo(
#     name = "std_exit",
#     cmo  = ["std_exit.cmo"],
# )

# filegroup(
#     name = "stdlib",
#     srcs = ["stdlib.cma"]
# )

filegroup(
    name = "std_exit_cmo",
    srcs = ["std_exit.cmo"]
)
