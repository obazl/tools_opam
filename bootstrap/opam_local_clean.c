#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_local_clean.h"

/*
  opam_local_clean

  1. rm ./.obazl.d/opam/local
  2. rm ./_opam
  3. write dummy to ./.obazl.d/opam/local/BOOTSTRAP.bzl
 */
void opam_local_clean(void) {
    if (verbose)
        printf("opam_local_clean\n");
    opam_local_clean_coswitch();

    return;

    opam_local_clean_switch();

}

void opam_local_clean_switch(void) {
    if (verbose)
        printf("opam_local_clean_switch\n");

    char *exe = "rm";
    char *init_argv[] = {
        "rm",
        opam_cmds_verbose? "-vdRf" : "-dRf",
        LOCAL_SWITCH_DIR,
        NULL
    };

    int argc = (sizeof(init_argv) / sizeof(init_argv[0])) - 1;
    int result = spawn_cmd_with_stdout(exe, argc, init_argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch _local)\n");
    }
}

void opam_local_clean_coswitch(void) {
    if (verbose)
        printf("opam_local_clean_coswitch\n");

    char *exe = "rm";
    char *init_argv[] = {
        "rm",
        opam_cmds_verbose? "-vdRf" : "-dRf",
        LOCAL_COSWITCH_ROOT,
        NULL
    };

    int argc = (sizeof(init_argv) / sizeof(init_argv[0])) - 1;
    int result = spawn_cmd_with_stdout(exe, argc, init_argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch _local)\n");
    }

    // write null ./COSWITCH.bzl
    UT_string *bootstrap_filename = NULL;
    utstring_new(bootstrap_filename);
    utstring_printf(bootstrap_filename,
                    "%s", "COSWITCH.bzl");
    if (verbose)
        fprintf(stdout, "Writing null %s\n", utstring_body(bootstrap_filename));
    bootstrap_FILE = fopen(utstring_body(bootstrap_filename), "w");
    if (bootstrap_FILE == NULL) {
        perror(utstring_body(bootstrap_filename));
        exit(EXIT_FAILURE);
    }
    fprintf(bootstrap_FILE, "# generated file - DO NOT EDIT\n");
    fprintf(bootstrap_FILE, "def register():\n");
    fprintf(bootstrap_FILE, "    native.new_local_repository(\n");
    fprintf(bootstrap_FILE, "        name       = \"coswitch\",\n");
    fprintf(bootstrap_FILE, "        path       = \".obazl.d/opam/local\",\n");
    fprintf(bootstrap_FILE, "        build_file_content = \"#\",\n");
    fprintf(bootstrap_FILE, "        workspace_file_content = \"#\"\n");
    fprintf(bootstrap_FILE, "    )\n");
    fclose(bootstrap_FILE);

    // write null .obazl.d/opam/local/BOOTSTRAP.bzl
    mkdir_r(LOCAL_COSWITCH_ROOT);
    utstring_renew(bootstrap_filename);
    utstring_printf(bootstrap_filename,
                    "%s/%s",
                    LOCAL_COSWITCH_ROOT,
                    OPAM_BOOTSTRAP);
    if (verbose)
        fprintf(stdout, "Writing null %s\n", utstring_body(bootstrap_filename));
    bootstrap_FILE = fopen(utstring_body(bootstrap_filename), "w");
    if (bootstrap_FILE == NULL) {
        perror(utstring_body(bootstrap_filename));
        exit(EXIT_FAILURE);
    }
    fprintf(bootstrap_FILE, "# generated file - DO NOT EDIT\n");
    fprintf(bootstrap_FILE, "def bootstrap():\n");
    fprintf(bootstrap_FILE, "    True\n");
    fclose(bootstrap_FILE);
    utstring_free(bootstrap_filename);

    return;                     /* testing */
}
