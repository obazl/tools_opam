#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>             /* access */

/* #if EXPORT_INTERFACE */
#include "libs7.h"
/* #endif */

#include "utarray.h"
#include "utstring.h"

#include "liblogc.h"

#include "emit_pkg_bindir.h"

s7_scheme *s7;

#define DEBUG_LEVEL coswitch_debug
extern int  DEBUG_LEVEL;
#define TRACE_FLAG coswitch_trace
extern bool TRACE_FLAG;
#define S7_DEBUG_LEVEL libs7_debug
extern int  libs7_debug;
extern bool libs7_trace;

/* extern UT_string *opam_switch_lib; */

#if defined(PROFILE_dev)
/* static char *tostr1 = NULL; */
/* LOCAL char *tostr2 = NULL; */
#define TO_STR(x) s7_object_to_c_string(s7, x)
/* #define LOG_S7_DEBUG(lvl, msg, obj) (({(coswitch_debug>lvl) ((tostr1 = TO_STR(obj)), (fprintf(stderr, GRN " S7: " CRESET "%s:%d " #msg ": %s\n", __FILE__, __LINE__, tostr1)), (free(tostr1)))})) */
/* #else */
/* #define LOG_S7_DEBUG(lvl, msg, obj) */
#endif

/* **************************************************************** */
/* s7_scheme *s7;                  /\* GLOBAL s7 *\/ */
/* /\* const char *errmsg = NULL; *\/ */

/* static int level = 0; */
extern int spfactor;
extern char *sp;

#if defined(PROFILE_dev)
extern int indent;
extern int delta;
#endif

/* bool stdlib_root = false; */

/* char *buildfile_prefix = "@//" HERE_OBAZL_ROOT "/buildfiles"; */
/* was: "@//.opam.d/buildfiles"; */

/* long *KPM_TABLE; */

/* FILE *opam_resolver; */

extern UT_string *repo_name;

UT_array *_get_pkg_stublibs(s7_scheme *s7, char *pkg, void *_stanzas)
{
    TRACE_ENTRY;
    (void)pkg;

    s7_pointer stanzas = (s7_pointer) _stanzas;
/*     LOG_DEBUG(0, "stanzas: %s", TO_STR(stanzas)); */

    UT_string *outpath;
    UT_string *opam_bin;
    utstring_new(outpath);
    utstring_new(opam_bin);

    UT_array *stubs;
    utarray_new(stubs, &ut_str_icd);

    s7_pointer iter, stublib_file;

    s7_pointer e = s7_inlet(s7,
                            s7_list(s7, 1,
                                    s7_cons(s7,
                                            s7_make_symbol(s7, "stanzas"),
                                            stanzas)));

    char * stublibs_sexp =
        "(let ((files (assoc 'files (cdr stanzas))))"
        "  (if files"
        "      (let ((bin (assoc 'stublibs (cdr files))))"
        "          (if bin (cadr bin)))))";

    s7_pointer stublibs = s7_eval_c_string_with_environment(s7, stublibs_sexp, e);

    if (stublibs == s7_unspecified(s7)) {
        /* LOG_DEBUG(0, "NO STUBLIBS"); */
        return stubs;
    }

        /* LOG_DEBUG(0, "Pkg: %s", utstring_body(dune_pkg_file)); */
        /// LOG_S7_DEBUG(0, "STUBLIBS", stublibs);


    /* result is list of stublibs installed in $PREFIX/bin */
    /* if (s7_is_list(s7, stublibs)) { */
    /*     if (verbose) { */
    /*         LOG_INFO(0, GRN "%s stublibs:" CRESET " %s", */
    /*                  pkg, */
    /*                  /\* " for %s: %s\n", *\/ */
    /*                  /\* utstring_body(dune_pkg_file), *\/ */
    /*                  TO_STR(stublibs)); */
    /*     } */
    /* } */
    iter = s7_make_iterator(s7, stublibs);
        //gc_loc = s7_gc_protect(s7, iter);
    if (!s7_is_iterator(iter)) {
        log_error("s7_is_iterator fail");
        LOG_S7_DEBUG(0, "not an iterator", iter);
    }
    if (s7_iterator_is_at_end(s7, iter)) {
        log_error("s7_iterator_is_at_end prematurely");
        LOG_S7_DEBUG(0, "iterator prematurely done", iter);
    }
    char *f;
    while (true) {
        stublib_file = s7_iterate(s7, iter);
        if (s7_iterator_is_at_end(s7, iter)) break;
        /// LOG_S7_DEBUG(0, "stublib_file", stublib_file);
        f = s7_object_to_c_string(s7, stublib_file);
        utarray_push_back(stubs, &f);
        free(f);
    }
    return stubs;
}

UT_array *_get_pkg_executables(s7_scheme *s7, void *_stanzas)
{
    TRACE_ENTRY;
    /* /// LOG_S7_DEBUG(0, "stanzas", _stanzas); */

    s7_pointer stanzas = (s7_pointer) _stanzas;
    UT_string *outpath;
    UT_string *opam_bin;
    utstring_new(outpath);
    utstring_new(opam_bin);

    UT_array *bins;
    utarray_new(bins, &ut_str_icd);

    s7_pointer iter, binfile;

    s7_pointer e = s7_inlet(s7,
                            s7_list(s7, 1,
                                    s7_cons(s7,
                                            s7_make_symbol(s7, "stanzas"),
                                            stanzas)));

    char * exec_sexp =
        "(let ((files (assoc 'files stanzas)))"
        "  (if files"
        "      (let ((bin (assoc 'bin (cdr files))))"
        "          (if bin (cadr bin)))))";

    s7_pointer executables = s7_eval_c_string_with_environment(s7, exec_sexp, e);
    /* /// LOG_S7_DEBUG(0, "execs", executables); */

    if (executables == s7_unspecified(s7)) {
        /* LOG_DEBUG(0, "NO BINARIES"); */
        return bins;
    }

        /* LOG_DEBUG(0, "Pkg: %s", utstring_body(dune_pkg_file)); */
        /// LOG_S7_DEBUG(0, "executables", executables);

    /* /\* result is list of executables installed in $PREFIX/bin *\/ */
    /* if (s7_is_list(s7, executables)) { */
    /*     if (verbose) { */
    /*     } */
    /* } */
    iter = s7_make_iterator(s7, executables);
        //gc_loc = s7_gc_protect(s7, iter);
    if (!s7_is_iterator(iter)) {
        log_error("s7_make_iterator failed");
        /// LOG_S7_DEBUG(0, "not an iterator", iter);
    }
    if (s7_iterator_is_at_end(s7, iter)) {
        log_error("s7_iterator_is_at_end prematurely");
        /// LOG_S7_DEBUG(0, "iterator prematurely done", iter);
    }
    char *f;
    while (true) {
        binfile = s7_iterate(s7, iter);
        if (s7_iterator_is_at_end(s7, iter)) break;
        /// LOG_S7_DEBUG(0, "binfile", binfile);
        f = s7_object_to_c_string(s7, binfile);
        utarray_push_back(bins, &f);
        free(f);
    }
        /* utstring_renew(opam_bin); */
        /* utstring_printf(opam_bin, "%s/%s", */
        /*                 utstring_body(opam_switch_bin), */
        /*                 TO_STR(binfile)); */

        /* utstring_renew(outpath); */
        /* utstring_printf(outpath, "%s/%s/bin/%s", */
        /*                 obazl, pkg, TO_STR(binfile)); */
        /* rc = symlink(utstring_body(opam_bin), */
        /*              utstring_body(outpath)); */
        /* if (rc != 0) { */
        /*     if (errno != EEXIST) { */
        /*         perror(NULL); */
        /*         fprintf(stderr, "exiting\n"); */
        /*         exit(EXIT_FAILURE); */
        /*     } */
        /* } */
        /* if (!emitted_bootstrapper) */
        /*     emit_local_repo_decl(bootstrap_FILE, pkg); */

        /* fprintf(ostream, "exports_files([\"%s\"])\n", TO_STR(binfile)); */
        /* fprintf(ostream, "## src: %s\n", utstring_body(opam_bin)); */
        /* fprintf(ostream, "## dst: %s\n", utstring_body(outpath)); */
    /* } */
    return bins;
}

/* FIXME: same in mibl/error_handler.c */
void _emit_opam_pkg_bindir(UT_string *dst_dir,
                           char *switch_pfx,
                           char *coswitch_lib,
                           const char *pkg
                          )
{
    TRACE_ENTRY;
    LOG_DEBUG(0, "switch_pfx: %s", switch_pfx);
    /* read dune-package file. if it exports executables:
       1. write bin/BUILD.bazel with a rule for each
       2. symlink from opam switch
     */
    UT_string *dune_pkg_file;
    utstring_new(dune_pkg_file);
    utstring_printf(dune_pkg_file, "%s/lib/%s/dune-package",
                    switch_pfx, /* global */
                    pkg);

    //FIXME: verify access
    LOG_DEBUG(0, "dune-pkg: %s", utstring_body(dune_pkg_file));
    // call read from dune_s7
    s7_pointer data_fname7 = s7_make_string(s7, utstring_body(dune_pkg_file));
    s7_pointer readlet
        = s7_inlet(s7,
                   s7_list(s7, 1,
                           s7_cons(s7,
                                   s7_make_symbol(s7, "datafile"),
                                   data_fname7)));

    char *cmd = "(call-with-input-file datafile dune:read)";
    s7_pointer stanzas = s7_eval_c_string_with_environment(s7, cmd, readlet);
    /* LOG_DEBUG(0, "called s7", ""); */
    /* LOG_S7_DEBUG(0, "stanzas", stanzas); */

    UT_array *executables = _get_pkg_executables(s7, stanzas);

    if (utarray_len(executables) == 0) goto stublibs;

    /* if executables not null:
       1. create 'bin' subdir of pkg
       2. symlink executables to <pkg>/bin
       3. add BUILD.bazel with exports_files for linked executables
     */

    UT_string *outpath;
    utstring_new(outpath);
    UT_string *opam_bin;
    utstring_new(opam_bin);

    errno = 0;
    utstring_new(outpath);
    utstring_printf(outpath,
                    "%s/bin",
                    utstring_body(dst_dir));
                    /* "%s/opam.%s/bin", */
                    /* coswitch_lib, */
                    /* pkg); */
    mkdir_r(utstring_body(outpath));

    /* create <pkg>/bin/BUILD.bazel */
    utstring_renew(outpath);
    utstring_printf(outpath,
                    "%s/bin/BUILD.bazel",
                    utstring_body(dst_dir));
                    /* "%s/opam.%s/bin/BUILD.bazel", */
                    /* coswitch_lib, */
                    /* pkg); */
    /* rc = access(utstring_body(build_bazel_file), F_OK); */
    LOG_DEBUG(0, "fopening: %s", utstring_body(outpath));

    errno = 0;
    FILE *ostream;
    ostream = fopen(utstring_body(outpath), "w");
    if (ostream == NULL) {
        printf(RED "ERROR" CRESET "fopen failure for %s", utstring_body(outpath));
        log_error("fopen failure for %s", utstring_body(outpath));
        perror(utstring_body(outpath));
        log_error("Value of errno: %d", errno);
        log_error("fopen error %s", strerror( errno ));
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "## generated file - DO NOT EDIT\n");

    fprintf(ostream, "exports_files([");

    /* For each executable, create symlink and exports_files entry */
    char **p = NULL;
    while ( (p=(char**)utarray_next(executables,p))) {
        LOG_DEBUG(0, "bin: %s",*p);
        fprintf(ostream, "\"%s\",", *p);
        /* symlink */
        utstring_renew(opam_bin);
        utstring_printf(opam_bin,
                        "%s/bin/%s",
                        switch_pfx,
                        /* utstring_body(switch_bin), */
                        *p);
        LOG_DEBUG(0, "SYMLINK SRC: %s", utstring_body(opam_bin));
        utstring_renew(outpath);
        utstring_printf(outpath,
                        "%s/bin/%s",
                        utstring_body(dst_dir),
                        *p);
                        /* "%s/opam.%s/bin/%s", */
                        /* coswitch_lib, */
                        /* pkg, */
                        /* *p); */
        LOG_DEBUG(0, "SYMLINK DST: %s", utstring_body(outpath));
        int rc = symlink(utstring_body(opam_bin),
                     utstring_body(outpath));
        symlink_ct++;
        if (rc != 0) {
            if (errno != EEXIST) {
                perror(NULL);
                fprintf(stderr, "exiting\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    fprintf(ostream, "])\n");
    fclose(ostream);

    if (verbose && verbosity > 1) {
        utstring_renew(outpath);
        utstring_printf(outpath,
                        "%s/bin", utstring_body(dst_dir));
                        /* "%s/opam.%s/bin", */
                        /* coswitch_lib, */
                        /* pkg); */
        
        LOG_INFO(0, "Created %s containing symlinked pkg executables",
                 utstring_body(outpath));
    }


 stublibs:
    ;
    /* LOG_DEBUG(0, "STUBLIBS"); */

    // FIXME: get stublibs dir from opam_switch_stublibs()

    /* opam dumps all stublibs ('dllfoo.so') in lib/stublibs; they are
       not found in the pkg's lib subdir. But the package's
       dune-package file lists them, so we read that and then symlink
       them from lib/stublibs to lib/<pkg>/stublibs.
     */

    UT_array *stublibs = _get_pkg_stublibs(s7, (char*)pkg, stanzas);
    if (utarray_len(stublibs) == 0) goto exit;

    UT_string *opam_stublib;
    utstring_new(opam_stublib);

    /* s7_pointer iter, binfile; */

    errno = 0;
    utstring_new(outpath);
    utstring_printf(outpath, "%s/%s/stublibs",
                    coswitch_lib,
                    pkg);
    mkdir_r(utstring_body(outpath));
    /* LOG_DEBUG(0, "stublibs: %s", utstring_body(outpath)); */
    utstring_renew(outpath);
    utstring_printf(outpath, "%s/%s/stublibs/BUILD.bazel",
                    coswitch_lib,
                    pkg);
    /* rc = access(utstring_body(build_bazel_file), F_OK); */
    LOG_DEBUG(0, "fopening: %s", utstring_body(outpath));

    /* FILE *ostream; */
    errno = 0;
    ostream = fopen(utstring_body(outpath), "w");
    if (ostream == NULL) {
        printf(RED "ERROR" CRESET "fopen failure for %s", utstring_body(outpath));
        log_error("fopen failure for %s", utstring_body(outpath));
        perror(utstring_body(outpath));
        log_error("Value of errno: %d", errno);
        log_error("fopen error %s", strerror( errno ));
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "## generated file - DO NOT EDIT\n");

    fprintf(ostream, "exports_files([");

    /* For each stublib, create symlink and exports_files entry */
    p = NULL;
    while ( (p=(char**)utarray_next(stublibs,p))) {
        LOG_DEBUG(0, "stublib: %s",*p);
        fprintf(ostream, "\"%s\",", *p);
        /* symlink */
        utstring_renew(opam_stublib);
        utstring_printf(opam_stublib, "%s/lib/stublibs/%s",
                        switch_pfx,
                        *p);
        LOG_DEBUG(0, "SYMLINK SRC: %s", utstring_body(opam_stublib));
        utstring_renew(outpath);
        utstring_printf(outpath, "%s/%s/stublibs/%s",
                        coswitch_lib,
                        pkg,
                        *p);
        LOG_DEBUG(0, "SYMLINK DST: %s", utstring_body(outpath));
        int rc = symlink(utstring_body(opam_stublib),
                     utstring_body(outpath));
        symlink_ct++;
        if (rc != 0) {
            if (errno != EEXIST) {
                perror(NULL);
                fprintf(stderr, "ERROR,exiting\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    fprintf(ostream, "])\n");
    fclose(ostream);

    if (verbose && verbosity > 1) {
        utstring_renew(outpath);
        utstring_printf(outpath, "%s/%s/stublibs",
                        coswitch_lib,
                        pkg);

        LOG_INFO(0, "Created %s containing symlinked stublibs",
                 utstring_body(outpath));
    }

 exit: ;
    TRACE_EXIT;
    /* utstring_free(outpath); */
}

UT_string *dune_pkg_file;

/* pkg always relative to (global) opam_switch_lib */
EXPORT void emit_pkg_bindir(UT_string *dst_dir,
                            char *opam_switch_pfx,
                            char *coswitch_lib,
                            const char *pkg)
{
    TRACE_ENTRY;
    LOG_TRACE(0, "emit_pkg_bindir: %s", pkg);

    utstring_renew(dune_pkg_file);
    utstring_printf(dune_pkg_file,
                    "%s/lib/%s/dune-package",
                    opam_switch_pfx, /* global */
                    pkg);

    LOG_DEBUG(0, "CHECKING DUNE-PACKAGE: %s\n", utstring_body(dune_pkg_file));
    if (access(utstring_body(dune_pkg_file), F_OK) == 0) {
        _emit_opam_pkg_bindir(dst_dir,
                              opam_switch_pfx,
                              coswitch_lib,
                              pkg); // dune_pkg_file);
                             /* switch_lib, */
                             /* pkgdir, */
                             /* obazl_opam_root, */
                             /* emitted_bootstrapper); */
    }
}
