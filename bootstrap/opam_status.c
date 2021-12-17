#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "opam_status.h"

EXPORT void opam_status(void) // char *_opam_switch, char *obazl_opam_root)
{
    printf("@opam//here/status\n");
    printf("\troot:   " HERE_OPAM_ROOT "\n");
    printf("\tswitch: " HERE_SWITCH_NAME "\n");
    log_info("opam_status");

    if (access(HERE_OPAM_ROOT, F_OK) != 0) {
        log_info("Project-local OPAM root '.opam' not found.\n");
        printf("Project-local OPAM root '.opam' not found.\n");
    } else {

        char *exe;
        int result;

        exe = "opam";
        char *argv[] = {
            "opam", "var",
            "--root", HERE_OPAM_ROOT,
            "--switch", HERE_SWITCH_NAME,
            NULL // null-terminated array of ptrs to null-terminated strings
        };

        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch obazl)\n");
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("%s\n", result); */
        }

        //*var_argv = NULL; //FIXME: otherwise run_cmd gets confused
        char *list_argv[] = {
            "opam", "list",
            "--root",   HERE_OPAM_ROOT,
            "--switch", HERE_SWITCH_NAME,
            "--columns", "name,version",
            NULL
        };

        argc = (sizeof(list_argv) / sizeof(list_argv[0])) - 1;
        result = spawn_cmd(exe, argc, list_argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: run_cmd(opam list --root .opam --switch obazl)\n");
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("RESULT: %s\n", result); */
        }

    }
}
