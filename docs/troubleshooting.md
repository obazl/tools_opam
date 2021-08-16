# troubleshooting


Order matters!  OBazl currently relies on ocamlfind, and unfortuntely
ocamlfind does not handle dependency ordering correctly in some cases.
In particular, it will place local (project) lib deps after OPAM lib deps.

* Source files should (always?) come after lib deps on the (generated)
  command line. If the debug output shows this is not the case, it may
  be an OBazl bug.

* findlib predicates (e.g. "-predicates driver") will result in
  changes in the deps and args generated.

##

File "group_map/bw19.ml", line 179, characters 28-30:
Error: Signature mismatch:
       ...
       Values do not match:
         val equal : t -> t -> Ppx_deriving_runtime.bool
       is not included in
         val equal : t -> t -> bool
       File "group_map/field_intf.ml", line 28, characters 2-28:
         Expected declaration
       File "snarkette/fields.ml", line 10, characters 2-24:
         Actual declaration
Target //examples/anonvote:anonvote.exe failed to build

Fixed by adding to the ppx for group_map:

    lazy_deps = ["@opam//pkg:ppx_deriving.runtime"]
