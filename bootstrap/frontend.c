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
#include "frontend.h"

extern bool debug;
extern bool local_opam;
extern bool verbose;

/* static int verbosity = 0; */
int errnum;
int rc;

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
    char *opts = "dfhlm:p:r:s:v";
    char *opam_switch = NULL;

    bool force = false;
    char *deps_root = NULL;
    char *manifest = NULL;
    char *package = NULL;

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
        case 'f':
            force = true;
            break;
        case 'l':
            local_opam = true;
            break;
        case 'm':
            manifest = optarg;
            break;
        case 'p':
            package = optarg;
            break;
        case 'r':
            deps_root = optarg;
            break;
        case 's':
            /* printf("option s (switch): %s\n", optarg); */
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
    config_logging(basename(argv[0]));
    /* char *wd = getcwd(NULL, 0); */
    /* fprintf(stdout, "CWD after bzl config: %s\n", wd); */

    /* CWD = getcwd(NULL, 0); */

    utarray_new(opam_packages, &ut_str_icd);

    initialize_config_flags();

    if (strcmp(basename(argv[0]), "deps") == 0) {
        opam_deps(deps_root);
    }
    else if (strcmp(basename(argv[0]), "export") == 0) {
        opam_export(manifest);
    }
    else if (strcmp(basename(argv[0]), "import") == 0) {
        opam_import(manifest);
    }
    else if (strcmp(basename(argv[0]), "init") == 0) {
        opam_init_project_switch(force, opam_switch);
    }
    else if (strcmp(basename(argv[0]), "ingest") == 0) {
        opam_ingest(opam_switch, obazl_opam_root);
    }
    else if (strcmp(basename(argv[0]), "install") == 0) {
        if (package)
            opam_install(package);
        else
            printf("install command requires -p <pkg> arg\n");
    }
    else if (strcmp(basename(argv[0]), "remove") == 0) {
        if (package)
            opam_remove(package);
        else
            printf("remove command requires -p <pkg> arg\n");
    }
    else if (strcmp(basename(argv[0]), "status") == 0) {
        opam_status();
    }
    else printf("Unknown cmd: %s\n", basename(argv[0]));

    dispose_flag_table();

    if (opam_switch)
        free(opam_switch);

    free(KPM_TABLE);

    /* fprintf(opam_resolver, ")\n"); */

    /* fclose(opam_resolver); */

#ifdef DEBUG
    log_info("obazl_opam_root: %s", obazl_opam_root);
    log_info("FINISHED");
#endif

    shutdown();
}
