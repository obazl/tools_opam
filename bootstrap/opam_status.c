#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>

#include "log.h"
#include "opam_status.h"

void display_direntries(DIR *d) {
    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        /* if(direntry->d_type==DT_REG){ */
        /* } */
        if (direntry->d_name[0] != '.') {
            switch (direntry->d_type) {
            case DT_REG:
                printf("\t" WHT "%s\n", direntry->d_name);
                break;
            case DT_DIR:
                printf("\t" BLU "%s\n", direntry->d_name);
                break;
            case DT_LNK:
                printf("\t" MAG "%s\n", direntry->d_name);
                break;
            default:
                printf("\t" RED "%s\n", direntry->d_name);
            }
        }
    }
}

void display_pkg_direntries(DIR *d) {
    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        /* if(direntry->d_type==DT_REG){ */
        /* } */
        if (direntry->d_name[0] != '.') {
            switch (direntry->d_type) {
            case DT_REG:
                printf(WHT " %s" CRESET, direntry->d_name);
                break;
            case DT_DIR:
                printf(BLU " %s" CRESET, direntry->d_name);
                break;
            case DT_LNK:
                printf(MAG " %s" CRESET, direntry->d_name);
                break;
            default:
                printf(RED " %s" CRESET, direntry->d_name);
            }
        }
    }
}

EXPORT void opam_here_switch_list(void)
{
    if (access(HERE_OPAM_ROOT, F_OK) != 0) {
        log_info("Project-local OPAM root '.opam' not found.\n");
        printf("Project-local OPAM root '.opam' not found.\n");
    } else {

        printf(UWHT "OPAM switch:" CRESET "\n");
        errno =0;
        char *coswitch_root = realpath(HERE_OPAM_ROOT, NULL);
        if (errno != 0) {
            fprintf(stderr, "realpath error %s for %s\n",
                    strerror(errno), HERE_OPAM_ROOT);
        }

        printf("root:\t\t\t" BLU "%s" CRESET "\n", coswitch_root);
        free(coswitch_root);
        printf("name:\t\t\t" BLU HERE_SWITCH_NAME CRESET "\n");

        char *exe = "opam";
        int result;

        char *compiler_version = get_compiler_version(NULL);
        fprintf(stdout, "compiler version:\t" BLU "%s" CRESET "\n");

        /* fflush(stdout); */
        /* char *compiler_version_argv[] = { */
        /*     "opam", "var", "ocaml-base-compiler:version", */
        /*     "--root",   HERE_OPAM_ROOT, */
        /*     "--switch", HERE_SWITCH_NAME, */
        /*     NULL // null-terminated array of ptrs to null-terminated strings */
        /* }; */

        /* int argc = (sizeof(compiler_version_argv) */
        /*             / sizeof(compiler_version_argv[0])) - 1; */
        /* result = spawn_cmd(exe, argc, compiler_version_argv); */
        /* if (result != 0) { */
        /*     fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch here)\n"); */
        /*     exit(EXIT_FAILURE); */
        /*     /\* } else { *\/ */
        /*     /\*     printf("%s\n", result); *\/ */
        /* } */

        char *argv[] = {
            "opam", "var",
            "--root", HERE_OPAM_ROOT,
            "--switch", HERE_SWITCH_NAME,
            NULL // null-terminated array of ptrs to null-terminated strings
        };

        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch here)\n");
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("%s\n", result); */
        }

    /* printf("Begining OPAM processor output:\n"); */

        printf(CRESET "\n");
        printf(UWHT "OPAM pkgs:" CRESET "\n");

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

