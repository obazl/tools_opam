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
#include "opam_config_here.h"

int errnum;
/* bool local_opam; */

void _opam_set_vars(void)
{
    opam_config_init();

    UT_string *cmd;
    utstring_new(cmd);

    char *result = NULL;

    if (verbose)
        log_info("configuring here-switch");

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var root --root .opam");
    errno = 0;
    result = NULL;
    result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* fprintf(stderr, "Fail: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else {
        utstring_printf(opam_switch_root, "%s", result);
    }

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var switch --root .opam");
    errno = 0;
    result = NULL;
    result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* fprintf(stderr, "Fail: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else {
        utstring_printf(opam_switch_name, "%s", result);
    }
    //FIXME: verify switch name == "here"
    /* printf("found here-switch %s\n", utstring_body(opam_switch_name)); */

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var prefix --root .opam --switch %s",
                    utstring_body(opam_switch_name));
    errno = 0;
    result = NULL;
    result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* fprintf(stderr, "Fail: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else {
        utstring_printf(opam_switch_pfx, "%s", result);
    }

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var bin --root .opam --switch %s",
                    utstring_body(opam_switch_name));
    /* cmd = "opam var bin"; */
    result = NULL;
    result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* log_fatal("FAIL: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else
        utstring_printf(opam_switch_bin, "%s", result);

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var lib --root .opam --switch %s",
                    utstring_body(opam_switch_name));
    /* cmd = "opam var lib"; */
    result = NULL;
    result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        perror(utstring_body(cmd));
        /* log_fatal("FAIL: run_cmd(%s)\n", cmd); */
        exit(EXIT_FAILURE);
    } else
        utstring_printf(opam_switch_lib, "%s", result);

    utstring_printf(bzl_switch_pfx, "%s", HERE_SWITCH_BAZEL_ROOT);

    if (verbose) {
        log_info("opam_switch_root: %s", utstring_body(opam_switch_root));
        log_info("opam_switch_name: %s", utstring_body(opam_switch_name));
        log_info("opam_switch_pfx: %s", utstring_body(opam_switch_pfx));
        log_info("opam_switch_bin: %s", utstring_body(opam_switch_bin));
        log_info("opam_switch_lib: %s", utstring_body(opam_switch_lib));
        log_info("bzl_switch_pfx: %s", utstring_body(bzl_switch_pfx));
    }
}

/* #define OPAM_LOADER "BOOTSTRAP.bzl" */
/* global: we write one local_repository rule per build file */
/* FILE *bootstrap_FILE; */

EXPORT void opam_config_here(void) // char *_opam_switch_name)
{
    log_info("opam_config_here entry"); // -s %s", _opam_switch_name);

    _opam_set_vars();

    /* make sure output dir exists */
    mkdir_r(utstring_body(bzl_switch_pfx));

    utarray_new(opam_packages, &ut_str_icd);

    /* first make hereswitch outdir a Bazel package */
    UT_string *bzl_switch_buildfile;
    utstring_new(bzl_switch_buildfile);
    utstring_printf(bzl_switch_buildfile,
                    "%s/BUILD.bazel", utstring_body(bzl_switch_pfx));
    if (access(utstring_body(bzl_switch_buildfile), R_OK) != 0) {
        FILE *f = fopen(utstring_body(bzl_switch_buildfile), "w");
        if (f == NULL) {
            perror(utstring_body(bzl_switch_buildfile));
            exit(EXIT_FAILURE);
        }
        fprintf(f, "## do not remove\n");
        fclose(f);
    }

    /* we're going to write one 'new_local_repository' target per
       build file: */
    UT_string *bootstrap_filename = NULL;
    utstring_new(bootstrap_filename);
    utstring_printf(bootstrap_filename,
                    "%s/%s",
                    utstring_body(bzl_switch_pfx),
                    OPAM_BOOTSTRAP);
    log_debug("bootstrap_filename: %s",
              utstring_body(bootstrap_filename));

    bootstrap_FILE = fopen(utstring_body(bootstrap_filename), "w");
    if (bootstrap_FILE == NULL) {
        perror(utstring_body(bootstrap_filename));
        exit(EXIT_FAILURE);
    }
    /* printf("opened bootstrap file: %s\n", */
    /*        utstring_body(bootstrap_filename)); */
    utstring_free(bootstrap_filename);

    /* fprintf(bootstrap_FILE, "load(\"@rules_ocaml//ocaml:rules.bzl\",\n"); */
    /* fprintf(bootstrap_FILE, "     \"new_local_pkg_repository\")\n"); */
    /* fprintf(bootstrap_FILE, "\n"); */
    fprintf(bootstrap_FILE, "def bootstrap():\n");

    /* if (_opam_switch_name == NULL) { */
    /* if (local_opam) { */
    /* } else { */
    /* } */

    /* now link srcs */
    UT_string *bzl_bin_link;
    utstring_new(bzl_bin_link);
    utstring_printf(bzl_bin_link, "%s/bin", utstring_body(bzl_switch_pfx));

    UT_string *bzl_lib_link;
    utstring_new(bzl_lib_link);
    utstring_printf(bzl_lib_link, "%s/_lib", utstring_body(bzl_switch_pfx));

    log_debug("bzl_bin_link: %s", utstring_body(bzl_bin_link));
    log_debug("bzl_lib_link: %s", utstring_body(bzl_lib_link));

    /*  now set output paths (in @ocaml) */
    mkdir_r(utstring_body(bzl_switch_pfx)); /* make sure bzl_switch_pfx exists */
    UT_string *bzl_bin;
    utstring_new(bzl_bin);
    utstring_printf(bzl_bin, "%s/bin", utstring_body(bzl_switch_pfx));

    UT_string *bzl_lib;
    utstring_new(bzl_lib);
    utstring_printf(bzl_lib, "%s", utstring_body(bzl_switch_pfx));

    log_debug("bzl_bin: %s", utstring_body(bzl_bin));
    log_debug("bzl_lib: %s", utstring_body(bzl_lib));

    //step 1: generate @ocaml
    emit_ocaml_repo(utstring_body(opam_switch_name), bootstrap_FILE);
    /* emit_ocaml_buildfile(utstring_body(opam_switch_name)); */

    // FIXME: always convert everything. otherwise we have to follow
    // the deps to make sure they are all converted.
    // (for dev/test, retain ability to do just one dir)
    utstring_printf(bzl_switch_pfx, "/pkgs");

    /* printf("opam_switch_lib: %s\n", utstring_body(opam_switch_lib)); */
    /* printf("bzl_switch_pfx: %s\n", utstring_body(bzl_switch_pfx)); */
    utstring_printf(bzl_switch_pfx, COSWITCH_LIB); // "/pkgs");

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

    } else {
        /* WARNING: only works for top-level pkgs */
        log_debug("converting listed opam pkgs in %s",
                  utstring_body(opam_switch_lib));
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

    utarray_free(pos_flags);
    utarray_free(neg_flags);
    /* utstring_free(opam_switch_name); */
    /* utstring_free(opam_switch_pfx); */
    /* utstring_free(opam_switch_bin); */
    /* utstring_free(opam_switch_lib); */
    opam_config_free();
    utstring_free(bzl_bin);
    utstring_free(bzl_lib);

    /* utstring_free(bazel_pkg_root); */
    /* utstring_free(build_bazel_file); */

    /* fprintf(opam_resolver, ")\n"); */
    fclose(opam_resolver);

    fclose(bootstrap_FILE);

    /* _free_skipped_pkg_list(); */
}

