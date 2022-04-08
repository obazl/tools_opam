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
#include "opam_config_xdg.h"

int errnum;
/* bool local_opam; */

/* #define OPAM_LOADER "BOOTSTRAP.bzl" */

/*
  default: configure xdg obazl repos
  alternatives:  -s <switch> configures for sys opam switch

  assumption: opam_configure() configures @ocaml.toolchain to use same switch

 */
EXPORT void opam_config_xdg(char *_opam_switch_name)
{
    if (verbose)
        log_info("config_xdg_repolib -s %s", _opam_switch_name);

    opam_config_init();

    UT_string *cmd;
    utstring_new(cmd);

    char *result = NULL;

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var root");
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
    if (verbose)
        log_info("opam_switch_root: %s", utstring_body(opam_switch_root));

    if (_opam_switch_name == NULL) {
        utstring_renew(cmd);
        utstring_printf(cmd, "opam var switch");
        result = NULL;
        result = run_cmd(utstring_body(cmd));
        if (result == NULL) {
            perror(utstring_body(cmd));
            /* fprintf(stderr, "Fail: run_cmd(%s)\n", cmd); */
            exit(EXIT_FAILURE);
        } else {
            utstring_printf(opam_switch_name, "%s", result);
        }
    } else {
        /* if (_opam_switch_name != NULL) { */
        /* printf("using switch: %s\n", _opam_switch_name); */
        utstring_printf(opam_switch_name, "%s", _opam_switch_name);
    }
    if (verbose)
        log_info("opam_switch_name: %s", utstring_body(opam_switch_name));

    // obtain switch prefix, bin, lib
    utstring_renew(cmd);
    utstring_printf(cmd, "opam var prefix --switch %s",
                    utstring_body(opam_switch_name));
    result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        fprintf(stderr, "FAIL: run_cmd(%s)\n", utstring_body(cmd));
        exit(EXIT_FAILURE);
    } else {
        utstring_printf(opam_switch_pfx, "%s", result);
    }

    if (access(utstring_body(opam_switch_pfx), F_OK) != 0) {
        fprintf(stderr, "Switch %s at %s not found; exiting.\n",
                utstring_body(opam_switch_name),
                utstring_body(opam_switch_pfx));
        utarray_free(pos_flags);
        utarray_free(neg_flags);
        utstring_free(opam_switch_name);
        utstring_free(opam_switch_pfx);
        utstring_free(opam_switch_bin);
        utstring_free(opam_switch_lib);
        /* utstring_free(bzl_bin); */
        /* utstring_free(bzl_lib); */
        fclose(opam_resolver);
        fclose(bootstrap_FILE);
        exit(EXIT_FAILURE);
    }

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var bin --switch %s",
                    utstring_body(opam_switch_name));
    result = NULL;
    result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        log_fatal("FAIL: run_cmd(%s)\n", utstring_body(cmd));
        exit(EXIT_FAILURE);
    } else
        utstring_printf(opam_switch_bin, "%s", result);

    utstring_renew(cmd);
    utstring_printf(cmd, "opam var lib --switch %s",
                    utstring_body(opam_switch_name));
    result = NULL;
    result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        log_fatal("FAIL: run_cmd(%s)\n", utstring_body(cmd));
        exit(EXIT_FAILURE);
    } else
        utstring_printf(opam_switch_lib, "%s", result);

    /* utstring_printf(bzl_switch_pfx, "%s/%s", */
    /*                 xdg_opam_root, */
    /*                 utstring_body(opam_switch_name)); */
    utstring_free(cmd);

    utstring_renew(bzl_switch_pfx);
    utstring_concat(bzl_switch_pfx, xdg_opam_root);
    utstring_printf(bzl_switch_pfx, "/%s", utstring_body(opam_switch_name));

    if (verbose) {
        log_info("opam_switch_root: %s", utstring_body(opam_switch_root));
        log_info("opam_switch_name: %s", utstring_body(opam_switch_name));
        log_info("opam_switch_pfx: %s", utstring_body(opam_switch_pfx));
        log_info("opam_switch_bin: %s", utstring_body(opam_switch_bin));
        log_info("opam_switch_lib: %s", utstring_body(opam_switch_lib));
        log_info("bzl_switch_pfx: %s", utstring_body(bzl_switch_pfx));
    }

    /* make sure output dir exists */
    mkdir_r(utstring_body(bzl_switch_pfx));

    utarray_new(opam_packages, &ut_str_icd);

    /* make xdg outdir a Bazel package */
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

    UT_string *repo_rules_filename = NULL;
    utstring_new(repo_rules_filename);
    utstring_printf(repo_rules_filename,
                    "%s/%s",
                    utstring_body(bzl_switch_pfx),
                    OPAM_BOOTSTRAP);
    log_debug("repo_rules_filename: %s",
              utstring_body(repo_rules_filename));

    bootstrap_FILE = fopen(utstring_body(repo_rules_filename), "w");
    if (bootstrap_FILE == NULL) {
        perror(utstring_body(repo_rules_filename));
        exit(EXIT_FAILURE);
    }
    utstring_free(repo_rules_filename);
    /* fprintf(bootstrap_FILE, "load(\"@rules_ocaml//ocaml:rules.bzl\",\n"); */
    /* fprintf(bootstrap_FILE, "     \"new_local_pkg_repository\")\n"); */
    /* fprintf(bootstrap_FILE, "\n"); */
    fprintf(bootstrap_FILE, "def bootstrap():\n");

    /* now link srcs */
    utstring_new(bzl_bin_link);
    utstring_printf(bzl_bin_link, "%s/bin", utstring_body(bzl_switch_pfx));

    utstring_new(bzl_lib_link);
    utstring_printf(bzl_lib_link, "%s/_lib", utstring_body(bzl_switch_pfx));

    log_debug("bzl_bin_link: %s", utstring_body(bzl_bin_link));
    log_debug("bzl_lib_link: %s", utstring_body(bzl_lib_link));


    /*  now set output paths (in @ocaml) */
    mkdir_r(utstring_body(bzl_switch_pfx));

    utstring_new(bzl_bin);
    utstring_printf(bzl_bin, "%s/bin", utstring_body(bzl_switch_pfx));

    utstring_new(bzl_lib);
    utstring_printf(bzl_lib, "%s", utstring_body(bzl_switch_pfx));

    log_debug("bzl_bin: %s", utstring_body(bzl_bin));
    log_debug("bzl_lib: %s", utstring_body(bzl_lib));

    //step 1: generate @ocaml
    emit_ocaml_repo(utstring_body(opam_switch_name), bootstrap_FILE);

    // step 2: pkgs subdir
    // FIXME: always convert everything. otherwise we have to follow
    // the deps to make sure they are all converted.
    // (for dev/test, retain ability to do just one dir)
    utstring_printf(bzl_switch_pfx, "/pkgs");

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

    /* fprintf(bootstrap_FILE, "    ## end bootstrap()\n\n"); */
    fprintf(bootstrap_FILE,
    "    native.register_toolchains(\"@ocaml//toolchains:ocaml_macos\")\n");
    fprintf(bootstrap_FILE,
    "    native.register_toolchains(\"@ocaml//toolchains:ocaml_linux\")");

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
    utstring_free(opam_switch_bin);
    utstring_free(opam_switch_lib);
    utstring_free(bzl_bin);
    utstring_free(bzl_lib);

    /* utstring_free(bazel_pkg_root); */
    /* utstring_free(build_bazel_file); */

    /* fprintf(opam_resolver, ")\n"); */
    fclose(opam_resolver);

    fclose(bootstrap_FILE);

    /* _free_skipped_pkg_list(); */
}

