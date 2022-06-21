#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

#if INTERFACE
#ifdef LINUX
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#endif

#include <unistd.h>

#include "log.h"
#include "utarray.h"
#include "utstring.h"

#if INTERFACE
#include "utstring.h"
#endif

#include "opam_lexer.h"
#include "opam_lex.h"

int main(int argc, char *argv[])
{
    int opt;

    UT_string *opam_file;
    utstring_new(opam_file);

    while ((opt = getopt(argc, argv, "f:hv")) != -1) {
        switch (opt) {
        case 'f':
            /* BUILD.bazel or BUILD file */
            utstring_printf(opam_file, "%s", optarg);
            break;
        case 'h':
            log_info("Help: ");
            exit(EXIT_SUCCESS);
        default:
            log_error("Usage: %s [-f] [opamfile]", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (utstring_len(opam_file) == 0) {
        log_error("-f <opamfile> must be provided.");
        exit(EXIT_FAILURE);
    }

    char *wd = getenv("BUILD_WORKING_DIRECTORY");
    if (wd) {
        /* we launched from bazel workspace, cd to launch dir */
        chdir(wd);
    }

    opam_lex_file(utstring_body(opam_file));
    /* UT_array *result = opam_lex_file(utstring_body(opam_file)); */

    /* UT_array *result = sealark_lex_string("'hello'\n#cmt1\n"); */

    log_debug("main RESULT dump:");
    /* dump_nodes(result); */
}
