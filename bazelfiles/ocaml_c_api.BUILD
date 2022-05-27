load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "c",
    srcs = glob(["lib/*.a", "caml/*.h"]),
    hdrs = glob(["caml/**"]),
    visibility = ["//visibility:public"],
);
