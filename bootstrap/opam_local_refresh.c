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
#endif

#define BUFSZ 4096

#include "log.h"
#include "opam_local_refresh.h"

int errnum;
/* bool local_opam; */

void _xopam_set_vars(void)
{
    opam_config_init();

    UT_string *cmd;
    utstring_new(cmd);

    char *result = NULL;

    if (verbose)
        log_info("configuring local coswitch");

    /* utstring_renew(cmd); */
    /* utstring_printf(cmd, "opam var root --root .opam"); */
    /* errno = 0; */
    /* result = NULL; */
    /* result = run_cmd(utstring_body(cmd), false); */
    /* if (result == NULL) { */
    /*     perror(utstring_body(cmd)); */
    /*     /\* fprintf(stderr, "Fail: run_cmd(%s)\n", cmd); *\/ */
    /*     exit(EXIT_FAILURE); */
    /* } else { */
    /*     utstring_printf(opam_switch_root, "%s", result); */
    /* } */

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var switch");
    errno = 0;
    result = NULL;
    result = run_cmd(utstring_body(cmd), false);
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* fprintf(stderr, "Fail: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else {
        utstring_printf(opam_switch_name, "%s", result);
    }
    utstring_renew(cmd);
    utstring_printf(cmd, "opam var prefix");
    errno = 0;
    result = NULL;
    result = run_cmd(utstring_body(cmd), false);
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* fprintf(stderr, "Fail: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else {
        utstring_printf(opam_switch_pfx, "%s", result);
    }

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var bin");
    /* cmd = "opam var bin"; */
    result = NULL;
    result = run_cmd(utstring_body(cmd), false);
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* log_fatal("FAIL: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else
        utstring_printf(opam_switch_bin, "%s", result);

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var lib");
    /* cmd = "opam var lib"; */
    result = NULL;
    result = run_cmd(utstring_body(cmd), false);
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* log_fatal("FAIL: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else
        utstring_printf(opam_switch_lib, "%s", result);

    utstring_printf(bzl_switch_pfx, "%s", LOCAL_COSWITCH_ROOT);

    if (verbose) {
        log_info("opam_switch_root: %s", utstring_body(opam_switch_root));
        log_info("opam_switch_name: %s", utstring_body(opam_switch_name));
        log_info("opam_switch_pfx: %s", utstring_body(opam_switch_pfx));
        log_info("opam_switch_bin: %s", utstring_body(opam_switch_bin));
        log_info("opam_switch_lib: %s", utstring_body(opam_switch_lib));
        log_info("bzl_switch_pfx: %s", utstring_body(bzl_switch_pfx));
    }
}

EXPORT void opam_local_refresh(void) // char *_opam_switch_name)
{
    log_info("opam_local_refresh entry"); // -s %s", _opam_switch_name);

    if (access(LOCAL_SWITCH_DIR, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM local switch '" LOCAL_SWITCH_DIR "' not found.");
        printf(RED "ERROR: " CRESET "OPAM local switch '" LOCAL_SWITCH_DIR "' not found.\n");
        exit(EXIT_FAILURE);
    }

    _xopam_set_vars();

    /* make sure output dir exists */
    if (!dry_run)
        mkdir_r(utstring_body(bzl_switch_pfx));

    utarray_new(opam_packages, &ut_str_icd);

    /* we're going to write one 'new_local_repository' target per
       build file: */
    UT_string *bootstrap_filename = NULL;
    utstring_new(bootstrap_filename);
    utstring_printf(bootstrap_filename,
                    "%s/%s",
                    /* utstring_body(bzl_switch_pfx), */
                    LOCAL_COSWITCH_ROOT,
                    OPAM_BOOTSTRAP);

    if (debug) {
        log_debug("writing BOOTSTRAP.bzl: %s",
                  utstring_body(bootstrap_filename));
        printf("writing BOOTSTRAP.bzl: %s\n",
               utstring_body(bootstrap_filename));
    }

    if (dry_run) {
        //FIXME: support dry run for this?
    }
    bootstrap_FILE = fopen(utstring_body(bootstrap_filename), "w");
    if (bootstrap_FILE == NULL) {
        perror(utstring_body(bootstrap_filename));
        exit(EXIT_FAILURE);
    }

    fprintf(bootstrap_FILE, "# generated file - DO NOT EDIT\n");

    fprintf(bootstrap_FILE, "def bootstrap():\n");

    //step 1: generate @ocaml
    emit_ocaml_workspace(utstring_body(opam_switch_name), bootstrap_FILE);
    /* emit_ocaml_buildfile(utstring_body(opam_switch_name)); */

    // FIXME: always convert everything. otherwise we have to follow
    // the deps to make sure they are all converted.
    // (for dev/test, retain ability to do just one dir)
    utstring_printf(bzl_switch_pfx, COSWITCH_LIB); // "/pkgs");

    if (debug) {
        printf("opam_switch_lib: %s\n", utstring_body(opam_switch_lib));
        printf("bzl_switch_pfx: %s\n", utstring_body(bzl_switch_pfx));
    }
    if (utarray_len(opam_packages) == 0) {
        /* printf("WALKING, opam_switch_lib: %s\n", */
        /*        utstring_body(opam_switch_lib)); */
        meta_walk(utstring_body(opam_switch_lib),
                  utstring_body(bzl_switch_pfx),
                  false,      /* link files? */
                  // "META",     /* file_to_handle */
                  handle_lib_meta); /* callback */

        /* now @ocaml with toolchain & core pkgs, @rules_ocaml//cfg/dynlink,
           @rules_ocaml//cfg/str, etc. */
        log_trace("finished meta_walk");
    } else {
        /* WARNING: only works for top-level pkgs */
        /* log_debug("converting listed opam pkgs in %s", */
        /*           utstring_body(opam_switch_lib)); */
        UT_string *s;
        utstring_new(s);
        char **a_pkg = NULL;
        /* log_trace("%*spkgs:", indent, sp); */
        while ( (a_pkg=(char **)utarray_next(opam_packages, a_pkg))) {
            utstring_clear(s);
            utstring_concat(s, opam_switch_lib);
            utstring_printf(s, "/%s/%s", *a_pkg, "META");
            /* log_debug("src root: %s", utstring_body(s)); */
            /* log_trace("%*s'%s'", delta+indent, sp, *a_pkg); */
            if ( ! access(utstring_body(s), R_OK) ) {
                /* log_debug("FOUND: %s", utstring_body(s)); */
                handle_lib_meta(utstring_body(opam_switch_lib),
                                utstring_body(bzl_switch_pfx),
                                /* obzl_meta_package_name(pkg), */
                                *a_pkg,
                                "META");
            } else {
                log_fatal("NOT found: %s", utstring_body(s));
                exit(EXIT_FAILURE);
            }
        }
        utstring_free(s);
    }

#ifdef DEBUG_TRACE
    /* FIXME: obsolete code */
    char **p;
    p = NULL;
    while ( (p=(char**)utarray_next(pos_flags, p))) {
        log_debug("pos_flag: %s", *p);
    }
    p = NULL;
    while ( (p=(char**)utarray_next(neg_flags, p))) {
        log_debug("neg_flag: %s", *p);
    }
#endif
    /* emit_bazel_config_setting_rules(bzl_switch_pfx); */

    /* write_local_compiler_file(NULL); /\* local.compiler, w/version string *\/ */

    utarray_free(pos_flags);
    utarray_free(neg_flags);

    /* utstring_free(opam_switch_name); */
    /* utstring_free(opam_switch_pfx); */
    /* utstring_free(opam_switch_bin); */
    /* utstring_free(opam_switch_lib); */
    opam_config_free();
    /* utstring_free(bzl_bin); */
    /* utstring_free(bzl_lib); */

    /* utstring_free(bazel_pkg_root); */
    /* utstring_free(build_bazel_file); */

    /* fprintf(opam_resolver, ")\n"); */
    fclose(opam_resolver);

    fprintf(bootstrap_FILE,
    "    native.register_toolchains(\"@ocaml//toolchains:ocaml_macos\")\n");
    fprintf(bootstrap_FILE,
    "    native.register_toolchains(\"@ocaml//toolchains:ocaml_linux\")\n");


    fclose(bootstrap_FILE);

    if (verbose || dry_run)
        printf("Wrote bootstrap file: %s\n",
               utstring_body(bootstrap_filename));

    /* do not set local as effective coswitch? */
    /* write_local_coswitch_file(); */

    utstring_free(bootstrap_filename);

    log_info("opam_local_refresh exit");
}
