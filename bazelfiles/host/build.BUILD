# generated file - DO NOT EDIT

package(default_visibility = ["//visibility:public"])

constraint_setting(name = "build")

constraint_value(name = "native",   constraint_setting = ":build")

constraint_value(name = "vm", constraint_setting = ":build")

platform(name = "nc",
         parents = ["@local_config_platform//:host"],
         constraint_values = [
             ":native",
         ])
