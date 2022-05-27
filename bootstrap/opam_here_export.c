#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_here_export.h"

EXPORT void opam_here_export(char *manifest)
{
    log_debug("opam_here_export: %s", manifest);

    UT_string *manifest_name;
    utstring_new(manifest_name);
    if (manifest) {
        utstring_printf(manifest_name, "%s", manifest);
    } else {
        /* char *workspace = get_workspace_name(); */
        /* printf("wsn: '%s'\n", workspace); */
        /* utstring_printf(manifest_name, "%s.opam.manifest", workspace); */
        utstring_printf(manifest_name, "%s/here.packages",
                        HERE_COSWITCH_ROOT);
    }

    char *exe;
    int result;

    if (access(HERE_OPAM_ROOT, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM root '" HERE_OPAM_ROOT "' not found.");
        printf("Project-local OPAM root '" HERE_OPAM_ROOT "' not found.\n");
        exit(EXIT_FAILURE);
    } else {
        exe = "opam";
        char *argv[] = {
            "opam", "switch",
            "export",
            opam_cmds_verbose? "--verbose": "",
            opam_cmds_debug? "--debug": "",
            "--cli=2.1",
            "--root=./" HERE_OPAM_ROOT,
            "--switch", HERE_SWITCH_NAME,
            "--freeze",
            /* "--full", */
            utstring_body(manifest_name),
            NULL
        };
        utstring_body(manifest_name);
        printf("Exporting %s\n", utstring_body(manifest_name));
        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: spawn_cmd(opam var --root .opam --switch _here)\n");
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("export result: %s\n", result); */
        }
    }
    // chmod(HERE_COSWITCH_ROOT "/here.compiler", S_IRUSR|S_IRGRP|S_IROTH);
}

