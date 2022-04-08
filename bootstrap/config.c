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

/* ANSI color codes for printf */
/* https://gist.github.com/RabaDabaDoba/145049536f815903c79944599c6f952a */
#if INTERFACE
//Regular text
#define BLK "\033[0;30m"
#define RED "\033[0;31m"
#define GRN "\033[0;32m"
#define YEL "\033[0;33m"
#define BLU "\033[0;34m"
#define MAG "\033[0;35m"
#define CYN "\033[0;36m"
#define WHT "\033[0;37m"

//Regular bold text
#define BBLK "\033[1;30m"
#define BRED "\033[1;31m"
#define BGRN "\033[1;32m"
#define BYEL "\033[1;33m"
#define BBLU "\033[1;34m"
#define BMAG "\033[1;35m"
#define BCYN "\033[1;36m"
#define BWHT "\033[1;37m"

//Regular underline text
#define UBLK "\033[4;30m"
#define URED "\033[4;31m"
#define UGRN "\033[4;32m"
#define UYEL "\033[4;33m"
#define UBLU "\033[4;34m"
#define UMAG "\033[4;35m"
#define UCYN "\033[4;36m"
#define UWHT "\033[4;37m"

//Regular background
#define BLKB "\033[40m"
#define REDB "\033[41m"
#define GRNB "\033[42m"
#define YELB "\033[43m"
#define BLUB "\033[44m"
#define MAGB "\033[45m"
#define CYNB "\033[46m"
#define WHTB "\033[47m"

//High intensty background
#define BLKHB "\033[0;100m"
#define REDHB "\033[0;101m"
#define GRNHB "\033[0;102m"
#define YELHB "\033[0;103m"
#define BLUHB "\033[0;104m"
#define MAGHB "\033[0;105m"
#define CYNHB "\033[0;106m"
#define WHTHB "\033[0;107m"

//High intensty text
#define HBLK "\033[0;90m"
#define HRED "\033[0;91m"
#define HGRN "\033[0;92m"
#define HYEL "\033[0;93m"
#define HBLU "\033[0;94m"
#define HMAG "\033[0;95m"
#define HCYN "\033[0;96m"
#define HWHT "\033[0;97m"

//Bold high intensity text
#define BHBLK "\033[1;90m"
#define BHRED "\033[1;91m"
#define BHGRN "\033[1;92m"
#define BHYEL "\033[1;93m"
#define BHBLU "\033[1;94m"
#define BHMAG "\033[1;95m"
#define BHCYN "\033[1;96m"
#define BHWHT "\033[1;97m"

//Reset
#define reset "\033[0m"
#define CRESET "\033[0m"
#define COLOR_RESET "\033[0m"
#endif

bool debug;
bool dry_run;
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

UT_string *logfile;
EXPORT void config_logging(UT_string *_logfile)
{
    /* printf("config_logging: %s\n", utstring_body(_logfile)); */
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
    log_add_fp(log_fp, LOG_TRACE);
    /* log_add_callback(log_fn, NULL, LOG_TRACE); */

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
                    getcwd(NULL, 0), utstring_body(logfile));
        utstring_free(logfile);

    }
}
