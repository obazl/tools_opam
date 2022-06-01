#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_local_export.h"

EXPORT void opam_local_export(char *manifest)
{
    log_debug("opam_local_export: %s", manifest);

    UT_string *manifest_name;
    utstring_new(manifest_name);
    if (manifest) {
        utstring_printf(manifest_name, "%s", manifest);
    } else {
        /* char *workspace = get_workspace_name(); */
        /* printf("wsn: '%s'\n", workspace); */
        /* utstring_printf(manifest_name, "%s.opam.manifest", workspace); */
        utstring_printf(manifest_name, "%s", LOCAL_SWITCH_MANIFEST);
    }

    char *exe;
    int result;

    if (access(LOCAL_SWITCH_DIR, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM local '" LOCAL_SWITCH_DIR "' not found.");
        printf("OPAM local dir '" LOCAL_SWITCH_DIR "' not found.\n");
        exit(EXIT_FAILURE);
    }

    exe = "opam";
    char *argv[] = {
        "opam", "switch", "export",
        "--cli=2.1",
        utstring_body(manifest_name),
        /* opam_cmds_verbose? "--verbose": "", */
        /* opam_cmds_debug? "--debug": "", */
        NULL
    };
    utstring_body(manifest_name);
    /* if (verbose) */
    /*     printf("Exporting %s\n", utstring_body(manifest_name)); */
    int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
    result = spawn_cmd(exe, argc, argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: spawn_cmd(opam var --root .opam --switch _local)\n");
        exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("export result: %s\n", result); */
    }
// chmod(LOCAL_COSWITCH_ROOT "/local.compiler", S_IRUSR|S_IRGRP|S_IROTH);
}

