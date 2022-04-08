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
    if (strncmp(basename(dirname(dirname(cmd))), "here", 4) == 0) {
        print_here_usage(cmd);
        return;
    }

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

void print_here_usage(char *argv0)
{
    char *cmd = basename(argv0);
    printf("Usage: bazel run @opam//here/%s -- [args]\n", cmd);

    if (strcmp(cmd, "config") == 0) {
        printf("\targs:\n");
        printf("\t\t-c\tcompiler version\n");
        printf("\t\t-s\tswitch name\n");
        printf("\t\t-d\tdebug\n");
        printf("\t\t-x\tdry-run\n");
        printf("\t\t-v\tverbose\n\n");

        printf("List all <here> commands: 'bazel run @opam//here'\n");
        return;
    }
    if (strcmp(cmd, "deps") == 0) {
        printf("\targs:\n");
        return;
    }
    if (strcmp(cmd, "export") == 0) {
        printf("\targs:\n");
        printf("\t\t-m\tmanifest file name\n");
        return;
    }
    if (strcmp(cmd, "import") == 0) {
        printf("\targs:\n");
        printf("\t\t-m\tmanifest file name\n");
        printf("Default (no args): imports .obazl.d/opam/here.packages if found.\n");
        return;
    }
    if (strcmp(cmd, "init") == 0) {
        printf("\targs:\n");
        printf("\t\t-c\tcompiler version\n");
        printf("\t\t-s\tswitch name\n");
        printf("\t\t-x\tdry-run\n");
        printf("\t\t-d\tdebug\n");
        printf("\t\t-v\tverbose\n");
        printf("\tDefault: uses compiler version listed in .obazl.d/here.compiler if found; otherwise prompts user.\n");
        return;
    }
    if (strcmp(cmd, "install") == 0) {
        printf("\targs:\n");
        printf("\t\t-p\tpackage  (repeatable)\n");
        printf("\t\t-s\tswitch  (copy pkgs from switch)\n");
        return;
    }
    if (strcmp(cmd, "remove") == 0) {
        printf("\targs:\n");
        printf("\t\t-p\tpackage  (repeatable)\n");
        return;
    }
    if (strcmp(cmd, "show") == 0) {
        printf("\targs: none\n");
    }
}

void print_ignore_msg(void)
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
            print_usage(argv[0]);
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

    /* obazl config sets cwd and logging, must be called first */
    obazl_configure(getcwd(NULL, 0), basename(argv[0]));

    /* UT_string *logfile; */
    /* utstring_new(logfile); */
    /* utstring_printf(logfile, "opam_%s", basename(argv[0])); */
    /* config_logging(logfile); */
    /* utstring_free(logfile); */

    utarray_new(opam_packages, &ut_str_icd);

    initialize_config_flags();

    int result;

    //FIXME: config should configure obazl adapters for already
    //installed opam switch.
    // then what to call cmd that does it all? install?

    if (strncmp(basename(argv[0]), "config", 6) == 0) {
        /* FIXME: make this a function */
        // exe is here/config/config
        if (strncmp(basename(dirname(dirname(argv[0]))), "here", 4) == 0) {
            if (compiler_version && opam_switch) {
                printf("@opam/here/init: only one of -c and -s may be passed\n");
                exit(EXIT_FAILURE);
            }

            if (access(HERE_OBAZL_ROOT, F_OK) == 0) {
                if (verbose)
                    printf("wiping existing .obazl.d/opam\n");
                run_cmd("rm -rf " HERE_OBAZL_ROOT); /* .obazl.d/opam */
            }

            /* now create obazl adapters for here-switch only, not pkgs */
            opam_config_here(); // compiler_version);
        } else {
            /* xdg/config */
            if (compiler_version && opam_switch) {
                printf("@opam/xdg/init: only one of -c and -s may be passed\n");
                exit(EXIT_FAILURE);
            }
            /* interrogate system to get xdg dirs */
            xdg_configure();
            printf("xdg_opam_root: %s\n", utstring_body(xdg_opam_root));
            /* config buildfiles etc. in xdg */
            opam_config_xdg(opam_switch);
        }
    }
    else if (strncmp(basename(argv[0]), "ingest", 6) == 0) {
        /* FIXME: make this a function */
        // exe is here/config/config
        if (strncmp(basename(dirname(dirname(argv[0]))), "here", 4) == 0) {
            if (compiler_version && opam_switch) {
                printf("@opam/here/init: only one of -c and -s may be passed\n");
                exit(EXIT_FAILURE);
            }

            /* construct the opam here switch */
            result = opam_init_here(force, compiler_version, opam_switch);
            if (result == -1) {
                /* fprintf(stdout, "cancelling\n"); */
                exit(EXIT_SUCCESS);
            } else {
                if (result == 0) {
                    print_ignore_msg();
                    /* if user passed -c, then do not install pkgs; if
                       user passed -s (opam_switch), then install pkgs
                       from the switch. */
                    if (opam_switch)
                        opam_install_here_pkgs(opam_switch);
                } else {
                    if (result == 1) {
                        /* neither -c nor -s passed, but when prompted
                           user accepted current switch. install pkgs from
                           current switch. */
                        print_ignore_msg();
                        opam_install_here_pkgs(NULL);
                    }
                }
            }
            /* now create obazl adapters for here-switch */
            opam_config_here(); // compiler_version);
        } else {
            if (compiler_version && opam_switch) {
                printf("@opam/xdg/init: only one of -c and -s may be passed\n");
                exit(EXIT_FAILURE);
            }
            /* interrogate system to get xdg dirs */
            xdg_configure();
            printf("xdg_opam_root: %s\n", utstring_body(xdg_opam_root));
            /* config buildfiles etc. in xdg */
            opam_config_xdg(opam_switch);
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
        print_ignore_msg();
    }
    else if (strcmp(basename(argv[0]), "init") == 0) {
        if (compiler_version && opam_switch) {
            printf("@opam/here/init: only one of -c and -s may be passed\n");
            exit(EXIT_FAILURE);
        }
        result = opam_init_here(force, compiler_version, opam_switch);
        if (result == 0)
            print_ignore_msg();
    }
    else if (strcmp(basename(argv[0]), "install") == 0) {
        //FIXME: if -s passed w/o arg, import from current switch
        // or: if no arg passed, prompt for approval to import
        // from current switch.
        // present choice: either one pkg, or a switch
        if (package) {
            opam_install(package);
            print_ignore_msg();
        } else if (opam_switch) {
                printf("installing pkgs from switch %s\n", opam_switch);
                opam_install_here_pkgs(opam_switch);
        } else {
            /* neither -c nor -s passed, but when prompted
               user accepted current switch. install pkgs from
               current switch. */
            /* opam_install_here_pkgs(NULL); */
            printf("xxxxxxxxxxxxxxxx\n");
        }
    }
    /* else if (strcmp(basename(argv[0]), "remove") == 0) { */
    else if (strcmp(basename(argv[0]), "clean") == 0) {
        if (package) {
            opam_remove(package);
            print_ignore_msg();
        } else
            printf("remove command requires -p <pkg> arg\n");
    }
    /* else if (strcmp(basename(argv[0]), "show") == 0) { */
    /*     if (strncmp(basename(dirname(dirname(argv[0]))), "here", 4) == 0) { */
    /*         printf("here xxxxxxxxxxxxxxxx\n"); */
    /*         opam_here_status(); */
    /*     } else { */
    /*         printf("xdg xxxxxxxxxxxxxxxx\n"); */
    /*         opam_xdg_status(); */
    /*     } */
    /* } */
    else if (strcmp(basename(argv[0]), "status") == 0) {
        if (strncmp(basename(dirname(dirname(argv[0]))), "here", 4) == 0) {
            opam_here_status();
        } else {
            printf("xdg xxxxxxxxxxxxxxxx\n");
            opam_xdg_status();
        }
    }
    else printf("Unknown cmd: %s\n", basename(argv[0]));

    dispose_flag_table();

    if (compiler_version) free(compiler_version);
    if (opam_switch) free(opam_switch);

    free(KPM_TABLE);

    /* fprintf(opam_resolver, ")\n"); */

    /* fclose(opam_resolver); */

#ifdef DEBUG
    log_info("opam_main FINISHED");
#endif

    shutdown();
    return EXIT_SUCCESS;
}
