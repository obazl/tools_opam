#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_local_export.h"

/* export local OPAM switch to .obazl.d/opam/local/opam.manifest */
EXPORT void opam_local_export(void)
{
    log_debug("opam_local_export");

    if (access(LOCAL_SWITCH_DIR, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM local dir '" LOCAL_SWITCH_DIR "' not found.");
        printf("OPAM local dir '" LOCAL_SWITCH_DIR "' not found.\n");
        exit(EXIT_FAILURE);
    }

    mkdir_r(LOCAL_COSWITCH_ROOT);

    char *exe;
    int result;

    exe = "opam";
    char *argv[] = {
        "opam", "switch", "export",
        "--cli=2.1",
        LOCAL_SWITCH_MANIFEST,
        /* opam_cmds_verbose? "--verbose": "", */
        /* opam_cmds_debug? "--debug": "", */
        NULL
    };

    if (verbose)
        printf("Exporting %s\n", LOCAL_SWITCH_MANIFEST);

    int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
    result = spawn_cmd(exe, argc, argv);
    if (result != 0) {
        log_error("FAIL: local export\n");
        fprintf(stderr, "FAIL: local export\n");
        exit(EXIT_FAILURE);
    }
}

