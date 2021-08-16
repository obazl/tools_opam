# notes

## ppx general

A collection of links:

Metaprogramming and PPX
https://ocamlverse.github.io/content/metaprogramming.html

Whitequark's original (Apr 2014):
https://whitequark.org/blog/2014/04/16/a-guide-to-extension-points-in-ocaml/

A Guide to PreProcessor eXtensions:
https://ocamlverse.github.io/content/ppx.html

A Tutorial to OCaml -ppx Language Extensions
Jun 19, 2018 · last updated on Jun 26, 2018:
https://victor.darvariu.me/jekyll/update/2018/06/19/ppx-tutorial.html

Rudi Grinberg (2017):

http://rgrinberg.com/posts/extension-points-3-years-later/

Preprocessor extensions for code generation (May 2017):

https://blog.shaynefletcher.org/2017/05/preprocessor-extensions-for-code.html

## basic (legacy) ppx

OCaml doc ch 27

Module Ast_mapper (https://caml.inria.fr/pub/docs/manual-ocaml/compilerlibref/Ast_mapper.html)

Registration API:

val register : string -> (string list -> mapper) -> unit
Apply the register_function. The default behavior is to run the mapper immediately, taking arguments from the process command line. This is to support a scenario where a mapper is linked as a stand-alone executable.

"It is possible to overwrite the register_function to define "-ppx drivers", which combine several mappers in a single process. Typically, a driver starts by defining register_function to a custom implementation, then lets ppx rewriters (linked statically or dynamically) register themselves, and then run all or some of them. It is also possible to have -ppx drivers apply rewriters to only specific parts of an AST."

## cookies

https://discordapp.com/channels/436568060288172042/469167266143928320/759399869688447007

"craigfe 9/26/20 at 8:03 AM
@gar: The idea behind cookies was to allow PPX rewriters to keep their own persistent state between separate invocations by the OCaml compiler (see https://github.com/ocaml/ocaml/issues/5904). This was back before OMP became a thing, when multiple PPX passes were implemented by multiple marshaling steps. In the more modern era of Ppxlib and Omp, I don't think it's ever necessary to use such mechanisms, since all PPX transformations occur within a single process lifetime."

https://discordapp.com/channels/436568060288172042/469167266143928320/759402107568390154
"craigfe 9/26/2020 at 8:12 AM
I've never come across a modern PPX that either requires a -cookie arg or requests one in its docs, but in theory a modern PPX shouldn't need it"

https://github.com/ocaml/ocaml/issues/5904:

"Commit 15314 adds a notion of "persistent cookies", which can be set by ppx processors and retrieved on further invocations when called from the toplevel.
Cookies are identified by an arbitrary string and hold an arbitrary Parsetree expression.

It is the responsibility of each ppx processor to store a cookie that represent its state and then restore it (using Ast_mapper.get_cookie/set_cookie). Note that cookies are available when the function registered through Ast_mapper.register is executed, so it is possible to set some global variables there."


OMP = Ocaml Migrate Parsetree, see below.

Used a lot in migrate_parse_tree - something to do with registering rewriters?

## ppx drivers

From dune docs (https://testrtdxyz.readthedocs.io/en/latest/advanced-topics.html):

```
Using a custom ppx driver

You can use a custom ppx driver by putting it as the last library in
(pps ...) forms. An example of alternative driver is ppx_driver. To
use it instead of ocaml-migrate-parsetree.driver-main, simply write
ppx_driver.runner as the last library:

(preprocess (pps (ppx_sexp_conv ppx_bin_prot ppx_driver.runner)))

Driver expectation
Jbuilder will invoke the executable resulting from linking the libraries given in the (pps ...) form as follows:

ppx.exe <flags-written-by-user> --dump-ast -o <output-file> \
  [--cookie library-name="<name>"] [--impl|--intf] <source-file>
Where <source-file> is either an implementation (.ml) or interface (.mli) OCaml source file. The command is expected to write a binary OCaml AST in <output-file>.

Additionally, it is expected that if the executable is invoked with --as-ppx as its first argument, then it will behave as a standard ppx rewirter as passed to -ppx option of OCaml. This is for two reason:

* to improve interoperability with build systems that Jbuilder
* so that it can be used with merlin
```


#### obazl ppx driver

use the source driver: `srcs = ["@obazl//ppxlib:driver_standalone_runner"],`


adding one of the precompiled drivers from opam won't work:
    #     ## this won't work - ocamlfind insists on reordering deps,
    #     ## putting opam deps before local deps, so this does not
    #     ## guarantee that the runner comes last:
    #     # "@opam//pkg:ppxlib.runner",


## ppx_core

5/2016:
https://blog.janestreet.com/ppx_core-context-free-rewriters-for-better-semantic-and-faster-compilation/

Documents the transition from piecemeal to compound PPX processing.

Contains examples using ppx flags `-inline-test-lib`, `-inline-test-drop`, `-no-check`, `-no-merge`

"All in all, our standard set has 19 rewriters:

ppx_assert
ppx_bench
ppx_bin_prot
ppx_compare
ppx_custom_printf
ppx_enumerate
ppx_expect
ppx_fail
ppx_fields_conv
ppx_here
ppx_inline_test
ppx_js_style*
ppx_let
ppx_pipebang
ppx_sexp_conv
ppx_sexp_message
ppx_sexp_value
ppx_typerep_conv
ppx_variants_conv"

## ppx_jane

Apparently a kitchen-sink ppx lib: "ppx-jane which [is] a standard
driver containing all of the Jane Street ppx rewriters linked in" (https://blog.janestreet.com/ppx_core-context-free-rewriters-for-better-semantic-and-faster-compilation/)

Predates ppx_core?

`kernel/src/dune`:

(library (name ppx_jane_kernel) (public_name ppx_jane.kernel)
 (kind ppx_rewriter)
 (libraries ppx_base ppx_assert ppx_bench ppx_bin_prot ppx_custom_printf
  ppx_disable_unused_warnings ppx_fields_conv ppx_fixed_literal ppx_here
  ppx_inline_test ppx_let ppx_module_timer ppx_optional ppx_optcomp
  ppx_pipebang ppx_sexp_message ppx_sexp_value ppx_string ppx_typerep_conv
  ppx_variants_conv)
 (preprocess no_preprocessing))

Some uses pass flags, e.g. Coda async_kernel/src/dune:

(pps ppx_jane -annotated-ignores -check-doc-comments)

But there is no sign of either of these flags in the ppx_jane code!

## ppx_js_style

Ppx JaneStreet Style -  Enforce Jane Street coding styles

https://github.com/janestreet/ppx_js_style.git

Good example of using Driver.add_arg

-dated-deprecation
-allow-unannotated-ignores
 NB:  `ignore` is a predefined fn

-check-doc-comments
-compat-32
-dont-check-underscored-literal

See also "OCaml Programming Guidelines" https://ocaml.org/learn/tutorials/guidelines.html

## bisect_ppx

https://github.com/aantron/bisect_ppx.git

Includes new dep  bisect_ppx.runtime

According to the readme, the `--conditional` flag makes processing
dependent on the env var BISECT_ENABLE=YES. But the code says either
the flag or the env var can be used to enable the lib.

Either way: this lib could be one of many in a compound ppx. CLI
params cannot be passed to a single lib in the collection. And since
ppx libs can define cli params, it must be that the CLI interface of
the executable must be the conjunction of the lib interfaces.

So what does it mean in dune, e.g. tuple_lib:

` (pps ppx_jane ppx_deriving.eq bisect_ppx -- -conditional))`

If this means "pass --conditional when you run the compiled ppx", then
coverage would depend on both the flag and the env var, which makes no
sense.

The lib registers with `Migrate_parsetree.Driver.register`. This must
have the side effect of adding its CLI args to the driver interface.

In any case, if bisect_ppx is disabled it runs `Migrate_parsetree.Ast_411.shallow_identity`

Takes a bunch of cli args - these are args for the resulting ppx
executable? Apparently; try running `bisect_ppx/ppx.exe` from opam repo.

I.e. each ppx lib implies a CLI protocol for the executable you get
from compiling with the lib.

Summary: if bisect_ppx is used, then for e.g. ocaml_compile.ppx_args
we need to select on something like //bzl/config:enable_bisect_ppx to
enable or disable the lib. Of course, we could get the same result by
compiling two ppx executables, one with and one without the lib.

## ppx testing

Note that the ppx built for a ppx_test target goes in its runtime
files, so it is not in the same place as it would be if built
directly. If you want to run it by hand, build it directly, e.g. `$ bazel build test:ppx_exe`


### ppx_inline_test

https://github.com/janestreet/ppx_inline_test

Insanity.

"If you use some other build system, code using this
extension must be compiled and linked using the
ppx_inline_test.runtime-lib library. The ppx_inline_test syntax
extension will reject any test if it wasn't passed a -inline-test-lib
libname flag." (https://github.com/janestreet/ppx_inline_test)

What this means, but does not quite say, is that the (ppxlib) Driver
(module) for the ppx_inline_test lib takes this arg. It also takes a
`library-name` cookies arg. And `ppx_inline_test.runtime-lib` must be
included as a compile-time dep.

Both of these take a `libname`, and then set
`Ppx_inline_test_libname.libname`, whose accessor is
`Ppx_inline_test_libname.get ()`. This is used by `ppx_inline_test1`,
`ppx_bench`, and `ppx_expect` to determine if the required arg was
passed (or the lib is "enabled" (ppx_bench), or
can_use_test_extensions (ppx_inline_test)).

"This libname is a way of restricting the tests run by the executable.
The dependencies of your library (or executable) could also use
ppx_inline_test, but you don't necessarily want to run their tests
too. For instance, core is built by giving -inline-test-lib core and
core_extended is built by giving -inline-test-lib core_extended. And
now when an executable linked with both core and core_extended is run
with a libname of core_extended, only the tests of core_extended are
run."

What does this mean? How is core "built by giving -inline-test-lib
core"? You can't pass `-inline-test-lib` to the compiler AFAIK. A
search for "inline-test-lib" in those repos comes up empty.

That means you pass `-inline-test-lib` to the ppx executable, which
must be controlled by Ppxlib.Driver. With OBazl this means passing it
with the `ppx_args` attribute of `ocaml_module` etc.

"Tests are executed when the executable containing the tests is called
with command line arguments:

your.exe inline-test-runner libname [options]"

"Finally, after running tests, Ppx_inline_test_lib.Runtime.exit ()
should be called (to exit with an error and a summary of the number of
failed tests if there were errors or exit normally otherwise)."

### ppx_expect

https://github.com/janestreet/ppx_expect

"When run, these tests will pass iff the output matches what was
expected. If a test fails, a corrected file with the suffix
“.corrected” will be produced with the actual output, and the
inline_tests_runner will output a diff."

So that's what the `-corrected-suffix` param is for!

Building: "Follow the same rules as for ppx_inline_test. Just make
sure to include ppx_expect.evaluator as a dependency of the test
runner. The Jane Street tests contains a few working examples using
oasis."

Great, that's clear. Not a word in the docs about
`ppx_expect.collector`, which is the "runtime" dep.

META file:

requires(ppx_driver) = "base
                        ppx_expect.common
                        ppx_expect.payload
                        ppx_here.expander
                        ppx_inline_test
                        ppx_inline_test.libname
                        ppxlib
                        ppxlib.metaquot_lifters"
ppx_runtime_deps = "ppx_expect.collector"
requires(-ppx_driver) = "ppx_expect.collector ppx_inline_test.runtime-lib"
package "collector" (
  requires = "fieldslib
    ppx_expect.common ppx_expect.config
    ppx_inline_test.runtime-lib
    ppx_sexp_conv.runtime-lib"

package "evaluator" (
  requires = "base
    ppx_expect.collector ppx_expect.common ppx_expect.matcher
    ppx_sexp_conv.runtime-lib ppxlib.print_diff stdio"

## OMP (ocaml migrate parsetree)

https://discuss.ocaml.org/t/ppx-omp-2-0-0-and-next-steps/6231

## ppxlib

https://discuss.ocaml.org/t/ppx-omp-2-0-0-and-next-steps/6231

"ppxlib is the first ppx library officially supported by the OCaml platform,"

(https://discuss.ocaml.org/t/ppx-omp-2-0-0-and-next-steps/6231)

### Ppxlib.Driver

submodule: cookies
Cookies??? The Cookies modules is in ppxlib/src/driver.ml. Each cookie
has one (or more?) handler.

submodule: Transform. impl, intf, aliase, etc. register fn to register
code transformers.

cookies fns use tool_name "ppxlib_driver"

standalone_main(), standalone_args - infer that the code this module
controls the compiled ppx executable. `Ppxlib.Driver.standalone ()` is
the entry point for ppx executables.

What's with `--as-ppx`?  We're already a ppx! How else can we run?

### Ppxlib.Deriving

## ppx_driver


## ocamlfind

### Predicates - build settings?

"Dune uses ``META`` files to support external libraries. However, it
doesn't export the full power of findlib to the user, and especially
it doesn't let the user specify *predicates*." (doc/advanced-topics.rst)

META predicat ``ppx_driver``: "when a library acts differently
  depending on whether it is linked as part of a driver or meant to
  add a ``-ppx`` argument to the compiler, choose the former behavior"



E.g. ppx_optcomp META: `requires(ppx_driver) = "base compiler-libs.common ppxlib stdio"`

Then similar for "variables" archive, plugin, ppxopt, library_kind

Negation of predicates - selects different set of deps. e.g. ppx_optcomp:

requires(ppx_driver) = "base compiler-libs.common ppxlib stdio"
requires(-ppx_driver) = ""
requires(-ppx_driver,-custom_ppx) += "ppx_deriving"
ppxopt(-ppx_driver,-custom_ppx) = "ppx_deriving,package:ppx_optcomp"

WTF?

"The variable 'ppx' is a command that is added to the compiler invocation via the -ppx option..."
"The variable 'ppxopt' is a set of options that are added to the ppx rewriter invocation..."

ppx_bin_prot:

requires(ppx_driver) = "base
                        compiler-libs.common
                        ppx_bin_prot.shape-expander
                        ppxlib"
...
ppx_runtime_deps = "bin_prot"
...
library_kind = "ppx_deriver"

base:
requires = "base.caml base.shadow_stdlib sexplib0"
linkopts(javascript) = "+base/runtime.js"
jsoo_runtime = "runtime.js"
no ppx_driver

compiler-libs.common
  requires = "compiler-libs"
no ppx_deriver

ppxlib:
requires: base, compiler-libs.common, ocaml-compiler-libs.common, ocaml-compiler-libs.shadow, ocaml-migrate-parsetree, ppx_derivers, ppxlib.ast, ppxlib.print_diff, ppxlib.traverse_builtins, stdio

ppxlib.metaquot:
  requires(ppx_driver) = "ppxlib
                          ppxlib.metaquot_lifters
                          ppxlib.traverse_builtins"
  requires(-ppx_driver) = ""
  ppx(-ppx_driver,-custom_ppx) = "./ppx.exe --as-ppx"
  library_kind = "ppx_rewriter"

ppxlib.metaquot_lifters:
  requires = "ppxlib ppxlib.traverse_builtins"

ppxlib.traverse_builtins:
  requires = ""

ocaml-migrate-parsetree:
  requires = "compiler-libs.common ppx_derivers result"

ppx_derivers:
  requires = ""

ppx_deriving:
 ppx(-custom_ppx) = "./ppx_deriving"
 requires = "ppx_deriving.runtime"

ppx_deriving.eq:
  requires(ppx_driver) = "compiler-libs.common ppx_deriving.api ppx_tools"
  ...
  requires(-ppx_driver) = "ppx_deriving.runtime"
  requires(-ppx_driver,-custom_ppx) += "ppx_deriving"
  ppxopt(-ppx_driver,-custom_ppx) = "ppx_deriving,package:ppx_deriving.eq"
  library_kind = "ppx_deriver"

ppx_deriving.api:
  requires =
  "compiler-libs.common ocaml-migrate-parsetree ppx_derivers ppx_tools result"
  ppx_runtime_deps = "ppx_deriving.runtime"

ppx_deriving.runtime:
  requires = "result"

### ppx runtime libs:

rg -I --no-line-number "\.runtime" .opam/4.07.1/lib/ | sort | uniq > ppx_runtime.log

edited to remove dups:

bisect_ppx.runtime
js_of_ocaml-compiler.runtime.num
ppx_assert.runtime-lib
ppx_bench.runtime-lib
ppx_compare.runtime-lib
ppx_deriving.runtime
ppx_deriving.runtime result
ppx_deriving_yojson.runtime
ppx_enumerate.runtime-lib
ppx_expect.collector
ppx_hash.runtime-lib
ppx_inline_test.runner.lib
ppx_inline_test.runtime-lib
ppx_module_timer.runtime
ppx_sexp_conv.runtime-lib

Only 14

### ppx_runtime_deps

rg ppx_runtime_deps .opam/.../lib:

.opam/4.07.1/lib/ppx_fields_conv/META:ppx_runtime_deps = "fieldslib"
.opam/4.07.1/lib/ppx_fields_conv/dune-package: (ppx_runtime_deps fieldslib)
.opam/4.07.1/lib/ppx_sexp_message/dune-package: (ppx_runtime_deps ppx_sexp_conv.runtime-lib)
.opam/4.07.1/lib/ppx_sexp_message/META:ppx_runtime_deps = "ppx_sexp_conv.runtime-lib"
.opam/4.07.1/lib/js_of_ocaml-ppx/META:ppx_runtime_deps = "js_of_ocaml"
.opam/4.07.1/lib/ppx_enumerate/dune-package: (ppx_runtime_deps ppx_enumerate.runtime-lib)
.opam/4.07.1/lib/ppx_enumerate/META:ppx_runtime_deps = "ppx_enumerate.runtime-lib"
.opam/4.07.1/lib/ppx_bench/dune-package: (ppx_runtime_deps ppx_bench.runtime-lib)
.opam/4.07.1/lib/ppx_bench/META:ppx_runtime_deps = "ppx_bench.runtime-lib"
.opam/4.07.1/lib/bisect_ppx/dune-package: (ppx_runtime_deps bisect_ppx.runtime)
.opam/4.07.1/lib/bisect_ppx/META:ppx_runtime_deps = "bisect_ppx.runtime"
.opam/4.07.1/lib/ppx_sexp_conv/dune-package: (ppx_runtime_deps ppx_sexp_conv.runtime-lib)
.opam/4.07.1/lib/ppx_assert/dune-package: (ppx_runtime_deps ppx_assert.runtime-lib)
.opam/4.07.1/lib/ppx_assert/META:ppx_runtime_deps = "ppx_assert.runtime-lib"
.opam/4.07.1/lib/ppx_sexp_conv/META:  ppx_runtime_deps = "ppx_sexp_conv.runtime-lib"
.opam/4.07.1/lib/ppx_hash/dune-package: (ppx_runtime_deps ppx_hash.runtime-lib)
.opam/4.07.1/lib/ppx_hash/META:  ppx_runtime_deps = "ppx_hash.runtime-lib"
.opam/4.07.1/lib/lens/dune-package: (ppx_runtime_deps lens)
.opam/4.07.1/lib/ppx_typerep_conv/dune-package: (ppx_runtime_deps typerep)
.opam/4.07.1/lib/ppx_typerep_conv/META:ppx_runtime_deps = "typerep"
.opam/4.07.1/lib/lens/META:  ppx_runtime_deps = "lens"
.opam/4.07.1/lib/ppx_module_timer/dune-package: (ppx_runtime_deps ppx_module_timer.runtime)
.opam/4.07.1/lib/ppx_variants_conv/dune-package: (ppx_runtime_deps variantslib)
.opam/4.07.1/lib/ppx_variants_conv/META:ppx_runtime_deps = "variantslib"
.opam/4.07.1/lib/ppx_module_timer/META:ppx_runtime_deps = "ppx_module_timer.runtime"
.opam/4.07.1/lib/ppx_inline_test/dune-package: (ppx_runtime_deps ppx_inline_test.runtime-lib)
.opam/4.07.1/lib/ppx_inline_test/META:ppx_runtime_deps = "ppx_inline_test.runtime-lib"
.opam/4.07.1/lib/ppx_compare/dune-package: (ppx_runtime_deps ppx_compare.runtime-lib)
.opam/4.07.1/lib/ppx_compare/META:  ppx_runtime_deps = "ppx_compare.runtime-lib"
.opam/4.07.1/lib/base_quickcheck/dune-package: (ppx_runtime_deps base base_quickcheck)
.opam/4.07.1/lib/base_quickcheck/META:    ppx_runtime_deps = "base base_quickcheck"
.opam/4.07.1/lib/ppx_deriving/META:  ppx_runtime_deps = "ppx_deriving.runtime"
.opam/4.07.1/lib/ppx_deriving_yojson/dune-package: (ppx_runtime_deps ppx_deriving_yojson.runtime yojson)
.opam/4.07.1/lib/ppx_deriving_yojson/META:ppx_runtime_deps = "ppx_deriving_yojson.runtime yojson"
.opam/4.07.1/lib/ppx_bin_prot/dune-package: (ppx_runtime_deps bin_prot)
.opam/4.07.1/lib/ppx_bin_prot/META:ppx_runtime_deps = "bin_prot"
.opam/4.07.1/lib/ppx_expect/dune-package: (ppx_runtime_deps ppx_expect.collector)
.opam/4.07.1/lib/ppx_expect/META:ppx_runtime_deps = "ppx_expect.collector"
.opam/4.07.1/lib/ppx_sexp_value/dune-package: (ppx_runtime_deps ppx_sexp_conv.runtime-lib)
.opam/4.07.1/lib/ppx_sexp_value/META:ppx_runtime_deps = "ppx_sexp_conv.runtime-lib"


## OCamlfind and Dune

Dune evidently has added some "variables" to the ocamlfind META grammar.

* `ppx_runtime_deps` - "This is what dune uses to find out the runtime dependencies of a preprocessor"

* `library_kind` - `ppx_deriver` or `ppx_rewriter`

* `ppx` - a string to be passed to the compiler? e.g. for ppx_tools, pkg metaquot:
  ** `  ppx = "./ppx_metaquot"`


################################################################
2020/09/21 09:56:04 Falling back to GCS due to GitHub error: could not get releases from github.com/bazelbuild/bazel: could not download list of Bazel releases from github.com/bazelbuild: unexpected status code while reading https://api.github.com/repos/bazelbuild/bazel/releases: 403

wait a while and try again


## static linking

https://eli.thegreenplace.net/2013/07/09/library-order-in-static-linking

https://www.systutorials.com/how-to-statically-link-c-and-c-programs-on-linux-with-gcc/

Building static lib with static deps (aggregating .a files):

https://github.com/bazelbuild/bazel/issues/1920

https://github.com/bazelbuild/rules_cc/tree/master/examples/my_c_archive

## Compiling

The key to compiling-with-deps is the cmi files. You cannot pass them
on the command line, you must add them to the search path using `-I`.

For namespace modules: the submodules must be on the search path.
Listing them on the command line is not sufficient and may have no effect.

## troubleshooting

#### async_kernel

File "src/synchronous_time_source0.ml", line 165, characters 32-37:
Error: This expression has type Async_kernel__Job_or_event_intf.Event.t
       but an expression was expected of type Event.t

fix: add :Job_or_event_intf as dep of synchronous_time_source0.ml

File "src/clock_ns.ml", line 1:
Error: The implementation bazel-out/darwin-fastbuild/bin/src/Async_kernel__Clock_ns.ml
       does not match the interface bazel-out/darwin-fastbuild/bin/src/async_kernel__Clock_ns.cmi:
       Values do not match:
         val at : Time_ns.t -> unit Async_kernel__Time_source_intf.Deferred.t
       is not included in
         val at : Time_ns.t -> unit Deferred1.t
       File "src/clock_intf.ml", line 31, characters 2-36:
         Expected declaration
       File "src/clock_ns.ml", line 11, characters 4-6: Actual declaration

Fix: add ":Time_source_intf" as dep of time_source.ml

####

Error: ppx_inline_test: extension is disabled because the tests would be ignored (the build system didn't pass -inline-test-lib)

Fix: pass ["-inline-test-lib", "tag"] to ppx_args attrib of
ocaml_module, where "tag" can be anything, apparently.

#### bad type

File "bazel-out/darwin-fastbuild/bin/src/lib/logproc_lib/Logproc_lib__Filter.ml", line 162, characters 23-48:
Error: This expression has type
         consume:Consume.t -> (Ast.t, string) Stdlib.result
       but an expression was expected of type ('a, 'b) Stdlib.result

In this case the problem was wrong version of the angstrom opam lib.
Lesson: always start by verifying you have the right versions of all
the external deps.

#### ocaml-extlib:

File "src/refList.ml", line 1:
Error: The implementation src/refList.ml
       does not match the interface bazel-out/darwin-fastbuild/bin/src/refList.cmi:
       ...
       In module Index:
       Values do not match:
         val index_of : unit list ref -> unit -> int
       is not included in
         val index_of : 'a t -> 'a -> int
       File "src/refList.mli", line 179, characters 2-34:
         Expected declaration
       File "src/refList.ml", line 122, characters 6-14: Actual declaration

Problem was passing `-strict-sequence`
https://github.com/ocaml/ocaml/pull/1971



## (self_build_stubs_archive (snarky_bn382_stubs))
## means we expect snarky_bn382_stubs.a
## but we don't have snarky_bn382_stubs.c, only snarky_bn382.c

## apparently, since lib snarky_bn382 depends on lib
## snarky_bn382_stubs, and it provides a c_name (snarky_bn382.c) and c
## flags, the latter will use this c file to produce its output. what
## does c_names mean? we don't know, since the manual only notes that
## it is "now deleted." evidently, refers to c files to be compiled by dune
## and "packaged together with" the lib.
