#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>              /* open() */
#include <libgen.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if INTERFACE
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif

#include "utarray.h"
#include "utstring.h"
#endif

#define BUFSZ 4096

#include "log.h"
#include "opam.h"

extern int errnum;
bool opam_cmds_debug;
bool opam_cmds_verbose;

/* global: we write on local_repository rule per build file */
FILE *repo_rules_FILE;

/* char *here_obazl_root = HERE_OBAZL_ROOT; // only for here-switch!!! */
/* char *here_switch_bazel_root = HERE_SWITCH_BAZEL_ROOT; */

void init_opam_resolver_raw(char *_opam_switch_name)
{
    printf("init_opam_resolver_raw, switch: %s\n", _opam_switch_name);
    /* we're going to write out a scheme file that our dune conversion
       routines can use to resolve opam pkg names to obazl target
       labels. */

    UT_string *raw_resolver_file;
    utstring_new(raw_resolver_file);
    utstring_printf(raw_resolver_file, "%s/%s/%s",
                    HERE_COSWITCH_ROOT,
                    _opam_switch_name,
                    "opam_resolver_raw.scm");
    /* if (access(raw_resolver_file, R_OK) != 0) return; */

    printf("opam_resolver_raw: %s\n", utstring_body(raw_resolver_file));

    if (dry_run) {
        printf("opam_resolver_raw: %s\n", utstring_body(raw_resolver_file));
        return;
    }

    extern FILE *opam_resolver;  /* in emit_build_bazel.c */
    opam_resolver = fopen(utstring_body(raw_resolver_file), "w");
    if (opam_resolver == NULL) {
        perror(utstring_body(raw_resolver_file));
        exit(EXIT_FAILURE);
    }
    utstring_free(raw_resolver_file);

    fprintf(opam_resolver, ";; CAVEAT: may contain duplicate entries\n");

    /* fprintf(opam_resolver, "("); */

    /* write predefined opam pkg mappings */
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs", "@rules_ocaml//cfg/compiler-libs");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.common", "@rules_ocaml//cfg/compiler-libs/common");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.bytecomp", "@rules_ocaml//cfg/compiler-libs/bytecomp");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.optcomp", "@rules_ocaml//cfg/compiler-libs/optcomp");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.toplevel", "@rules_ocaml//cfg/compiler-libs/toplevel");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.native-toplevel",
            "@rules_ocaml//cfg/compiler-libs/native-toplevel");

    fprintf(opam_resolver, "(%s . %s)\n",
            "bigarray", "@rules_ocaml//cfg/bigarray");
    fprintf(opam_resolver, "(%s . %s)\n",
            "dynlink", "@rules_ocaml//cfg/dynlink");
    fprintf(opam_resolver, "(%s . %s)\n",
            "str", "@rules_ocaml//cfg/str");
    fprintf(opam_resolver, "(%s . %s)\n",
            "unix", "@rules_ocaml//cfg/unix");

    fprintf(opam_resolver, "(%s . %s)\n",
            "threads.posix", "@rules_ocaml//cfg/threads");
    fprintf(opam_resolver, "(%s . %s)\n",
            "threads.vm", "@rules_ocaml//cfg/threads");
}

char *get_workspace_name()
{
    /* char *exe; */
    char *result;

    /* exe = "bazel"; */
    /* char *argv[] = { */
    /*     "bazel", "info", */
    /*     "execution_root", */
    /*     NULL */
    /* }; */

    /* int argc = (sizeof(argv) / sizeof(argv[0])) - 1; */
    /* result = spawn_cmd(exe, argc, argv); */
    result = run_cmd("bazel info execution_root", false);
    if (result == NULL) {
        fprintf(stderr, "FAIL: spawn_cmd(bazel info execution_root)\n");
        exit(EXIT_FAILURE);
    } else {
        /* printf("BAZEL EXECUTION ROOT %s\n", result); */
        char *wsname = basename(result);
        /* printf("wsname '%s'\n", wsname); */
        int n = strlen(wsname);
        wsname[n] = '\0'; // remove trailing newline
        /* printf("wsname '%s'\n", wsname); */
        return wsname;
    }
}

/* opam_install -p <pkg>
   init a new opam installation rooted at ./opam, switch '_here',
   and install pkgs
 */
/* EXPORT void opam_remove(char *_package) */
/* { */
/*     if (debug) */
/*         log_debug("opam_remove: %s", _package); */

/*     char *exe; */
/*     int result; */

/*     if (access(HERE_OPAM_ROOT, F_OK) != 0) { */
/*         //FIXME: print error msg */
/*         log_error("OPAM root '" HERE_OPAM_ROOT "' not found."); */
/*         printf("OPAM root '" HERE_OPAM_ROOT "' not found.\n"); */
/*         exit(EXIT_FAILURE); */
/*     } else { */
/*         exe = "opam"; */
/*         char *argv[] = { */
/*             "opam", "remove", */
/*             /\* "-v", *\/ // verbose? "-v" : "", */
/*             "--yes", */
/*             "--cli=2.1", */
/*             "--root=./" HERE_OPAM_ROOT, */
/*             "--switch", HERE_SWITCH_NAME, */
/*             _package, */
/*             NULL */
/*         }; */

/*         if (verbose) */
/*             log_info("Removing package %s from here-switch\n", _package); */
/*         int argc = (sizeof(argv) / sizeof(argv[0])) - 1; */
/*         result = spawn_cmd(exe, argc, argv); */
/*         if (result != 0) { */
/*             fprintf(stderr, "FAIL: spawn_cmd(opam remove --root .opam --switch _here %s)\n", _package); */
/*             exit(EXIT_FAILURE); */
/*         /\* } else { *\/ */
/*         /\*     printf("remove result: %s\n", result); *\/ */
/*         } */
/*     } */
/* } */

void opam_test(void) {
    get_compiler_variants(NULL);
}
