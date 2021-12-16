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
#include "opam_config.h"

int errnum;
/* bool local_opam; */

/* global: we write on new_local_repository rule per build file */
FILE *repo_rules_FILE;

UT_array *opam_packages;

char *bzl_switch_root; // outdir for build files etc. mirrors opam switch

/*
  default: configure here-switch - build rules ocaml_import from here-switch
  alternatives:  -s <switch> configures for sys opam switch

  assumption: opam_configure() configures @ocaml.toolchain to use same switch

 */
EXPORT void opam_config(char *_opam_switch) //, char *bzl_switch_root)
{
    log_info("opam_config switch: %s", _opam_switch);

    char *opam_switch;

    UT_string *switch_bin;
    utstring_new(switch_bin);

    UT_string *switch_lib;
    utstring_new(switch_lib);

    char *cmd, *result;

    if (_opam_switch != NULL) {
        // configure non-here-switch
        printf("using current switch of sys opam\n");
        log_info("using current switch of sys opam");

        cmd = "opam var switch";
        opam_switch = run_cmd(cmd);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(%s)\n", cmd);
            exit(EXIT_FAILURE);
        }
        /* } else */
        /*     opam_switch = strndup(result, PATH_MAX); */

        cmd = "opam var bin";
        result = NULL;
        result = run_cmd(cmd);
        if (result == NULL) {
            log_fatal("FAIL: run_cmd(%s)\n", cmd);
            exit(EXIT_FAILURE);
        } else
            utstring_printf(switch_bin, "%s", result);

        cmd = "opam var lib";
        result = NULL;
        result = run_cmd(cmd);
        if (result == NULL) {
            log_fatal("FAIL: run_cmd(%s)\n", cmd);
            exit(EXIT_FAILURE);
        } else
            utstring_printf(switch_lib, "%s", result);

        //TODO: set bzl_switch_root
        bzl_switch_root = OBAZL_OPAM_ROOT "/4.10";

    } else {
        // configure here-switch
        printf("configuring here-switch\n");
        /* opam_switch = "obazl"; // strndup(_opam_switch, PATH_MAX); */
        /* utstring_printf(switch_bin, "./.opam/%s/bin", opam_switch); */
        /* utstring_printf(switch_lib, "./.opam/%s/lib", opam_switch); */

        cmd = "opam var --root .opam switch";
        errno = 0;
        opam_switch = run_cmd(cmd);
        if (opam_switch == NULL) {
            perror(cmd);
            /* fprintf(stderr, "Fail: run_cmd(%s)\n", cmd); */
            exit(EXIT_FAILURE);
        }
        printf("found here-switch %s\n", opam_switch);

        cmd = "opam var bin";
        result = NULL;
        result = run_cmd(cmd);
        if (result == NULL) {
            log_fatal("FAIL: run_cmd(%s)\n", cmd);
            exit(EXIT_FAILURE);
        } else
            utstring_printf(switch_bin, "%s", result);

        cmd = "opam var lib";
        result = NULL;
        result = run_cmd(cmd);
        if (result == NULL) {
            log_fatal("FAIL: run_cmd(%s)\n", cmd);
            exit(EXIT_FAILURE);
        } else
            utstring_printf(switch_lib, "%s", result);

        bzl_switch_root = HERE_SWITCH_BAZEL_ROOT;
    }

    printf("switch: %s\n", opam_switch);
    printf("switch bin: %s\n", utstring_body(switch_bin));
    printf("switch lib: %s\n", utstring_body(switch_lib));
    printf("bzl_switch_root: %s\n", bzl_switch_root);
    /* make sure output dir exists */
    mkdir_r(bzl_switch_root);

    utarray_new(opam_packages, &ut_str_icd);

    init_opam_resolver_raw(opam_switch);
    /* _initialize_skipped_pkg_list(); */

    /* first make hereswitch outdir a Bazel package */
    UT_string *bzl_switch_buildfile;
    utstring_new(bzl_switch_buildfile);
    utstring_printf(bzl_switch_buildfile, "%s/BUILD.bazel", bzl_switch_root);
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
    UT_string *repo_rules_filename = NULL;
    utstring_new(repo_rules_filename);
    utstring_printf(repo_rules_filename,
                    "%s/opam_repos.bzl",
                    bzl_switch_root);
    log_debug("repo_rules_filename: %s",
              utstring_body(repo_rules_filename));

    repo_rules_FILE = fopen(utstring_body(repo_rules_filename), "w");
    if (repo_rules_FILE == NULL) {
        perror(utstring_body(repo_rules_filename));
        exit(EXIT_FAILURE);
    }
    utstring_free(repo_rules_filename);
    fprintf(repo_rules_FILE, "load(\"@rules_ocaml//ocaml:rules.bzl\",\n");
    fprintf(repo_rules_FILE, "     \"new_local_pkg_repository\")\n");
    fprintf(repo_rules_FILE, "\n");
    fprintf(repo_rules_FILE, "def fetch():\n");

    /* if (_opam_switch == NULL) { */
    /* if (local_opam) { */
    /* } else { */
    /* } */

    /* now link srcs */
    UT_string *bzl_bin_link;
    utstring_new(bzl_bin_link);
    utstring_printf(bzl_bin_link, "%s/bin", bzl_switch_root);

    UT_string *bzl_lib_link;
    utstring_new(bzl_lib_link);
    utstring_printf(bzl_lib_link, "%s/_lib", bzl_switch_root);

    log_debug("bzl_bin_link: %s", utstring_body(bzl_bin_link));
    log_debug("bzl_lib_link: %s", utstring_body(bzl_lib_link));


    /* link to opam bin, lib dirs. we could do this in starlark, but
       then we would not be able to test independently. */
    /* rc = symlink(utstring_body(switch_bin), utstring_body(bzl_bin_link)); */
    /* if (rc != 0) { */
    /*     errnum = errno; */
    /*     if (errnum != EEXIST) { */
    /*         perror(utstring_body(bzl_bin_link)); */
    /*         log_error("symlink failure for %s -> %s\n", utstring_body(switch_bin), utstring_body(bzl_bin_link)); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /* } */

    /* rc = symlink(utstring_body(switch_lib), utstring_body(bzl_lib_link)); */
    /* if (rc != 0) { */
    /*     errnum = errno; */
    /*     if (errnum != EEXIST) { */
    /*         perror(utstring_body(bzl_lib_link)); */
    /*         log_error("symlink failure for %s -> %s\n", utstring_body(switch_lib), utstring_body(bzl_lib_link)); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /* } */

    /*  now set output paths (in @ocaml) */
    mkdir_r(bzl_switch_root); /* make sure bzl_switch_root exists */
    UT_string *bzl_bin;
    utstring_new(bzl_bin);
    utstring_printf(bzl_bin, "%s/bin", bzl_switch_root);

    UT_string *bzl_lib;
    utstring_new(bzl_lib);
    utstring_printf(bzl_lib, "%s/buildfiles", bzl_switch_root);

    log_debug("bzl_bin: %s", utstring_body(bzl_bin));
    log_debug("bzl_lib: %s", utstring_body(bzl_lib));

    // FIXME: always convert everything. otherwise we have to follow
    // the deps to make sure they are all converted.
    // (for dev/test, retain ability to do just one dir)
    if (utarray_len(opam_packages) == 0) {
        meta_walk(utstring_body(switch_lib),
                  bzl_switch_root,
                  false,      /* link files? */
                  // "META",     /* file_to_handle */
                  handle_lib_meta); /* callback */
    } else {
        /* WARNING: only works for top-level pkgs */
        log_debug("converting listed opam pkgs in %s",
                  utstring_body(switch_lib));
        UT_string *s;
        utstring_new(s);
        char **a_pkg = NULL;
        /* log_trace("%*spkgs:", indent, sp); */
        while ( (a_pkg=(char **)utarray_next(opam_packages, a_pkg))) {
            utstring_clear(s);
            utstring_concat(s, switch_lib);
            utstring_printf(s, "/%s/%s", *a_pkg, "META");
            /* log_debug("src root: %s", utstring_body(s)); */
            /* log_trace("%*s'%s'", delta+indent, sp, *a_pkg); */
            if ( ! access(utstring_body(s), R_OK) ) {
                /* log_debug("FOUND: %s", utstring_body(s)); */
                handle_lib_meta(utstring_body(switch_lib),
                                bzl_switch_root,
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
    /* emit_bazel_config_setting_rules(bzl_switch_root); */

    utarray_free(pos_flags);
    utarray_free(neg_flags);
    utstring_free(switch_bin);
    utstring_free(switch_lib);
    utstring_free(bzl_bin);
    utstring_free(bzl_lib);

    /* fprintf(opam_resolver, ")\n"); */
    fclose(opam_resolver);

    fclose(repo_rules_FILE);

    /* _free_skipped_pkg_list(); */
}

