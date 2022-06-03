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
#include "utarray.h"
#if INTERFACE
#include "utstring.h"
#endif

#include "config.h"

bool debug;
bool dry_run;
bool verbose;
bool expunge; /* for @opam//foo:clean */

FILE *log_fp;
/* const char *logfile = OBAZL_OPAM_ROOT "/update.log"; */
UT_string *logfile;
bool logging_to_file = false;

char log_buf[512];

/* s7_scheme *s7;                  /\* GLOBAL s7 *\/ */
/* s7_pointer old_port; */
/* LOCAL s7_pointer result; */
/* int gc_loc = -1; */
/* const char *errmsg = NULL; */

/* #if EXPORT_INTERFACE */
char *host_repo = "ocaml";
/* #endif */

/* char *callback_script_file = "ocamlark.scm"; // passed in 'data' attrib */
/* char *callback = "ocamlark_handler"; /\* fn in callback_script_file  *\/ */

UT_string *exec_root;
UT_string *runfiles_root;
UT_string *proj_root;
UT_string *obazl_d;

UT_string *runtime_data_dir;

bool ini_error = false;

EXPORT int inih_handler(void* config, const char* section, const char* name, const char* value)
{
#if defined(DEBUG_TRACE)
    log_debug("inih_handler section %s: %s=%s", section, name, value);
#endif
    struct configuration_s *pconfig = (struct configuration_s*)config;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("obazl", "version")) {
        if (verbose) {
            fprintf(stdout, "obazl version: %s\n", value);
            log_debug("obazl version: %s", value);
        }
        return 1; // why?
    }

    if (MATCH("here", "root")) {
        size_t len = strlen((char*)value);
        pconfig->here_root = malloc(len+1);
        strlcpy(pconfig->here_root, (char*)value, len+1);
        return 1;
    }

    if (MATCH("here", "switch")) {
        size_t len = strlen((char*)value);
        pconfig->here_switch = malloc(len+1);
        strlcpy(pconfig->here_switch, (char*)value, len+1);
        return 1;
    }

    if (MATCH("compiler", "version")) {
        size_t len = strlen((char*)value);
        pconfig->compiler_version = malloc(len+1);
        strlcpy(pconfig->compiler_version, (char*)value, len+1);
        return 1;
    }

    if (MATCH("compiler", "options")) {
        char *token, *sep = " ,\t";
        token = strtok((char*)value, sep);
        while( token != NULL ) {
#if defined(DEBUG_TRACE)
            log_debug("compiler build option: %s", token);
#endif
            utarray_push_back(pconfig->compiler_options, &token);
            token = strtok(NULL, sep);
        }
        return 1;
    }

    if (MATCH("opam", "deps")) {
        char *token, *sep = " ,\t";
        token = strtok((char*)value, sep);
        while( token != NULL ) {
/* #if defined(DEBUG_TRACE) */
/*             log_debug("opam dep: %s", token); */
/* #endif */
            utarray_push_back(pconfig->opam_packages, &token);
            token = strtok(NULL, sep);
        }
        return 1;
    }


    if (MATCH("srcs", "dirs")) {
        /* log_debug("section: srcs; entry: dirs"); */
        /* log_debug("\t%s", value); */
        char *token, *sep = " ,\t";
        token = strtok((char*)value, sep);
        while( token != NULL ) {
            /* if (token[0] == '/') { */
            /*     log_error("Ini file: 'dir' values in section 'srcs' must be relative paths: %s", token); */
            /*     ini_error = true; */
            /*     return 0; */
            /* } else { */
                /* log_debug("pushing src dir: %s", token); */
                utarray_push_back(pconfig->src_dirs, &token);
                token = strtok(NULL, sep);
            /* } */
        }
        return 1;
    }

    if (MATCH("watch", "dirs")) {
        /* log_debug("section: watch; entry: dirs"); */
        /* log_debug("\t%s", value); */
        char *token, *sep = " ,\t";
        token = strtok((char*)value, sep);
        while( token != NULL ) {
            if (token[0] == '/') {
                log_error("Ini file: 'dir' values in section 'watch' must be relative paths: %s", token);
                ini_error = true;
                return 0;
            } else {
                /* log_debug("pushing watch dir: %s", token); */
                utarray_push_back(pconfig->watch_dirs, &token);
                token = strtok(NULL, sep);
            }
        }
        return 1;
    }
    return 1;
}

void log_fn(log_Event *evt)
{
    /* DO NOT USE! It seems to corrupt data somehow... */
    /* happens when we use basename on evt->file */

    /* typedef struct { */
    /*   va_list ap; */
    /*   const char *fmt; */
    /*   const char *file; */
    /*   struct tm *time; */
    /*   void *udata; */
    /*   int line; */
    /*   int level; */
    /* } log_Event; */

    static char fname_buffer[MAXPATHLEN];
    memset(log_buf, '0', 512);
    /* DO NOT use allocating basename, it corrupts something... */
    basename_r((char*)evt->file, fname_buffer);
    snprintf(log_buf, 512, "%d %s:%d ",
            evt->level, (char*)&fname_buffer, evt->line);
    fprintf(log_fp, "%s", log_buf);
    vsprintf(log_buf, evt->fmt, evt->ap);
    fprintf(log_fp, "%s\n", log_buf);
}

UT_string *logfile;
EXPORT void config_logging(UT_string *_logfile)
{
#if defined(DEBUG_TRACE)
    log_debug("config_logging");
#endif
    /* CWD = getcwd(NULL, 0); */
    int rc = access(OBAZL_ROOT "/logs", F_OK);
    if (rc < 0) {
        /* printf("creating %s/logs\n", OBAZL_ROOT); */
        mkdir_r(OBAZL_ROOT "/logs");
        /* fprintf(stderr, "ERROR: %s not found at %s\n", */
        /*         OBAZL_OPAM_ROOT, getcwd(NULL, 0)); */
        /* exit(EXIT_FAILURE); */
    }
    rc = access(OBAZL_ROOT, F_OK);
    if (rc < 0) {
        printf("huh?\n");
        exit(EXIT_FAILURE);
    }

    utstring_new(logfile);
    utstring_printf(logfile, "%s/logs/%s.log",
                    OBAZL_ROOT,
                    utstring_body(_logfile));
    /* printf("logfile: %s\n", utstring_body(logfile)); */
    log_fp = fopen(utstring_body(logfile), "w");
    if (log_fp == NULL) {
        perror(utstring_body(logfile));
        log_error("fopen fail on %s", logfile);
        fflush(stderr); fflush(stdout);
        utstring_free(logfile);
        exit(EXIT_FAILURE);
    }
    /* utstring_free(logfile); */
    logging_to_file = true;
    /* fprintf(stdout, "opened logfile %s\n", utstring_body(logfile)); */

    /* one or the other: */
#if defined(DEBUG_TRACE)
    log_debug("setting logfile to: %s", utstring_body(logfile));
#endif
    log_add_fp(log_fp, LOG_TRACE);
    /* log_add_callback(log_fn, NULL, LOG_TRACE); */

#ifdef DEBUG_TRACE
    /* fprintf(stdout, "DEBUG\n"); */
    log_set_quiet(false);
    log_set_level(LOG_TRACE);
#else
    log_set_quiet(true);
    log_set_level(LOG_INFO);
#endif
}


// FIXME: replace with utstrings
EXPORT char* mystrcat( char* dest, char* src )
{
     while (*dest) dest++;
     while ( (*dest++ = *src++) );
     return --dest;
}


EXPORT void shutdown(void)
{
    fflush(stdout);
    fflush(stderr);

    if (logging_to_file) {
        fclose(log_fp);
        if (verbose)
            fprintf(stdout, "logfile: %s/%s\n",
                    getcwd(NULL, 0), utstring_body(logfile));
        utstring_free(logfile);
        return;
    }
    return;
}
