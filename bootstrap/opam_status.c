#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>

#include "log.h"
#include "opam_status.h"

EXPORT void opam_here_switch_status(void)
{
    printf("@opam//here/status\n");
    printf("\troot:   " HERE_OPAM_ROOT "\n");
    printf("\tswitch: " HERE_SWITCH_NAME "\n");
    log_info("opam_here_switch_status");

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
            fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch here)\n");
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

/*
  status of .obazl.d/opam/here
 */
EXPORT void obazl_here_status(void)
{
    log_info("obazl_here_status");

    printf("WORKSPACEs:\n");

    /* if (access(HERE_SWITCH_BAZEL_ROOT, F_OK) != 0) { */
    /*     log_info("Project-local obazl root '" */
    /*              HERE_SWITCH_BAZEL_ROOT */
    /*              "' not found.\n"); */
    /*     fprintf(stdout, "Project-local obazl root '" */
    /*             HERE_SWITCH_BAZEL_ROOT */
    /*             "' not found.\n"); */
    /* } else { */
    /*     fprintf(stdout, "Found project-local obazl root '" */
    /*             HERE_SWITCH_BAZEL_ROOT "'\n"); */
    /* } */

    if (access(HERE_OBAZL_OPAM_WSS_OCAML, F_OK) != 0) {
        log_info("Project-local obazl root '"
                 HERE_OBAZL_OPAM_WSS_OCAML
                 "' not found.\n");
        fprintf(stdout, "Project-local obazl opam root '"
                HERE_OBAZL_OPAM_WSS_OCAML
                "' not found.\n");
    } else {

        //FIXME: functionize
        fprintf(stdout, "toolchain: "
                HERE_OBAZL_OPAM_WSS_OCAML
                "\n");

        DIR *d = opendir(HERE_OBAZL_OPAM_WSS_OCAML);
        if (d == NULL) {
            fprintf(stderr, "Unable to opendir "
                    HERE_OBAZL_OPAM_WSS_OCAML "\n");
            return;
        }

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
                    printf("\t" CYN "%s\n", direntry->d_name);
                    break;
                case DT_LNK:
                    printf("\t" MAG "%s\n", direntry->d_name);
                    break;
                default:
                        printf("\t" RED "%s\n", direntry->d_name);
                }
            }
        }
        // stublibs
        // opam pkgs
    }
}

EXPORT void opam_here_status(void)
{
    opam_here_switch_status();
    obazl_here_status();
}

/* **************************************************************** */
EXPORT void opam_xdg_status(void)
{
    /* opam_switch_status(); */
}
