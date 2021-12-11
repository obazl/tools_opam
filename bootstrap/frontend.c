#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "utarray.h"
#if INTERFACE
#include "uthash.h"
#endif
#include "utstring.h"

#include "log.h"

#include "bootstrap.h"
/* FIXME: consolidate hdrs from libbootstrap */
/* #include "config.h" */
/* #include "driver.h" */
/* #include "meta_flags.h" */
/* #include "obazl.h" */
/* #include "opam.h" */

#include "frontend.h"

extern bool debug;
extern bool local_opam;
extern bool verbose;

/* char *host_repo = "ocaml"; */

/* char *CWD; */
/* char log_buf[512]; */

/* **************************************************************** */
/* static int level = 0; */
/* static int spfactor = 4; */
/* static char *sp = " "; */

/* static int indent = 2; */
/* static int delta = 2; */

/* **************************************************************** */

/* static int verbosity = 0; */
int errnum;
int rc;

bool g_ppx_pkg;

char work_buf[PATH_MAX];

UT_array *opam_packages;

char coqlib[PATH_MAX];

struct buildfile_s {
    char *name;                 /* set from strndup optarg; must be freed */
    char *path;                 /* set from getenv; do not free */
    UT_hash_handle hh;
} ;
struct buildfile_s *buildfiles = NULL;

struct fileset_s *filesets = NULL;

/* struct package_s *packages = NULL; */

EXPORT int opam_main(int argc, char *argv[]) // , char **envp)
{
    char *opts = "dhls";
    char *opam_switch = NULL;

    int opt;
    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
        case '?':
            log_debug("uknown opt: %c", optopt);
            exit(EXIT_FAILURE);
            break;
        case ':':
            log_debug("uknown option: %c", optopt);
            exit(EXIT_FAILURE);
            break;
        case 'd':
            debug = true;
            break;
        case 'l':
            local_opam = true;
            break;
        case 's':
            printf("option s (switch): %s\n", optarg);
            opam_switch = strndup(optarg, PATH_MAX);
            break;
        /* case 'x': */
        /*     verbosity++; */
        /*     break; */
        case 'v':
            verbose = true;
            break;
        case 'h':
            log_info("Usage: install [options]");
#ifdef DEBUG
            log_info("\toptions: -d (debug), -l (local opam)");
#else
            log_info("\toptions: -d debug, -l (local opam)");
#endif
            exit(EXIT_SUCCESS);
            break;
        default:
            ;
        }
    }

    if (debug)
        fprintf(stdout, "DEBUGGING\n");

    /* we search all pkg names for '/'; this table means we only build
       the internal Knuth-Morris-Pratt table once */
    extern long *KPM_TABLE; /* in emit_build_bazel.c */
    KPM_TABLE = (long *)malloc( sizeof(long) * 2); /* (strlen("")) + 1)); */
    _utstring_BuildTable("/", 1, KPM_TABLE);

    /* obazl config sets cwd, must be called first */
    obazl_configure(getcwd(NULL, 0));
    config_logging();

    /* char *wd = getcwd(NULL, 0); */
    /* fprintf(stdout, "CWD after bzl config: %s\n", wd); */

    /* CWD = getcwd(NULL, 0); */

    utarray_new(opam_packages, &ut_str_icd);

    initialize_config_flags();

    char bzlroot[PATH_MAX];

    mystrcat(bzlroot, "./.opam.d");

    printf("PROG: %s\n", basename(argv[0]));
    if (strcmp(basename(argv[0]), "init") == 0) {
        install_project_opam(opam_switch, bzlroot);
    } else {
        if (strcmp(basename(argv[0]), "ingest") == 0) {
            opam_ingest(opam_switch, bzlroot);
        } else {
            if (strcmp(basename(argv[0]), "install") == 0) {
                printf("install\n");
                /* opam_install(opam_switch, bzlroot); */
            } else {
                if (strcmp(basename(argv[0]), "status") == 0) {
                    printf("status\n");
                    /* opam_status(opam_switch, bzlroot); */
                }
            }
        }
    }

    dispose_flag_table();

    if (opam_switch)
        free(opam_switch);

    free(KPM_TABLE);

    /* fprintf(opam_resolver, ")\n"); */

    /* fclose(opam_resolver); */

#ifdef DEBUG
    log_info("bzlroot: %s", bzlroot);
    log_info("FINISHED");
#endif

    shutdown();
}
