//FIXME: support -j (--jsoo-enable) flag

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

#if INTERFACE
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#endif
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cwalk.h"
#include "gopt.h"
#include "libs7.h"
#include "liblogc.h"
/* #include "semver.h" */
/* #include "utarray.h" */
/* #include "uthash.h" */
#include "utstring.h"
/* #include "librunfiles.h" */

#include "main.h"

bool bazel_env;

extern char *switch_name;
/* static char *coswitch_name; // may be "local" */

#define DEBUG_LEVEL debug_opam
int  DEBUG_LEVEL;
#define TRACE_FLAG trace_opam
bool TRACE_FLAG;

extern int  debug_findlibc;
extern bool trace_findlibc;

extern int  coswitch_debug;
extern bool coswitch_trace;

bool quiet;
bool verbose;
int  verbosity;

extern s7_scheme *s7;

enum OPTS {
    OPT_PKG = 0,
    OPT_SWITCH_ID,
    FLAG_CLEAN,
    FLAG_DEBUG,
    FLAG_TRACE,
    FLAG_DEBUG_FINDLIBC,
    FLAG_TRACE_FINDLIBC,
    FLAG_VERBOSE,
    FLAG_VERSION,
    FLAG_QUIET,
    FLAG_HELP,
    LAST
};

void _print_usage(void) {
    if (bazel_env) {
        printf("Usage:\t$ bazel run @coswitch//new [args]\n");
    } else {
        printf("Usage:\t$ coswitch [args]\n");
    }

    printf("\t-s, --switch <name>\t\tOPAM switch to use. Default: current switch\n");

    /* printf("Flags\n"); */
    /* printf("\t-j, --jsoo\t\t\tImport Js_of_ocaml resources.\n"); */
    /* printf("\t-c, --clean\t\t\tClean coswitch and reset to uninitialized state. (temporarily disabled)\n"); */

    printf("\t-q, --quiet\t\t\tSuppress all stdout msgs.\n");
    printf("\t-v, --verbose\t\t\tEnable verbosity. Repeatable.\n");
    printf("\t    --version\t\t\tPrint version identifier.\n");

    printf("\t-d, --debug\t\t\tEnable debug flags. Repeatable.\n");
    printf("\t-t, --trace\t\t\tEnable all trace flags (debug only).\n");
}

static struct option options[] = {
    /* 0 */
    [OPT_PKG] = {.long_name="pkg",.short_name='p',
                 .flags=GOPT_ARGUMENT_REQUIRED},
    [OPT_SWITCH_ID] = {.long_name="switch",
        .flags=GOPT_ARGUMENT_REQUIRED},
    /* [OPT_SWITCH_LIB] = {.long_name="switch-lib", */
    /*     .flags=GOPT_ARGUMENT_REQUIRED}, */
    [FLAG_CLEAN] = {.long_name="clean",.short_name='c',
                    .flags=GOPT_ARGUMENT_FORBIDDEN},
    [FLAG_DEBUG] = {.long_name="debug",.short_name='d',
                    .flags=GOPT_ARGUMENT_FORBIDDEN | GOPT_REPEATABLE},
    [FLAG_DEBUG_FINDLIBC] = {
        .long_name="debug-findlibc",
        .flags=GOPT_ARGUMENT_FORBIDDEN | GOPT_REPEATABLE},
    [FLAG_TRACE] = {.long_name="trace",.short_name='t',
                    .flags=GOPT_ARGUMENT_FORBIDDEN},
    [FLAG_TRACE_FINDLIBC] = {
        .long_name="trace-findlibc",
        .flags=GOPT_ARGUMENT_FORBIDDEN},
    [FLAG_VERBOSE] = {.long_name="verbose",.short_name='v',
                      .flags=GOPT_ARGUMENT_FORBIDDEN | GOPT_REPEATABLE},
    [FLAG_VERSION] = {.long_name="version",
                      .flags=GOPT_ARGUMENT_FORBIDDEN },
    [FLAG_QUIET] = {.long_name="quiet",.short_name='q',
                    .flags=GOPT_ARGUMENT_FORBIDDEN},
    [FLAG_HELP] = {.long_name="help",.short_name='h',
                   .flags=GOPT_ARGUMENT_FORBIDDEN},
    [LAST] = {.flags = GOPT_LAST}
};

void _set_options(struct option options[])
{
    if (options[FLAG_HELP].count) {
        _print_usage();
        exit(EXIT_SUCCESS);
    }

    if (options[FLAG_VERBOSE].count) {
        /* printf("verbose ct: %d\n", options[FLAG_VERBOSE].count); */
        verbose = true;
        verbosity = options[FLAG_VERBOSE].count;
    }

    if (options[FLAG_DEBUG].count) {
        debug_opam = options[FLAG_DEBUG].count;
        coswitch_debug = debug_opam;
    }
    if (options[FLAG_TRACE].count) {
        trace_opam = true;
        coswitch_trace = trace_opam;
    }
    if (options[FLAG_DEBUG_FINDLIBC].count) {
        debug_findlibc = options[FLAG_DEBUG].count;
    }
    if (options[FLAG_TRACE_FINDLIBC].count) {
        trace_findlibc = true;
    }

}

extern char **environ;

int main(int argc, char *argv[])
{
    argc = gopt(argv, options);
    (void)argc;

    gopt_errors (argv[0], options);

    _set_options(options);

    if (options[FLAG_VERSION].count) {
        /* fprintf(stdout, "%s\n", config_pkg_version); */
        exit(EXIT_SUCCESS);
    }
    /* dump env vars: */
    /* log_debug("ENV"); */
    /* int i = 0; */
    /* while(environ[i]) { */
    /*     log_info("%s", environ[i++]); */
    /* } */

    if (options[FLAG_QUIET].count) {
        quiet = true;
    }

    if (!options[OPT_PKG].count) {
        log_error("-p <pkg> or --pkg <pkg> required");
        exit(EXIT_FAILURE);
    }
    if (!options[OPT_SWITCH_ID].count) {
        log_error("--switch <switch id> required");
        exit(EXIT_FAILURE);
    }
    /* if (!options[OPT_SWITCH_LIB].count) { */
    /*     log_error("--switch-lib <path> required"); */
    /*     exit(EXIT_FAILURE); */
    /* } */

    char *cwd = getcwd(NULL, 0);
    log_debug("cwd: '%s'", cwd);
    /* size_t length; */
    /* cwk_path_get_dirname(cwd, &length); */
    /* log_debug("cwd dirname is: '%.*s'", (int)length, cwd); */
    /* UT_string *dir; */
    /* utstring_new(dir); */
    /* utstring_printf(dir, "%.*stools_opam+", */
    /*                 (int)length, cwd); */
    /* log_debug("runfiles: %s", utstring_body(dir)); */

    /* for reading dune-project files */
    s7 = libs7_init(argv[0]);

    /* this uses dlsym to fine libdune_s7_init,
     and should work with either static or dso lib */
    libs7_load_plugin(s7, "dune");

    /* rf_init(utstring_body(dir)); */
    /* rf_init(argv[0]); */
    LOG_INFO(0, "BAZEL_CURRENT_REPOSITORY: %s", BAZEL_CURRENT_REPOSITORY);
    /* LOG_INFO(0, "rf_root: %s", rf_root()); */

    opam_pkg_handler(options[OPT_SWITCH_ID].argument,
                     options[OPT_PKG].argument);
                     /* options[OPT_SWITCH_LIB].argument, */
    LOG_INFO(0, "cwd: %s", getcwd(NULL, 0));
    TRACE_EXIT;
}

    /* UT_string *dst_file; */
    /* utstring_new(dst_file); */
    /* utstring_printf(dst_file, "%s/TESTMOD", */
    /*                 //options[OPT_PKG].argument */
    /*                 "."); */
    /* FILE *ostream = fopen(utstring_body(dst_file), "w"); */
    /* if (ostream == NULL) { */
    /*     log_error("%s", strerror(errno)); */
    /*     perror(utstring_body(dst_file)); */
    /*     exit(EXIT_FAILURE); */
    /* } */
    /* fprintf(ostream, "## generated file - DO NOT EDIT\n\n"); */
    /* fprintf(ostream, "module(\n"); */
    /* fprintf(ostream, "    name = \"test\", version = \"0.0.0\",\n"); */
    /* fprintf(ostream, "    compatibility_level = 0,\n"); */
    /* fprintf(ostream, "    bazel_compatibility = [\">=8.0.0\"]\n"); */
    /* fprintf(ostream, ")\n"); */
    /* fprintf(ostream, "\n"); */
    /* fclose(ostream); */

    /* fprintf(stdout, "dep01\n"); */
    /* fprintf(stdout, "dep02\n"); */
    /* fprintf(stdout, "cwd: %s\n", cwd); */

