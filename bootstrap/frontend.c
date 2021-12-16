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
//extern bool dry_run;
extern bool verbose;

/* extern bool local_opam; */

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

void print_usage(char *cmd)
{
    printf("Usage: bazel run @opam//%s -- [args]\n", cmd);
    if (strcmp(cmd, "config") == 0) {
        printf("\targs: none\n");
    }
    else if (strcmp(cmd, "deps") == 0) {
    }
    else if (strcmp(cmd, "export") == 0) {
        printf("\targs: -m <manifest file>\n");
    }
    else if (strcmp(cmd, "import") == 0) {
        printf("\targs: -m <manifest file>\n");
        printf("\tDefault (no args): imports .obazl.d/opam/here.packages if found.\n");
    }
    else if (strcmp(cmd, "init") == 0) {
        printf("\targs: -c <compiler version>\n");
        printf("\tDefault: uses compiler version listed in .obazl.d/opam.switch if found; otherwise uses compiler for current switch, unless it is the system compiler.\n");
    }
    else if (strcmp(cmd, "install") == 0) {
        printf("\targs: -p <package>\n");
    }
    else if (strcmp(cmd, "remove") == 0) {
        printf("\targs: -p <package>\n");
    }
    else if (strcmp(cmd, "status") == 0) {
        printf("\targs: none\n");
    }
}

void ignore_msg(void)
{
    printf("(obazl says: you can ignore any advice about running eval to update the shell environment, so long as you use @opam commands to control your here-switch.)\n");
}

EXPORT int opam_main(int argc, char *argv[]) // , char **envp)
{
    char *opam_switch = NULL;
    char *compiler_version = NULL;

    bool force = false;
    char *deps_root = NULL;
    char *manifest = NULL;
    char *package = NULL;

    char *opts = "c:dfhm:p:r:s:vx";
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
        case 'c':
            compiler_version = strndup(optarg, PATH_MAX);
            break;
        case 'd':
            debug = true;
            break;
        case 'f':
            force = true;
            break;
        /* case 'l': */
        /*     local_opam = true; */
        /*     break; */
        case 'm':
            manifest = optarg;
            break;
        case 'p':
            package = optarg;
            break;
        case 'r':
            deps_root = optarg;
            break;
        case 'x':
            dry_run = true;
            break;
        case 's':
            /* printf("option s (switch): %s\n", optarg); */
            opam_switch = strndup(optarg, PATH_MAX);
            break;
        case 'v':
            verbose = true;
            break;
        case 'h':
            print_usage(basename(argv[0]));
            exit(EXIT_SUCCESS);
            break;
        default:
            ;
        }
    }

    if (debug)
        fprintf(stdout, "DEBUGGING\n");

    if (dry_run)
        printf("DRY RUN\n");

    /* we search all pkg names for '/'; this table means we only build
       the internal Knuth-Morris-Pratt table once */
    extern long *KPM_TABLE; /* in emit_build_bazel.c */
    KPM_TABLE = (long *)malloc( sizeof(long) * 2); /* (strlen("")) + 1)); */
    _utstring_BuildTable("/", 1, KPM_TABLE);

    /* obazl config sets cwd, must be called first */
    obazl_configure(getcwd(NULL, 0));

    UT_string *logfile;
    utstring_new(logfile);
    utstring_printf(logfile, "opam_%s", basename(argv[0]));
    config_logging(logfile);
    utstring_free(logfile);

    utarray_new(opam_packages, &ut_str_icd);

    initialize_config_flags();

    int result;

    if (strncmp(basename(argv[0]), "config", 6) == 0) {
        if (strncmp(basename(dirname(argv[0])), "here", 4) == 0) {
            opam_config(NULL);
        } else {
            opam_config(opam_switch);
        }
    }
    else if (strcmp(basename(argv[0]), "deps") == 0) {
        opam_deps(deps_root);
    }
    else if (strcmp(basename(argv[0]), "export") == 0) {
        opam_export(manifest);
    }
    else if (strcmp(basename(argv[0]), "import") == 0) {
        opam_import(manifest);
        ignore_msg();
    }
    else if (strcmp(basename(argv[0]), "init") == 0) {
        if (compiler_version && opam_switch) {
            printf("@opam/here/init: only one of -c and -s may be passed\n");
            exit(EXIT_FAILURE);
        }
        result = opam_init_here(force, compiler_version, opam_switch);
        if (result == 0)
            ignore_msg();
    }
    else if (strcmp(basename(argv[0]), "install") == 0) {
        if (package) {
            opam_install(package);
            ignore_msg();
        } else
            printf("install command requires -p <pkg> arg\n");
    }
    else if (strcmp(basename(argv[0]), "remove") == 0) {
        if (package) {
            opam_remove(package);
            ignore_msg();
        } else
            printf("remove command requires -p <pkg> arg\n");
    }
    else if (strcmp(basename(argv[0]), "show") == 0) {
        opam_status();
    }
    else if (strcmp(basename(argv[0]), "status") == 0) {
        opam_status();
    }
    else printf("Unknown cmd: %s\n", basename(argv[0]));

    dispose_flag_table();

    if (compiler_version) free(compiler_version);
    if (opam_switch) free(opam_switch);

    free(KPM_TABLE);

    /* fprintf(opam_resolver, ")\n"); */

    /* fclose(opam_resolver); */

#ifdef DEBUG
    log_info("obazl_opam_root: %s", obazl_opam_root);
    log_info("FINISHED");
#endif

    shutdown();
}
