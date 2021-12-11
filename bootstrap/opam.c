#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
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

#include "log.h"
#include "opam.h"

int errnum;
bool local_opam;

/* global: we write on new_local_repository rule per build file */
FILE *repo_rules_FILE;

/* char *rootdir = "buildfiles"; */

UT_array *opam_packages;

char *run_cmd(char *cmd)
{
    static char buf[PATH_MAX];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return NULL;
    }

    while (fgets(buf, sizeof buf, fp) != NULL) {
        /* printf("SWITCH: %s\n", buf); */
        buf[strcspn(buf, "\n")] = 0;
    }

    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
        return NULL;
    }
    return buf;
}

void init_opam_resolver_raw()
{
    /* we're going to write out a scheme file that our dune conversion
       routines can use to resolve opam pkg names to obazl target
       labels. */
    //FIXME: config the correct outfile name
    extern FILE *opam_resolver;  /* in emit_build_bazel.c */
    opam_resolver = fopen(".opam.d/opam_resolver_raw.scm", "w");
    if (opam_resolver == NULL) {
        perror("opam_resolver fopen");
        exit(EXIT_FAILURE);
    }

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

/* install_project_opam
   init a new opam installation rooted at ./opam, switch 'obazl',
   and install pkgs
 */
EXPORT void install_project_opam(char *_opam_switch, char *bzlroot)
{
    log_info("opam_install bzlroot: %s", bzlroot);

    utarray_new(opam_packages, &ut_str_icd);

    char *opam_switch;

    UT_string *switch_bin;
    utstring_new(switch_bin);

    UT_string *switch_lib;
    utstring_new(switch_lib);

    char *cmd, *result;
    log_info("using local opam");
    opam_switch = "obazl"; // strndup(_opam_switch, PATH_MAX);
    utstring_printf(switch_bin, "./.opam/%s/bin", opam_switch);
    utstring_printf(switch_lib, "./.opam/%s/lib", opam_switch);

    log_debug("switch: %s", opam_switch);
    log_debug("switch_bin: %s", utstring_body(switch_bin));
    log_debug("switch_lib: %s", utstring_body(switch_lib));

    /* now link srcs */
    /* mkdir_r(bzlroot, "");       /\* make sure bzlroot exists *\/ */
    /* UT_string *bzl_bin_link; */
    /* utstring_new(bzl_bin_link); */
    /* utstring_printf(bzl_bin_link, "%s/bin", bzlroot); */

    /* UT_string *bzl_lib_link; */
    /* utstring_new(bzl_lib_link); */
    /* utstring_printf(bzl_lib_link, "%s/_lib", bzlroot); */
    /* log_debug("bzl_bin_link: %s", utstring_body(bzl_bin_link)); */
    /* log_debug("bzl_lib_link: %s", utstring_body(bzl_lib_link)); */

    /*  now set output paths (in @ocaml) */
    /* mkdir_r(bzlroot, "");       /\* make sure bzlroot exists *\/ */
    /* UT_string *bzl_bin; */
    /* utstring_new(bzl_bin); */
    /* utstring_printf(bzl_bin, "%s/bin", bzlroot); */

    /* UT_string *bzl_lib; */
    /* utstring_new(bzl_lib); */
    /* utstring_printf(bzl_lib, "%s/buildfiles", bzlroot); */

    /* log_debug("bzl_bin: %s", utstring_body(bzl_bin)); */
    /* log_debug("bzl_lib: %s", utstring_body(bzl_lib)); */


    /* utarray_free(pos_flags); */
    /* utarray_free(neg_flags); */
    utstring_free(switch_bin);
    utstring_free(switch_lib);
    /* utstring_free(bzl_bin); */
    /* utstring_free(bzl_lib); */

    /* fclose(repo_rules_FILE); */

    /* _free_skipped_pkg_list(); */
}

EXPORT void opam_ingest(char *_opam_switch, char *bzlroot)
{
    log_info("opam_ingest bzlroot: %s", bzlroot);

    utarray_new(opam_packages, &ut_str_icd);

    init_opam_resolver_raw();
    /* _initialize_skipped_pkg_list(); */

    /* we're going to write one 'new_local_repository' target per
       build file: */
    UT_string *repo_rules_filename = NULL;
    utstring_new(repo_rules_filename);
    utstring_printf(repo_rules_filename, "%s/opam_repos.bzl", bzlroot);
    log_debug("repo_rules_filename: %s",
              utstring_body(repo_rules_filename));

    repo_rules_FILE = fopen(utstring_body(repo_rules_filename), "w");
    if (repo_rules_FILE == NULL) {
        perror(utstring_body(repo_rules_filename));
        exit(EXIT_FAILURE);
    }
    utstring_free(repo_rules_filename);
    fprintf(repo_rules_FILE, "load(\"@obazl_rules_ocaml//ocaml/_repo_rules:new_local_pkg_repository.bzl\",\n");
    fprintf(repo_rules_FILE, "     \"new_local_pkg_repository\")\n");
    fprintf(repo_rules_FILE, "\n");
    fprintf(repo_rules_FILE, "def fetch():\n");

    /* first discover current switch info */
    char *opam_switch;

    UT_string *switch_bin;
    utstring_new(switch_bin);

    UT_string *switch_lib;
    utstring_new(switch_lib);

    char *cmd, *result;
    /* if (_opam_switch == NULL) { */
    if (local_opam) {
        log_info("using local opam");
        opam_switch = "obazl"; // strndup(_opam_switch, PATH_MAX);
        utstring_printf(switch_bin, "./.opam/%s/bin", opam_switch);
        utstring_printf(switch_lib, "./.opam/%s/lib", opam_switch);

    } else {
        log_info("using current switch of sys opam");
        cmd = "opam var switch";

        result = run_cmd(cmd);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(%s)\n", cmd);
        } else
            opam_switch = strndup(result, PATH_MAX);

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

    }

    log_debug("switch: %s", opam_switch);
    log_debug("switch_bin: %s", utstring_body(switch_bin));
    log_debug("switch_lib: %s", utstring_body(switch_lib));

    /* now link srcs */
    mkdir_r(bzlroot, "");       /* make sure bzlroot exists */
    UT_string *bzl_bin_link;
    utstring_new(bzl_bin_link);
    utstring_printf(bzl_bin_link, "%s/bin", bzlroot);

    UT_string *bzl_lib_link;
    utstring_new(bzl_lib_link);
    utstring_printf(bzl_lib_link, "%s/_lib", bzlroot);

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
    mkdir_r(bzlroot, "");       /* make sure bzlroot exists */
    UT_string *bzl_bin;
    utstring_new(bzl_bin);
    utstring_printf(bzl_bin, "%s/bin", bzlroot);

    UT_string *bzl_lib;
    utstring_new(bzl_lib);
    utstring_printf(bzl_lib, "%s/buildfiles", bzlroot);

    log_debug("bzl_bin: %s", utstring_body(bzl_bin));
    log_debug("bzl_lib: %s", utstring_body(bzl_lib));

    // FIXME: always convert everything. otherwise we have to follow
    // the deps to make sure they are all converted.
    // (for dev/test, retain ability to do just one dir)
    if (utarray_len(opam_packages) == 0) {
        meta_walk(utstring_body(switch_lib),
                  bzlroot,
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
                                bzlroot,
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
    /* emit_bazel_config_setting_rules(bzlroot); */

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
