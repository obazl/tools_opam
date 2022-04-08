#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_init_here.h"

/* opam_install_here_pkgs */
EXPORT void opam_install_here_pkgs(char *opam_switch_name)
{
    printf("opam_install_here_pkgs\n");
    log_debug("opam_install_here_pkgs");

    UT_string *cmd;
    utstring_new(cmd);

    if (opam_switch_name == NULL) {
        // get current switch name?
        utstring_printf(cmd, "opam switch export %s/here.packages",
                        HERE_OBAZL_ROOT);
    } else {
        utstring_printf(cmd,
                        "opam switch export %s/here.packages --switch %s",
                        HERE_OBAZL_ROOT,
                        opam_switch_name); // , opam_switch_name);
    }

    errno = 0;
    char *result = run_cmd(utstring_body(cmd));
    if (result == NULL) {
        fprintf(stderr, "export: %s", strerror(errno));
        log_error("export: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    opam_import(NULL);
}

/* ********************* */
LOCAL int _opam_init(void)
{
    char *exe;
    int result;

    if (verbose)
        log_info("initializing opam root at: .opam");

    exe = "opam";
    char *init_argv[] = {
        "opam", "init",
        "--cli=2.1",
        "--root=./.opam",
        "--bare",
        "--no-setup", "--no-opamrc",
        "--bypass-checks",
        "--yes", "--quiet",
        /* dry_run? "--dry-run" : NULL, */
        NULL
    };

    int argc = (sizeof(init_argv) / sizeof(init_argv[0])) - 1;
    result = spawn_cmd(exe, argc, init_argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch _here)\n");
    }
    return result;
}

/* **************************************************************** */
LOCAL int _opam_create_switch(char *compiler_version)
{
    /* char *ws_name = get_workspace_name(); */
    /* UT_string *desc; */
    /* utstring_new(desc); */
    /* utstring_printf(desc, "'here-switch for workspace %s'", ws_name); */

    char *exe;
    int result;

    if (verbose)
        log_info("creating switch with compiler version %s", compiler_version);

    exe = "opam";
    char *switch_argv[] = {
        "opam", "switch",
        "--cli=2.1",
        "--root=./.opam",
        /* "--description", utstring_body(desc), */
        "create", HERE_SWITCH_NAME, // "obazl",
        compiler_version,
        /* dry_run? "--dry-run" : NULL, */
        NULL
    };

    int argc = (sizeof(switch_argv) / sizeof(switch_argv[0])) - 1;
    result = spawn_cmd(exe, argc, switch_argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: run_cmd(opam switch --root .opam create _here)\n");
    }

    if (!dry_run) {
        FILE *here_compiler_file = fopen(HERE_COMPILER_FILE, "w");
        if (here_compiler_file == NULL) {
            if (errno == EACCES) {
                int rc = unlink(HERE_COMPILER_FILE);
                if (rc == 0) {
                    /* log_debug("unlinked existing %s\n", HERE_COMPILER_FILE); */
                    here_compiler_file = fopen(HERE_COMPILER_FILE, "w");
                    if (here_compiler_file == NULL) {
                        perror(HERE_COMPILER_FILE);
                        exit(EXIT_FAILURE);
                    }
                    /* log_debug("opened %s for w\n", HERE_COMPILER_FILE); */
                } else {
                    perror(HERE_COMPILER_FILE);
                    exit(EXIT_FAILURE);
                }
            } else {
                perror(HERE_COMPILER_FILE);
                exit(EXIT_FAILURE);
            }
        }
        fprintf(here_compiler_file, "%s\n", compiler_version);
        fclose(here_compiler_file);
        chmod(HERE_COMPILER_FILE, S_IRUSR|S_IRGRP|S_IROTH);
        if (debug)
            log_debug("wrote " HERE_COMPILER_FILE);
    }
    return result;
}
