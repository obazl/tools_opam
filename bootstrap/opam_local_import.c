#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_local_import.h"

/*
  import .obazl.d/opam/local.manifest
 */
EXPORT void opam_local_import(void)
{
    if (debug) {
        log_debug("opam_import");
        printf("opam_import\n");
    }
    if (access(LOCAL_SWITCH_MANIFEST, R_OK) != 0) {
        log_error(RED "ERROR: " CRESET "missing manifest: %s\n",
                  LOCAL_SWITCH_MANIFEST);
        fprintf(stderr, RED "ERROR: " CRESET "missing manifest: %s\n",
               LOCAL_SWITCH_MANIFEST);
        exit(EXIT_FAILURE);
    }

    if (verbose)
        printf("Importing %s\n", LOCAL_SWITCH_MANIFEST);

    int result;

    char *exe = "opam";
    char *argv[] = {
        "opam", "switch",
        "--cli=2.1",
        "--yes",
        "import",
        LOCAL_SWITCH_MANIFEST,
        "--switch", ".",
        /* opam_cmds_debug? "--debug": "", */
        opam_cmds_verbose? "--verbose": NULL,
        NULL
    };

    int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
    if (verbose)
        result = spawn_cmd_with_stdout(exe, argc, argv);
    else
        result = spawn_cmd(exe, argc, argv);
    if (result != 0) {
        fprintf(stderr, RED "ERROR: " CRESET "opam import\n");
        exit(EXIT_FAILURE);
    }
}

