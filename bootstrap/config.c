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
#include "utstring.h"

#include "config.h"

bool debug;
bool verbose;

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

LOCAL UT_string *opam_switch;
LOCAL UT_string *opam_bin;
LOCAL UT_string *opam_lib;

UT_string *exec_root;
UT_string *runfiles_root;
UT_string *proj_root;
UT_string *obazl_d;

UT_string *runtime_data_dir;

bool ini_error = false;
UT_array *src_files;            /* FIXME: put this in configuration_s? */

EXPORT int inih_handler(void* config, const char* section, const char* name, const char* value)
{
    log_debug("inih_handler section %s: %s=%s", section, name, value);
    struct configuration_s *pconfig = (struct configuration_s*)config;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("obazl", "version")) {
        if (verbose)
            log_debug("obazl version: %s", value);
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

EXPORT void config_logging(char *_logfile)
{
    /* printf("config_logging: %s\n", OBAZL_OPAM_ROOT); */
    /* CWD = getcwd(NULL, 0); */
    int rc = access(OBAZL_OPAM_ROOT, F_OK);
    if (rc < 0) {
        printf("creating %s\n", OBAZL_OPAM_ROOT);
        mkdir_r(OBAZL_OPAM_ROOT);
        /* fprintf(stderr, "ERROR: %s not found at %s\n", */
        /*         OBAZL_OPAM_ROOT, getcwd(NULL, 0)); */
        /* exit(EXIT_FAILURE); */
    }
    rc = access(OBAZL_OPAM_ROOT, F_OK);
    if (rc < 0) {
        printf("huh?\n");
        exit(EXIT_FAILURE);
    }

    utstring_new(logfile);
    utstring_printf(logfile, "%s/%s.log", OBAZL_OPAM_ROOT, _logfile);
    log_fp = fopen(utstring_body(logfile), "w");
    if (log_fp == NULL) {
        perror(utstring_body(logfile));
        log_error("fopen fail on %s", logfile);
        fflush(stderr); fflush(stdout);
        utstring_free(logfile);
        exit(EXIT_FAILURE);
    }
    utstring_free(logfile);
    logging_to_file = true;
    /* fprintf(stdout, "opened logfile %s\n", logfile); */

    /* one or the other: */
    /* log_add_fp(log_fp, LOG_TRACE); */
    log_add_callback(log_fn, NULL, LOG_TRACE);

#ifdef DEBUG_TRACE
    fprintf(stdout, "XDEBUG\n");
    log_set_quiet(true);
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
    if (logging_to_file) {
        fclose(log_fp);
        if (verbose)
            fprintf(stdout, "logfile: %s/%s\n",
                    getcwd(NULL, 0), logfile);
    }
}
