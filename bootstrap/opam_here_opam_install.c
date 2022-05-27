#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_here_opam_install.h"

EXPORT void opam_here_opam_install_pkg(char *_package)
{
    log_debug("opam_here_opam_install_pkg %s", _package);

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
            "opam", "install",
            /* opam_cmds_verbose? "--verbose": "", */
            /* opam_cmds_debug? "--debug": "", */
            "--yes",
            "--cli=2.1",
            "--root=./" HERE_OPAM_ROOT,
            "--switch", HERE_SWITCH_NAME,
            "--require-checksums",
            _package,
            NULL
        };

        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        /* if (verbose) */
        /*     printf("Installing OPAM package: %s\n", _package); */
        result = spawn_cmd(exe, argc, argv);
        /* result = spawn_cmd_with_stdout(exe, argc, argv); */
        if (result != 0) {
            fprintf(stderr, "FAIL: spawn_cmd(opam install --root .opam --switch _here --require-checksums %s)\n", _package);
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("install result: %s\n", result); */
        }
    }
}

void opam_here_opam_remove_pkg(char *_package) {
    if (verbose)
        printf("opam_here_opam_remove_pkg: %s\n", _package);

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
            "opam", "remove",
            opam_cmds_verbose? "--verbose": "",
            opam_cmds_debug? "--debug": "",
            "--yes",
            "--cli=2.1",
            "--root=./" HERE_OPAM_ROOT,
            "--switch", HERE_SWITCH_NAME,
            _package,
            NULL
        };

        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        if (verbose)
            printf("Removing OPAM here-switch package: %s\n", _package);
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: spawn_cmd(opam remove --root .opam --switch _here --require-checksums %s)\n", _package);
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("install result: %s\n", result); */
        }
    }
}

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
                        HERE_COSWITCH_ROOT);
    } else {
        utstring_printf(cmd,
                        "opam switch export %s/here.packages --switch %s",
                        HERE_COSWITCH_ROOT,
                        opam_switch_name); // , opam_switch_name);
    }

    errno = 0;
    char *result = run_cmd(utstring_body(cmd), true);
    if (result == NULL) {
        fprintf(stderr, "export: %s", strerror(errno));
        log_error("export: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    opam_import(NULL);
}
