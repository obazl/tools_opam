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
#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>
#endif

#include "utarray.h"
#include "utstring.h"
#endif

#define BUFSZ 4096

#include "log.h"
#include "opam.h"

int errnum;
/* bool local_opam; */

/* global: we write on new_local_repository rule per build file */
FILE *repo_rules_FILE;

#if INTERFACE
#define ROOT_DIRNAME ".opam"
#define OBAZL_OPAM_ROOT OBAZL_ROOT "/opam"
#define HERE_COMPILER_FILE OBAZL_OPAM_ROOT "/here.compiler"
#define HERE_SWITCH_BAZEL_ROOT OBAZL_OPAM_ROOT "/_here"
#define HERE_SWITCH_NAME "_here"
#endif

char *obazl_opam_root = OBAZL_OPAM_ROOT;
char *here_switch_bazel_root = HERE_SWITCH_BAZEL_ROOT;


#if INTERFACE
#define HERE_COMPILER "/here.compiler"
#endif

char * read_here_compiler_file(void)
{
    char *here_compiler = OBAZL_OPAM_ROOT HERE_COMPILER;
    /* printf("reading %s\n", here_compiler); */
    if (access(here_compiler, R_OK) == 0) {
        char buff[512];
        FILE *f = fopen(here_compiler, "r");
        fgets(buff, 512, f);
        /* printf("String read: '%s'\n", buff); */
        int len = strnlen(buff, 512);
        if (buff[len-1] == '\n') { // remove newline
            buff[len - 1] = '\0';
        }
        fclose(f);
        printf("found %s specifying version: %s\n",
               here_compiler, buff);
        return strndup(buff, len);
    }
    return NULL;
}

void init_opam_resolver_raw(char *_opam_switch)
{
    printf("init_opam_resolver_raw, switch: %s\n", _opam_switch);
    /* we're going to write out a scheme file that our dune conversion
       routines can use to resolve opam pkg names to obazl target
       labels. */

    UT_string *raw_resolver_file;
    utstring_new(raw_resolver_file);
    utstring_printf(raw_resolver_file, "%s/%s/%s",
                    OBAZL_OPAM_ROOT,
                    _opam_switch,
                    "opam_resolver_raw.scm");
    /* if (access(raw_resolver_file, R_OK) != 0) return; */

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
            "compiler-libs", "@ocaml//compiler-libs");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.common", "@ocaml//compiler-libs/common");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.bytecomp", "@ocaml//compiler-libs/bytecomp");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.optcomp", "@ocaml//compiler-libs/optcomp");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.toplevel", "@ocaml//compiler-libs/toplevel");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.native-toplevel",
            "@ocaml//compiler-libs/native-toplevel");

    fprintf(opam_resolver, "(%s . %s)\n",
            "bigarray", "@ocaml//bigarray");
    fprintf(opam_resolver, "(%s . %s)\n",
            "dynlink", "@ocaml//dynlink");
    fprintf(opam_resolver, "(%s . %s)\n",
            "str", "@ocaml//str");
    fprintf(opam_resolver, "(%s . %s)\n",
            "unix", "@ocaml//unix");

    fprintf(opam_resolver, "(%s . %s)\n",
            "threads.posix", "@ocaml//threads");
    fprintf(opam_resolver, "(%s . %s)\n",
            "threads.vm", "@ocaml//threads");
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
    result = run_cmd("bazel info execution_root");
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

/* opam_export */
EXPORT void opam_export(char *manifest)
{
    log_debug("opam_export: %s", manifest);

    UT_string *manifest_name;
    utstring_new(manifest_name);
    if (manifest) {
        utstring_printf(manifest_name, "%s", manifest);
    } else {
        /* char *workspace = get_workspace_name(); */
        /* printf("wsn: '%s'\n", workspace); */
        /* utstring_printf(manifest_name, "%s.opam.manifest", workspace); */
        utstring_printf(manifest_name, "%s/here.packages",
                        obazl_opam_root);
    }

    char *exe;
    int result;

    if (access(ROOT_DIRNAME, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM root '" ROOT_DIRNAME "' not found.");
        printf("Project-local OPAM root '" ROOT_DIRNAME "' not found.\n");
        exit(EXIT_FAILURE);
    } else {
        exe = "opam";
        char *argv[] = {
            "opam", "switch",
            "export",
            "--cli=2.1",
            "--root=./" ROOT_DIRNAME,
            "--switch", HERE_SWITCH_NAME,
            "--freeze",
            "--full",
            utstring_body(manifest_name),
            NULL
        };
        utstring_body(manifest_name);
        printf("Exporting %s\n", utstring_body(manifest_name));
        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: spawn_cmd(opam var --root .opam --switch _here)\n");
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("export result: %s\n", result); */
        }
    }
    // chmod(OBAZL_OPAM_ROOT "/here.compiler", S_IRUSR|S_IRGRP|S_IROTH);
}

/* opam_install -p <pkg>
   init a new opam installation rooted at ./opam, switch '_here',
   and install pkgs
 */
EXPORT void opam_install(char *_package)
{
    log_debug("opam_install %s", _package);

    char *exe;
    int result;

    if (access(ROOT_DIRNAME, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM root '" ROOT_DIRNAME "' not found.");
        printf("Project-local OPAM root '" ROOT_DIRNAME "' not found.\n");
        exit(EXIT_FAILURE);
    } else {

        exe = "opam";
        char *argv[] = {
            "opam", "install",
            /* "-v", */
            "--yes",
            "--cli=2.1",
            "--root=./" ROOT_DIRNAME,
            "--switch", HERE_SWITCH_NAME,
            "--require-checksums",
            _package,
            NULL
        };

        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        if (verbose)
            printf("Installing OPAM package: %s\n", _package);
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: spawn_cmd(opam install --root .opam --switch _here --require-checksums %s)\n", _package);
            exit(EXIT_FAILURE);
        } else {
            printf("install result: %s\n", result);
        }
    }
}

EXPORT void opam_remove(char *_package)
{
    log_debug("opam_remove: %s", _package);

    char *exe;
    int result;

    if (access(ROOT_DIRNAME, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM root '" ROOT_DIRNAME "' not found.");
        printf("OPAM root '" ROOT_DIRNAME "' not found.\n");
        exit(EXIT_FAILURE);
    } else {

        exe = "opam";
        char *argv[] = {
            "opam", "remove",
            /* "-v", */ // verbose? "-v" : "",
            "--yes",
            "--cli=2.1",
            "--root=./" ROOT_DIRNAME,
            "--switch", HERE_SWITCH_NAME,
            _package,
            NULL
        };

        /* printf("Removing package %s\n", _package); */
        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: spawn_cmd(opam remove --root .opam --switch _here %s)\n", _package);
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("remove result: %s\n", result); */
        }
    }
}

