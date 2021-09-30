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

/* FIXME: consolidate hdrs from libbootstrap */
/* #include "libbootstrap.h" */
#include "config.h"
#include "driver.h"
#include "meta_flags.h"

#include "update.h"

extern bool debug;
extern bool verbose;

char *host_repo = "ocaml";

char *CWD;
char log_buf[512];

/* **************************************************************** */
static int level = 0;
static int spfactor = 4;
static char *sp = " ";

static int indent = 2;
static int delta = 2;

/* **************************************************************** */

static int verbosity = 0;
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

int main(int argc, char *argv[]) // , char **envp)
{

    char *opts = "dh";
    int opt;
    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
        case '?':
            /* log_debug("uknown opt: %c", optopt); */
            exit(EXIT_FAILURE);
            break;
        case ':':
            /* log_debug("uknown opt: %c", optopt); */
            exit(EXIT_FAILURE);
            break;
        case 'd':
            debug = true;
            break;
        /* case 'b': */
        /*     /\* build_files *\/ */
        /*     printf("option b: %s\n", optarg); */
        /*     printf("build file: %s\n", getenv(optarg)); */
        /*     struct buildfile_s *the_buildfile = (struct buildfile_s *)calloc(sizeof (struct buildfile_s), 1); */
        /*     if (the_buildfile == NULL) { */
        /*         errnum = errno; */
        /*         fprintf(stderr, "main calloc failure for struct buildfile_s *\n"); */
        /*         perror(getenv(optarg)); */
        /*         exit(1); */
        /*     } */
        /*     the_buildfile->name = strndup(optarg, 512); */
        /*     the_buildfile->path = getenv(optarg); */
        /*     HASH_ADD_STR(buildfiles, name, the_buildfile); */
        /*     break; */
        /* case 'p': */
        /*     printf("option p: %s\n", optarg); */
        /*     /\* if pkg name has form foo.bar, the META file will be in foo *\/ */

        /*     utarray_push_back(opam_packages, &optarg); */
        /*     break; */
        /* case 's': */
        /*     printf("option s: %s\n", optarg); */
        /*     opam_switch = strndup(optarg, PATH_MAX); */
        /*     break; */
        /* case 'x': */
        /*     verbosity++; */
        /*     break; */
        case 'v':
            verbose = true;
        case 'h':
            log_info("Usage: bootstrap_opam[options]");
#ifdef DEBUG
            log_info("\toptions: b:o:p:s:vh");
#else
            log_info("\toptions: b:p:s:v");
#endif
            exit(EXIT_SUCCESS);
            break;
        default:
            ;
        }
    }

    if (debug)
        fprintf(stdout, "DEBUGGING\n");

    obazl_configure(getcwd(NULL, 0));

    char *wd = getcwd(NULL, 0);
    fprintf(stdout, "CWD after bzl config: %s\n", wd);

    char *opam_switch;

    CWD = getcwd(NULL, 0);

    utarray_new(opam_packages, &ut_str_icd);

    initialize_config_flags();

    char bzlroot[PATH_MAX];

    mystrcat(bzlroot, "./.opam");

    opam_config(opam_switch, bzlroot);

    dispose_flag_table();

#ifdef DEBUG
    log_info("bzlroot: %s", bzlroot);
    log_info("FINISHED");
#endif

    shutdown();
}
