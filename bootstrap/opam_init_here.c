#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_init_here.h"

/* opam_init_here
   init a new opam installation rooted at ./opam, switch 'here',
   and install pkgs

   Default: install here.compiler and import here.packages
   Alternative: -c compiler version, -s switch (use its compiler)
   Fallback: prompt user
*/
EXPORT int opam_init_here(bool force, char *_compiler_version, char *_opam_switch)
{
    log_debug("opam_init_here");
    /* log_debug("  force: %d, switch: %s", force, _compiler_version); */

    bool replace = false;

    if (access(".opam", F_OK) == 0) {

        char *here_switch = run_cmd("opam switch --root .opam show");
        char *here_compiler = run_cmd(
             "opam exec --root " HERE_OPAM_ROOT
             " --switch " HERE_SWITCH_NAME
             " -- ocamlc --version");

        printf("OPAM here-switch already configured at root ./.opam, switch '%s', compiler: '%s'.\n", here_switch, here_compiler);

        free(here_switch);
        free(here_compiler);

        char do_replace[2];
        printf("Replace? [yN] ");
        while(1) {
            memset(do_replace, '\0', 2);
            fflush(stdin);
            fgets(do_replace, 2, stdin);
            if (do_replace[0] == '\n') {
                break; // default: no
            } else {
                if ( (do_replace[0] == 'y') || (do_replace[0] == 'Y') ) {
                    /* printf("Replacing y\n"); */
                    replace = true;
                    break;
                } else {
                    if ( (do_replace[0] == 'n') || (do_replace[0] == 'N') ) {
                        break;
                    } else {
                        printf("Please enter y or n (or <enter> for default)\n");
                        printf("Replace? [Yn] ");
                    }
                }
            }
        }
        if (replace) {
            printf("removing ./.opam\n");
            run_cmd("rm -rf " HERE_OPAM_ROOT);
        } else {
            printf("cancelling here-switch init\n");
            return -1;
        }
    }

    /* if we got this far, then there is no here-switch at ./.opam */

    /* FIXME: prevent use of system compiler for project switch */
    /* UT_string *compiler_version; */
    /* utstring_new(compiler_version); */

    /* frontend guarantees only one of _compiler_version or
       _opam_switch is defined */

    if (_compiler_version != NULL) {
        /* utstring_printf(compiler_version, "%s", _compiler_version); */
        /* compiler_version = _compiler_version; */

        _opam_init();

        _opam_create_switch(_compiler_version);

        free(_compiler_version);

        // do not import here.packages if -c passed

        return 0;

    }
    else if (_opam_switch != NULL) {
        UT_string *cmd;
        utstring_new(cmd);
        utstring_printf(cmd, "opam exec --switch %s -- ocamlc --version",
                        _opam_switch);
        char *switch_compiler = run_cmd(utstring_body(cmd));
        if (switch_compiler == NULL) {
            log_error("Switch %s not found.", _opam_switch);
            printf("here: switch %s not found.\n", _opam_switch);
            exit(EXIT_FAILURE);
        }
        printf("Using compiler version %s from opam switch %s\n",
               switch_compiler, _opam_switch);

        _opam_init();

        _opam_create_switch(switch_compiler);

        // do not import here.packages if -s passed

        free(switch_compiler);

        return 0;
    } else {
        /* log_debug("-c <version> not passed\n"); */

        char *compiler_version = read_here_compiler_file();
        if (compiler_version) {
            if (verbose) {
                fprintf(stderr, "INFO installing compiler version '%s' (specified in %s)\n",  compiler_version, HERE_OBAZL_ROOT HERE_COMPILER);
                log_info("@opam//init: installing compiler version '%s' (specified in %s)",  compiler_version, HERE_OBAZL_ROOT HERE_COMPILER);
            }

            /* exit(EXIT_FAILURE); /\* TESTING *\/ */

            _opam_init();

            _opam_create_switch(compiler_version);

            free(compiler_version);

            if (access(HERE_OBAZL_ROOT "/here.packages", R_OK) == 0)
                opam_import(NULL);
            else
                if (dry_run)
                    printf("here.packages not found\n");

            return 0;

        } else {
            /* if (verbose) */
            /*     log_debug(HERE_OBAZL_ROOT HERE_COMPILER " not found.\n"); */
            // get compiler for current switch
            // 'opam var switch' returns switch name, not compiler
            char *current_switch = run_cmd("opam var switch");
            /* printf("current switch %s\n", current_switch); */

            compiler_version = run_cmd("opam exec -- ocamlc --version");
            if (verbose)
                log_debug("current switch ocamlc version: %s", compiler_version);

            bool use_current = prompt_use_current(current_switch, compiler_version);
            if (!use_current) {
                compiler_version = prompt_compiler_version();
                if (compiler_version == NULL) {
                    log_info("cancelling");
                    printf("cancelling\n");
                    //FIXME: cleanup
                    return 0;
                }
            }

            /* exit(EXIT_FAILURE); /\* TESTING *\/ */

            _opam_init();

            _opam_create_switch(compiler_version);

            // do not import here.packages

            free(current_switch);
            free(compiler_version);

            if (use_current)
                return 1;
            else
                return 0;
        }
    }

    /* should not get here? */

    fprintf(stderr, "FIXME\n");
    exit(EXIT_FAILURE);

    /* if (_opam_init() != 0) { */
    /*     free(compiler_version); */
    /*     exit(EXIT_FAILURE); */
    /* } */

    /* _opam_create_switch(compiler_version); */
    /* free(compiler_version); */

    /* free(compiler_version); */
    return 0;
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
        fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch here)\n");
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
        log_info("creating switch with compiler version %s",
                 compiler_version);

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
        fprintf(stderr, "FAIL: run_cmd(opam switch --root .opam create here)\n");
    }

    if (!dry_run) {
        mkdir_r(HERE_OBAZL_ROOT);
        /* printf("opam_init_here opening " HERE_COMPILER_FILE "\n"); */
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
                    log_error("fopen %s: %s",
                              HERE_COMPILER_FILE,
                              strerror(errno));
                    fprintf(stderr, "fopen %s: %s",
                            HERE_COMPILER_FILE,
                            strerror(errno));
                    exit(EXIT_FAILURE);
                }
            } else {
                log_error("fopen %s: %s",
                          HERE_COMPILER_FILE,
                          strerror(errno));
                fprintf(stderr, "fopen %s: %s",
                        HERE_COMPILER_FILE,
                        strerror(errno));
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