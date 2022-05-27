#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_init_xdg.h"

/* opam_init_xdg
   init a new opam installation rooted at xdg
   and install pkgs

   Default: current switch
   Alternative: -c compiler version, -s switch (use its compiler)
   Fallback: prompt user
*/
EXPORT int opam_init_xdg(bool force, char *_compiler_version, char *_opam_switch)
{
    log_debug("opam_init_xdg");
    /* log_debug("  force: %d, switch: %s", force, _compiler_version); */

    bool replace = false;

    char *compiler_version;
    if (_compiler_version != NULL) {
        /* utstring_printf(compiler_version, "%s", _compiler_version); */
        compiler_version = _compiler_version;

        _opam_init();

        _opam_create_switch(compiler_version);

        free(compiler_version);

        // do not import here.packages if -c passed

        exit(EXIT_SUCCESS);

    }
    else if (_opam_switch != NULL) {
        UT_string *cmd;
        /* utstring_new(cmd); */
        /* utstring_printf(cmd, "opam exec --switch %s -- ocamlc --version", */
        /*                 _opam_switch); */
        /* char *switch_compiler = run_cmd(utstring_body(cmd), false); */
        char *compiler_version = get_compiler_version(NULL);

        if (compiler_version == NULL) {
            log_error("Switch %s not found.", _opam_switch);
            printf("_here: switch %s not found.\n", _opam_switch);
            exit(EXIT_FAILURE);
        }
        printf("Using compiler version %s from opam switch %s\n",
               compiler_version, _opam_switch);
        printf("xxxx9\n");
        char *compiler_variants = get_compiler_variants(NULL);

        _opam_init();

        /* _opam_create_switch(switch_compiler); */
        opam_here_create_switch(compiler_variants
                                ?compiler_variants
                                :compiler_version);

        // do not import here.packages if -s passed

        /* free(switch_compiler); */

        exit(EXIT_SUCCESS);


    } else {
        /* log_debug("-c <version> not passed\n"); */
        /* if -c not passed, then use version from here.compiler */
        compiler_version = read_here_compiler_file();
        bool use_here_compiler = true;
        if (compiler_version != NULL) {
            printf("Your here switch is configured to use compiler version: %s"
                   " (specified in %s)\n",
                   compiler_version, HERE_COMPILER_FILE);
            use_here_compiler = prompt_yn("Reconfigure using same version?"
                                          " (if no, you will be prompted for"
                                          " a different version)\n");
            printf("ANSWER: %d\n", use_here_compiler);
        }

        /* if (compiler_version) { */
        if (use_here_compiler) {
            if (verbose) {
                log_info("@opam//init: installing compiler version '%s' (specified in %s)",  HERE_COSWITCH_ROOT HERE_COMPILER, compiler_version);
            }

            _opam_init();

            _opam_create_switch(compiler_version);

            free(compiler_version);

            if (access(HERE_COSWITCH_ROOT "/here.packages", R_OK) == 0)
                opam_import(NULL);
            else
                if (dry_run)
                    printf("here.packages not found\n");

            exit(EXIT_SUCCESS);

        } else {
            // get compiler for current switch
            // 'opam var switch' returns switch name, not compiler
            char *current_switch = run_cmd("opam var switch", false);
            /* printf("current switch %s\n", current_switch); */

            compiler_version = get_compiler_version(current_switch);
            if (verbose) {
                printf("Compiler version: %s (effective switch: %s)\n",
                       compiler_version, current_switch);
                log_debug("Compiler version: %s (effective switch: %s)",
                          compiler_version, current_switch);
            }

            printf("xxxx10\n");
            char *compiler_variants = get_compiler_variants(current_switch);
            if (verbose) {
                printf("Compiler variants: %s (effective switch: %s)\n",
                       compiler_variants, current_switch);
                log_debug("Compiler variants: %s (effective switch: %s)",
                          compiler_variants, current_switch);
            }

            bool use_current = prompt_use_current_switch(current_switch,
                                                         compiler_version,
                                                         compiler_variants);
            if (!use_current) {
                compiler_version = prompt_compiler_version();
                if (compiler_version == NULL) {
                    log_info("cancelling");
                    printf("cancelling\n");
                    //FIXME: cleanup
                    exit(EXIT_SUCCESS);
                }
            }

            _opam_init();

            _opam_create_switch(compiler_version);

            // do not import here.packages

            free(current_switch);
            free(compiler_version);

            exit(EXIT_SUCCESS);
        }
    }

    if (_opam_init() != 0) {
        free(compiler_version);
        exit(EXIT_FAILURE);
    }

    _opam_create_switch(compiler_version);
    free(compiler_version);

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
        fprintf(stderr, "FAIL: spawn_cmd(opam switch --root .opam create _here)\n");
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
