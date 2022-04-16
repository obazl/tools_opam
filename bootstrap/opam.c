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

/* global: we write on local_repository rule per build file */
FILE *repo_rules_FILE;

#if INTERFACE
#define HERE_OPAM_ROOT ".opam"
#define HERE_OBAZL_ROOT OBAZL_ROOT "/opam"
#define HERE_COMPILER_FILE HERE_OBAZL_ROOT "/here.compiler"
//FIXME: rename, HERE_OBAZL_OPAM_WSS?
#define HERE_SWITCH_BAZEL_ROOT HERE_OBAZL_ROOT "/here"
#define HERE_OBAZL_OPAM_WSS_OCAML HERE_OBAZL_ROOT "/here/ocaml"
#define COSWITCH_LIB "/lib"
#define HERE_OBAZL_OPAM_WSS_PKGS HERE_SWITCH_BAZEL_ROOT COSWITCH_LIB
//"/here/pkgs"
#define HERE_OBAZL_OPAM_WSS_STUBLIBS HERE_OBAZL_ROOT "/here/stublibs"
#define HERE_SWITCH_NAME "here"

#define OPAM_SRC_ROOT HERE_OPAM_ROOT "/" HERE_SWITCH_NAME "/.opam-switch/sources"

#endif

char *here_obazl_root = HERE_OBAZL_ROOT; // only for here-switch!!!
char *here_switch_bazel_root = HERE_SWITCH_BAZEL_ROOT;


#if INTERFACE
#define HERE_COMPILER "/here.compiler"
#endif

char *read_here_compiler_file(void)
{
    char *here_compiler_file = HERE_OBAZL_ROOT HERE_COMPILER;
    /* printf("reading %s\n", here_compiler_file); */
    if (access(here_compiler_file, R_OK) == 0) {
        char here_version[512];
        FILE *f = fopen(here_compiler_file, "r");
        fgets(here_version, 512, f);
        /* printf("String read: '%s'\n", here_version); */
        int len = strnlen(here_version, 512);
        if (here_version[len-1] == '\n') { // remove newline
            here_version[len - 1] = '\0';
        }
        fclose(f);
        /* printf("found %s specifying version: %s\n", */
        /*        here_compiler_file, here_version); */
        printf("Your here switch is configured to use compiler version: %s"
               " (specified in %s)\n",
               here_version, here_compiler_file);

        printf("Reconfigure using with same version?"
               " (if no, you will be prompted for a different version)\n"
               "[Yn] ");

        bool use_current = false;
        char ok[2];
        while(1) {
            memset(ok, '\0', 2);
            fflush(stdin);
            fgets(ok, 2, stdin);
            /* printf("ok[0]: %c\n", ok[0]); */
            if (ok[0] == '\n') {
                use_current = true;
                fflush(stdin);
                break;
            } else {
                if ( (ok[0] == 'y') || (ok[0] == 'Y') ) {
                    /* printf("Using same version y\n"); */
                    use_current = true;
                    break;
                } else {
                    if ( (ok[0] == 'n') || (ok[0] == 'N') ) {
                        use_current = false;
                        break;
                    } else {
                        fprintf(stdout,
                                "Please enter y or n\n");
                        fprintf(stdout,
                                "Configure here-switch with compiler version %s? [Yn] ",
                                here_version);
                    }
                }
            }
        }

        if (use_current) {
            return strndup(here_version, len);
        } else {
            return NULL;
        }
    }
    return NULL;
}

void init_opam_resolver_raw(char *_opam_switch_name)
{
    printf("init_opam_resolver_raw, switch: %s\n", _opam_switch_name);
    /* we're going to write out a scheme file that our dune conversion
       routines can use to resolve opam pkg names to obazl target
       labels. */

    UT_string *raw_resolver_file;
    utstring_new(raw_resolver_file);
    utstring_printf(raw_resolver_file, "%s/%s/%s",
                    HERE_OBAZL_ROOT,
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
                        here_obazl_root);
    }

    char *exe;
    int result;

    if (access(HERE_OPAM_ROOT, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM root '" HERE_OPAM_ROOT "' not found.");
        printf("Project-local OPAM root '" HERE_OPAM_ROOT "' not found.\n");
        exit(EXIT_FAILURE);
    } else {
        exe = "opam";
        char *argv[] = {
            "opam", "switch",
            "export",
            "--cli=2.1",
            "--root=./" HERE_OPAM_ROOT,
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
    // chmod(HERE_OBAZL_ROOT "/here.compiler", S_IRUSR|S_IRGRP|S_IROTH);
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

    if (access(HERE_OPAM_ROOT, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM root '" HERE_OPAM_ROOT "' not found.");
        printf("Project-local OPAM root '" HERE_OPAM_ROOT "' not found.\n");
        exit(EXIT_FAILURE);
    } else {

        exe = "opam";
        char *argv[] = {
            "opam", "install",
            /* "-v", */
            "--yes",
            "--cli=2.1",
            "--root=./" HERE_OPAM_ROOT,
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
        /* } else { */
        /*     printf("install result: %s\n", result); */
        }
    }
}

EXPORT void opam_remove(char *_package)
{
    if (debug)
        log_debug("opam_remove: %s", _package);

    char *exe;
    int result;

    if (access(HERE_OPAM_ROOT, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM root '" HERE_OPAM_ROOT "' not found.");
        printf("OPAM root '" HERE_OPAM_ROOT "' not found.\n");
        exit(EXIT_FAILURE);
    } else {
        exe = "opam";
        char *argv[] = {
            "opam", "remove",
            /* "-v", */ // verbose? "-v" : "",
            "--yes",
            "--cli=2.1",
            "--root=./" HERE_OPAM_ROOT,
            "--switch", HERE_SWITCH_NAME,
            _package,
            NULL
        };

        if (verbose)
            log_info("Removing package %s from here-switch\n", _package);
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

