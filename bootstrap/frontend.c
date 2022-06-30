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
extern int errnum;
int rc;

char work_buf[PATH_MAX];

/* UT_array *opam_packages; */

char coqlib[PATH_MAX];

struct buildfile_s {
    char *name;                 /* set from strndup optarg; must be freed */
    char *path;                 /* set from getenv; do not free */
    UT_hash_handle hh;
} ;
struct buildfile_s *buildfiles = NULL;

struct fileset_s *filesets = NULL;

/* struct package_s *packages = NULL; */

#if EXPORT_INTERFACE
#define XDG   0
#define HERE  1
#define LOCL 2  // avoid clash with makeheaders LOCAL
#endif

void print_ignore_msg(void)
{
    printf("(obazl says: you can ignore any advice about running eval to update the shell environment, so long as you use @opam commands to control your here-switch.)\n");
}

/* ctx: 1 == here, 0 == xdg  */
EXPORT int opam_main(int argc, char *argv[], int oswitch) // bool here)
{

    /* printf("OPAM_MAIN ARGC: %d\n", argc); */
    /* for (int i=0; i<argc; i++) { */
    /*     printf("OPAM_MAIN ARGV[%d]: %s\n", i, argv[i]); */
    /* } */

    char *opam_switch = NULL;
    char *compiler_version = NULL;

    bool force = false;
    /* char *deps_root = NULL; */
    char *manifest = NULL;

    char *opts = "c:dDfhom:s:vVx";
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
        case 'D':
            opam_cmds_debug = true;
            /* dry_run = true; */
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
        /* case 'p': */
        /*     package = optarg; */
        /*     break; */
        /* case 'r': */
        /*     char *deps_root = optarg; */
        /*     break; */
        case 's':
            printf("option s (switch): %s\n", optarg);
            opam_switch = strndup(optarg, PATH_MAX);
            break;
        case 'V':
            opam_cmds_verbose = true;
            break;
        case 'v':
            verbose = true;
            break;
        case 'x':
            expunge = true;
            break;

            /* -h should have been handled by cmd handlers */
            /* case 'h': */
        /*     print_usage(argv[0]); */
        /*     exit(EXIT_SUCCESS); */
        /*     break; */
        default:
            ;
        }
    }

    if (dry_run)
        printf(RED "DRY RUN - DRY RUN - DRY RUN - DRY RUN" CRESET "\n");

#if defined(DEBUG_TRACE)
    log_debug("DEBUGGING");
#endif

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

    if (oswitch == XDG) xdg_configure();

    if (oswitch==HERE && strncmp(basename(argv[0]), "clean", 5) == 0) {
        opam_here_clean();
    }
    else if (oswitch==LOCL && strncmp(basename(argv[0]), "clean", 5) == 0) {
        if (expunge)
            printf("Expunging\n");
        else
            printf("Cleaning\n");
        opam_local_clean();
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "clone", 5) == 0) {
        int index;
        char *package = NULL;
        for (index = optind; index < argc; index++) {
            package = strdup(argv[index]);
        }
        if (package != NULL) {
            printf("clone: %s\n", package);
            opam_here_opam_clone(package);
            free(package);
        } else {
            /* should not happen */
            fprintf(stderr, "ERROR: missing pkg arg\n");
            /* printf("remove opam_switch: %s\n", opam_switch); */
            /* opam_install_here(package); */
        }
        /* copy from switch arg */
        /*     /\* opam_install_here_pkgs(NULL); *\/ */
    }
    else if (oswitch==HERE
             && strncmp(basename(argv[0]), "refresh", 7) == 0) {
        if (access(HERE_COSWITCH_ROOT, F_OK) == 0) {
            if (verbose)
                printf("Wiping existing .obazl.d/opam\n");
            run_cmd("rm -rf " HERE_COSWITCH_ROOT, true); /* .obazl.d/opam */
        }

        /* now create obazl adapters for here-switch only, not pkgs */
        opam_here_refresh(); // compiler_version);
    }
    else if (oswitch==LOCL
             && strncmp(basename(argv[0]), "refresh", 7) == 0) {
        /* now create obazl adapters for here-switch only, not pkgs */
        opam_local_refresh(); // compiler_version);
    }
    else if (oswitch==XDG && strncmp(basename(argv[0]), "refresh", 7) == 0) {
        /* xdg_configure();        /\* config dir vars *\/ */
        int ct = 0;
        for (int i = optind; i < argc; i++) {
            if (argv[i] != NULL) {
                if (verbose)
                    printf("refreshing shared coswitch '%s'\n",
                           argv[i]);
                opam_xdg_refresh(argv[i]);
                ct++;
            }
        }
        /* if (ct == 0) {/\* no args *\/ */
        /*     // refresh current OPAM switch */
        /*     opam_xdg_refresh(NULL); */
        /* } */
        /* int index; */
        /* char *coswitch = NULL; */
        /* for (index = optind; index < argc; index++) { */
        /*     coswitch = strdup(argv[index]); */
        /* } */
        /* printf("coswitch: %s\n", coswitch); */
        /* if (coswitch != NULL) { */
        /*     opam_xdg_refresh(coswitch); */
        /*     free(coswitch); */
        /* } else { */
        /*     opam_xdg_refresh(opam_switch); */
        /* } */
    }
    /* else if (oswitch && strncmp(basename(argv[0]), "ingest", 6) == 0) { */
    /*     if (compiler_version && opam_switch) { */
    /*         printf("@opam/here/init: only one of -c and -s may be passed\n"); */
    /*         exit(EXIT_FAILURE); */
    /*     } */

    /*     /\* construct the opam here switch *\/ */
    /*     result = opam_init_here(force, compiler_version, opam_switch); */
    /*     if (result == -1) { */
    /*         /\* fprintf(stdout, "cancelling\n"); *\/ */
    /*         exit(EXIT_SUCCESS); */
    /*     } else { */
    /*         if (result == 0) { */
    /*             print_ignore_msg(); */
    /*             /\* if user passed -c, then do not install pkgs; if */
    /*                user passed -s (opam_switch), then install pkgs */
    /*                from the switch. *\/ */
    /*             if (opam_switch) */
    /*                 opam_install_here_pkgs(opam_switch); */
    /*         } else { */
    /*             if (result == 1) { */
    /*                 /\* neither -c nor -s passed, but when prompted */
    /*                    user accepted current switch. install pkgs from */
    /*                    current switch. *\/ */
    /*                 print_ignore_msg(); */
    /*                 opam_install_here_pkgs(NULL); */
    /*             } */
    /*         } */
    /*     } */
    /*     /\* now create obazl adapters for here-switch *\/ */
    /*     opam_config_here(); // compiler_version); */
    /* } */
    /* else if (here==XDG && strncmp(basename(argv[0]), "ingest", 6) == 0) { */
    /*     if (compiler_version && opam_switch) { */
    /*         printf("@opam/xdg/init: only one of -c and -s may be passed\n"); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /*     /\* interrogate system to get xdg dirs *\/ */
    /*     xdg_configure(); */
    /*     printf("xdg_coswitch_root: %s\n", utstring_body(xdg_coswitch_root)); */
    /*     opam_xdg_refresh(opam_switch); */
    /* } */
    /* else if (strcmp(basename(argv[0]), "deps") == 0) { */
    /*     opam_deps(deps_root); */
    /* } */
    /* else if (strncmp(basename(argv[0]), "export", 6) == 0) { */
    else if (oswitch==HERE && strncmp(basename(argv[0]), "export", 6) == 0) {
        opam_here_export(manifest);
    }
    else if (oswitch==LOCL && strncmp(basename(argv[0]), "export", 6) == 0) {
        opam_local_export();
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "import", 6) == 0) {
        opam_import(manifest);
        /* print_ignore_msg(); */
    }
    else if (oswitch==LOCL && strncmp(basename(argv[0]), "import", 6) == 0) {
        opam_local_import();
        /* print_ignore_msg(); */
    }
    else if (oswitch==LOCL && strncmp(basename(argv[0]), "create", 6) == 0) {
        if (compiler_version && opam_switch) {
            fprintf(stderr,
                    "@opam//local:create: only one of -c and -s may be passed\n");
            exit(EXIT_FAILURE);
        }
        /* result = opam_local_create(); */
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "create", 6) == 0) {
        if (compiler_version && opam_switch) {
            fprintf(stderr,
                    "@opam//here:create: only one of -c and -s may be passed\n");
            exit(EXIT_FAILURE);
        }
        /* int result = */
        opam_here_create();
        /* result = opam_here_create(force, compiler_version, opam_switch); */
        /* if (result == 0) */
            /* print_ignore_msg(); */
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "init", 4) == 0) {
        /* HIDDEN, for use during development */
        if (compiler_version && opam_switch) {
            printf("@opam/here/init: only one of -c and -s may be passed\n");
            exit(EXIT_FAILURE);
        }
        /* int result = */
        opam_here_opam_init(force, compiler_version, opam_switch);
        /* if (result == 0) */
            /* print_ignore_msg(); */
    }
    else if (oswitch==LOCL && strncmp(basename(argv[0]), "init", 4) == 0) {
        //FIXME: verify at most one arg
        /* int index; */
        for (int i = optind; i < argc; i++) {
            if (argv[i] != NULL) {
                if (verbose)
                    printf("here switch: installing: %s\n", argv[i]);
                opam_local_init(argv[i]);
                return EXIT_SUCCESS;
            }
        }
        opam_local_init(NULL);
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "reinit", 4) == 0) {
        if (compiler_version && opam_switch) {
            printf("@opam/here/init: only one of -c and -s may be passed\n");
            exit(EXIT_FAILURE);
        }
        /* int result = */
        opam_here_reinit(force, compiler_version, opam_switch);
        /* opam_import(NULL); */

        /* result = opam_here_reinit(force, compiler_version, opam_switch); */
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "install", 7) == 0) {
        /* install only valid for here switch */
        /* int index; */
        for (int i = optind; i < argc; i++) {
            if (argv[i] != NULL) {
                if (verbose)
                    printf("here switch: installing: %s\n", argv[i]);
                opam_here_opam_install_pkg(argv[i]);
                /* opam_install_here(package); */
            }
        }
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "remove", 6) == 0) {
        /* @opam//here/opam:remove -- pkg */
        for (int i = optind; i < argc; i++) {
            if (argv[i] != NULL) {
                printf("remove here opam pkg: %s\n", argv[i]);
                opam_here_opam_remove_pkg(argv[i]);
            }
        }
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "expunge", 6) == 0) {
        /* @opam//here:expunge */
        opam_here_expunge();
    }
    else if (oswitch==XDG && strncmp(basename(argv[0]), "expunge", 7) == 0) {
        /* @opam//coswitch:expunge -- coswitch */
        int index;
        char *coswitch = NULL;
        for (index = optind; index < argc; index++) {
            coswitch = strdup(argv[index]);
        }
        if (coswitch != NULL) {
            printf("remove coswitch: %s\n", coswitch);
            opam_coswitch_remove(coswitch);
            free(coswitch);
        } else {
            /* should not happen */
            printf("remove opam_switch: %s\n", opam_switch);
            opam_coswitch_remove(opam_switch);
        }
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "list", 4) == 0) {
        opam_here_list(); //FIXME: rename: obazl_here_list
    }
    else if (oswitch==LOCL && strncmp(basename(argv[0]), "list", 4) == 0) {
        opam_local_list();
    }
    else if (oswitch==XDG && strncmp(basename(argv[0]), "list", 4) == 0) {
        int index;
        char *coswitch = NULL;
        for (index = optind; index < argc; index++) {
            coswitch = strdup(argv[index]);
        }
        /* printf("coswitch: %s\n", coswitch); */
        if (coswitch != NULL) {
            opam_xdg_list(coswitch);
            free(coswitch);
        } else
            opam_xdg_list(opam_switch);
    }
    else if (oswitch==XDG && strncmp(basename(argv[0]), "coswitch", 8) == 0) {
        opam_xdg_list(opam_switch);
    }
    else if (strncmp(basename(argv[0]), "show", 4) == 0) {
        opam_coswitch_show();
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "set", 3) == 0) {
        opam_coswitch_set("here");
    }
    else if (oswitch==LOCL && strncmp(basename(argv[0]), "set", 3) == 0) {
        //FIXME: verify no args passed
        opam_local_coswitch_set();
    }
    else if (oswitch==XDG && strncmp(basename(argv[0]), "set", 3) == 0) {
        int index;
        int ct = 0;
        char *coswitch = NULL;
        for (index = optind; index < argc; index++) {
            coswitch = strdup(argv[index]);
            ct++;
        }
        if (ct == 0) {
            fprintf(stderr,
                    RED "ERROR: " CRESET "One arg required: switch name\n");
            exit(EXIT_FAILURE); //FIXME
        }
        if (ct > 1) {
            fprintf(stderr,
                    RED "ERROR: " CRESET "Only one arg allowed: %d\n", ct);
            exit(EXIT_FAILURE); //FIXME
        }
        if (coswitch != NULL) {
            printf("set coswitch: %s\n", coswitch);
            opam_coswitch_set(coswitch);
            free(coswitch);
        /* } else { */
        /*     /\* should not happen *\/ */
        /*     printf("set opam_switch: %s\n", coswitch); */
        /*     opam_coswitch_set(opam_switch); */
        }
    }
    else if (oswitch==HERE && strncmp(basename(argv[0]), "test", 3) == 0) {
        opam_test();
    }
    else printf(RED "Unknown cmd: %s" CRESET "\n", basename(argv[0]));

    dispose_flag_table();

    if (compiler_version) free(compiler_version);
    if (opam_switch) free(opam_switch);

    free(KPM_TABLE);

    /* fprintf(opam_resolver, ")\n"); */

    /* fclose(opam_resolver); */

#if defined(DEBUG_TRACE)
    log_info("opam_main FINISHED");
#endif

    shutdown();
    return EXIT_SUCCESS;
}
