#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>             /* PATH_MAX */
#endif
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ini.h"
#include "log.h"
/* #include "s7.h" */
#include "utstring.h"
#if INTERFACE
#include "utarray.h"
#endif

#include "obazl_config.h"

bool debug;
bool verbose;

/* FILE *log_fp; */
/* const char *logfile = "./.opam.d/update.log"; */
/* bool logging_to_file = false; */

/* s7_scheme *s7;                  /\* GLOBAL s7 *\/ */
/* s7_pointer old_port; */
/* LOCAL s7_pointer result; */
/* int gc_loc = -1; */
/* const char *errmsg = NULL; */


/* char *callback_script_file = "ocamlark.scm"; // passed in 'data' attrib */
/* char *callback = "ocamlark_handler"; /\* fn in callback_script_file  *\/ */

/* script directories: sys, user, proj, in order */
   /* obazl (sys) scripts: */
   /*     run under `bazel run`: dir in runfiles containing callback script */
   /*         defaults to @ocamlark//scm */
   /*     run directly: xdg system dir */
   /*         XDG_DATA_DIRS = (/usr/local/share)/obazl/scm */
   /* user scripts: */
   /*     ($HOME)/.obazl.d/scm */
   /*     XDG_DATA_HOME = ($HOME/.local/share)/obazl/scm */
   /* proj scripts: */
   /*     .obazl.d */

/* char *bazel_script_dir = NULL; */

/* LOCAL UT_string *opam_switch; */
/* LOCAL UT_string *opam_bin; */
/* LOCAL UT_string *opam_lib; */

#if INTERFACE
#define OBAZL_ROOT ".obazl.d"
#endif

char *obazl_root = OBAZL_ROOT;

UT_string *exec_root;
UT_string *runfiles_root;
UT_string *proj_root;
UT_string *obazl_d;

UT_string *runtime_data_dir;

/* bool ini_error = false; */
UT_string *obazl_ini_path;
#if INTERFACE
#define OBAZL_INI_FILE ".obazlrc"
#endif

/* UT_string *codept_args_file; */
/* const char *codept_args_filename = "codept.args"; */

/* /\* static const char codept_depends[] = ".obazl.d/codept.depends"; *\/ */
/* UT_string *codept_deps_file; */
/* const char *codept_deps_filename = "codept.deps"; */

#if EXPORT_INTERFACE
struct configuration_s {
    char *obazl_version;
    int libct;
    char *here_root;
    char *here_switch;
    char *compiler_version;
    UT_array *compiler_options;

    UT_array *opam_packages;        /* string list */

    // for dune conversion?
    UT_array *src_dirs; /* string list; used by fileseq to get src_files */
    UT_array *watch_dirs;       /* string list */
    /* struct lib_s *ocamllibs[10]; /\* is 10 enough? *\/ */
    /* struct lib_s *coqlibs[10]; /\* is 10 enough? *\/ */
};
#endif

#define OBAZL_VERSION "0.1.0"

struct configuration_s obazl_config = {
    .obazl_version = OBAZL_VERSION,
    .libct = 0
};

UT_array *src_files;            /* FIXME: put this in configuration_s? */

EXPORT void obazl_configure(char *_exec_root, char *cmd)
{
#if defined(DEBUG_TRACE)
    /* printf("obazl_configure: %s, %s\n", _exec_root, cmd); */
    log_debug("obazl_configure: %s", cmd);
#endif

    /* if (access("bazel-out/volatile.txt", F_OK)) */
    /*     printf("FOUND: bazel-out/volatile.txt\n"); */
    /* else */
    /*     printf("NOT FOUND: bazel-out/volatile.txt\n"); */

    char *subcmd = basename(cmd);

    utstring_new(exec_root);
    utstring_printf(exec_root, "%s", _exec_root);
#if defined(DEBUG_TRACE)
    log_debug("EXEC_ROOT: %s", utstring_body(exec_root));
#endif
    utstring_new(runfiles_root);
    utstring_printf(runfiles_root, "%s", getcwd(NULL, 0));
#if defined(DEBUG_TRACE)
    log_debug("runfiles_root: %s", utstring_body(runfiles_root));
#endif
    char *_proj_root = getenv("BUILD_WORKSPACE_DIRECTORY");

#if defined(DEBUG_TRACE)
    if (_proj_root == NULL) {
        log_debug("BUILD_WORKSPACE_DIRECTORY: null");
    } else {
        log_debug("BUILD_WORKSPACE_DIRECTORY: %s", _proj_root);
    }
#endif

    char *_wd = getenv("BUILD_WORKING_DIRECTORY");

#if defined(DEBUG_TRACE)
    if (_wd == NULL) {
        log_debug("BUILD_WORKING_DIRECTORY: null");
    } else {
        log_debug("BUILD_WORKING_DIRECTORY: %s", _wd);
    }
#endif

    utstring_new(proj_root);
    if (_proj_root == NULL)
        utstring_printf(proj_root, "%s", getcwd(NULL, 0));
    else
        utstring_printf(proj_root, "%s", _proj_root);

    /* we always want cwd to be proj root  */
    chdir(_proj_root);
    /* _config_logging(); */

    UT_string *logfile;
    utstring_new(logfile);
    utstring_printf(logfile, "opam_%s", cmd);
    config_logging(logfile);
    utstring_free(logfile);

    /* .obazlrc config file */
    utstring_new(obazl_ini_path);
    utstring_printf(obazl_ini_path, "%s/%s", utstring_body(proj_root), OBAZL_INI_FILE);

    rc = access(utstring_body(obazl_ini_path), R_OK);
    if (rc) {
        /* if (verbose || debug) */
        log_warn("Config file %s not found.", utstring_body(obazl_ini_path));
    } else {
#if defined(DEBUG_TRACE)
        log_trace("found config file at %s\n",
                  utstring_body(obazl_ini_path));
#endif
        ini_error = false;
        utarray_new(obazl_config.compiler_options, &ut_str_icd);
        utarray_new(obazl_config.src_dirs, &ut_str_icd);
        utarray_new(obazl_config.watch_dirs, &ut_str_icd);
        utarray_new(obazl_config.opam_packages, &ut_str_icd);
        rc = ini_parse(utstring_body(obazl_ini_path), inih_handler, &obazl_config);
        if (rc < 0) {
            //FIXME: deal with missing .obazlrc
            perror("ini_parse");
            log_fatal("Can't load/parse ini file: %s", utstring_body(obazl_ini_path));
            exit(EXIT_FAILURE);
        }
        if (ini_error) {
            log_error("Error parsing ini file");
            exit(EXIT_FAILURE);
        /* } else { */
        /*     log_debug("Config loaded from %s", utstring_body(obazl_ini_path)); */
        }
    }

    /* if (obazl_config.here_root == NULL) { */
    /*     obazl_config.here_root = malloc(6); */
    /*     strlcpy(obazl_config.here_root, ".opam", 6); */
    /* } */
    /* if (obazl_config.here_switch == NULL) { */
    /*     obazl_config.here_switch = malloc(5); */
    /*     strlcpy(obazl_config.here_switch, "here", 5); */
    /* } */

    /* if (obazl_config.here_compiler == NULL) { */
    /*     obazl_config.here_compiler = malloc(5); */
    /*     strlcpy(obazl_config.here_compiler, "here", 5); */
    /* } */

#if defined(DEBUG_TRACE)
    if (obazl_config.here_root)
        log_debug("here root: %s", obazl_config.here_root);
    if (obazl_config.here_switch)
        log_debug("here switch: %s", obazl_config.here_switch);
    if (obazl_config.compiler_version)
        log_debug("compiler version: %s", obazl_config.compiler_version);
    if (obazl_config.compiler_options != NULL) {
        log_debug("compiler_options ct: %d",
                  utarray_len(obazl_config.compiler_options));
        char **p = NULL;
        while ( (p=(char**)utarray_next(obazl_config.compiler_options, p))) {
            log_debug("option: %s",*p);
        }
    }
    if (obazl_config.opam_packages != NULL) {
        log_debug("opam deps ct: %d", utarray_len(obazl_config.opam_packages));
        char **p = NULL;
        while ( (p=(char**)utarray_next(obazl_config.opam_packages, p))) {
            log_debug("%s",*p);
        }
    }
#endif
    utarray_new(src_files,&ut_str_icd);

    /* _s7_init(); */
}
