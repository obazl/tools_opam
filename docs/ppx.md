# ppx

## ppx_jane

Kitchen sink.  Implicates a set of lazy deps.

To build a ppx_executable using ppx_jane, add
`@opam//pkg:ppx_expect.collector` and
`@opam//pkg:ppx_inline_test.runtime-lib` to `lazy_deps`.

BUT: ppx_jane is a shotgun. Not all users need expect or inline tests. Better not to use ppx_jane?

To use the executable, pass ppx_args = ["-inline-test-lib", "{{tag}}}"]

We cannot associate the args with the executable, since each user may pass a different {{tag}}.




requires(ppx_driver) = "base_quickcheck.ppx_quickcheck
                            ppx_runtime_deps = "base base_quickcheck"
                            requires base_quickcheck.ppx_quickcheck.expander
                                   ppx_runtime_deps = "base base_quickcheck"
                         ppx_expect
                            ppx_runtime_deps = "ppx_expect.collector"
                            requires(-ppx_driver) = "ppx_expect.collector ppx_inline_test.runtime-lib"
                        ppx_jane.kernel
                        ppx_stable"

requires(-ppx_driver) = "base
                         base_quickcheck
                         bin_prot
                         fieldslib
                         ppx_assert.runtime-lib
                         ppx_bench.runtime-lib
                         ppx_compare.runtime-lib
                         ppx_enumerate.runtime-lib
                         ppx_expect.collector
                         ppx_hash.runtime-lib
                         ppx_inline_test.runtime-lib
                         ppx_module_timer.runtime
                         ppx_sexp_conv.runtime-lib
                         typerep
                         variantslib"
ppx_jane.kernel:

  requires(ppx_driver) = "ppx_assert
                          ppx_base
                          ppx_bench
                          ppx_bin_prot
                          ppx_custom_printf
                          ppx_fail
                          ppx_fields_conv
                          ppx_here
                          ppx_inline_test
                          ppx_let
                          ppx_module_timer
                          ppx_optcomp
                          ppx_optional
                          ppx_pipebang
                          ppx_sexp_message
                          ppx_sexp_value
                          ppx_typerep_conv
                          ppx_variants_conv"

  requires(-ppx_driver) = "bin_prot
                           fieldslib
                           ppx_assert.runtime-lib
                           ppx_bench.runtime-lib
                           ppx_compare.runtime-lib
                           ppx_enumerate.runtime-lib
                           ppx_hash.runtime-lib
                           ppx_inline_test.runtime-lib
                           ppx_module_timer.runtime
                           ppx_sexp_conv.runtime-lib
                           typerep
                           variantslib"
