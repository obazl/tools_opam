#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_here_opam_init.h"

EXPORT int opam_here_reinit(bool force, char *_compiler_version, char *_opam_switch)
{
#if defined(DEBUG_TRACE)
    log_debug("opam_here_opam_init");
    /* log_debug("  force: %d, switch: %s", force, _compiler_version); */
#endif

    /*
      case 0: .opam/here exists - direct user to :expunge
      case 1: init ab initio - no .opam, no .obazl.d
      case 2: .opam does not exist, but .obazl.d/opam/here.compiler does
    */


    UT_string *cmd;
    utstring_new(cmd);

    /* bool replace = false; */

    if (access(HERE_OPAM_ROOT, F_OK) != 0) {
#if defined(DEBUG_TRACE)
        log_debug("OPAM here-installation at .opam NOT FOUND");
#endif
        fprintf(stderr, "OPAM here-installation at .opam NOT FOUND.\n");
        fprintf(stderr, "Did you mean @opam//here:create or @opam/here/opam:init ?\n");
        exit(EXIT_FAILURE);
    } else {
#if defined(DEBUG_TRACE)
        log_debug("Found OPAM here-installation at " HERE_OPAM_ROOT);
#endif
        /* FIXME: tell user to run @opam//here:expunge first? */
        /* check .obazl.d/opam/here.compiler for version */
        char *compiler_version = read_here_compiler_file();
        /* bool use_here_compiler = true; */
        if (compiler_version != NULL) {
            /* if (verbose) { */
            /*     printf("removing " HERE_OPAM_ROOT "\n"); */
            /*     log_debug("removing " HERE_OPAM_ROOT "\n"); */
            /* } */

            /* // rm opam here installation */
            /* run_cmd("rm -rf " HERE_OPAM_ROOT, true); */
            /* /\* run_cmd("rm -rf " HERE_OBAZL_ROOT, true); *\/ */

            /* utstring_renew(cmd); */
            /* utstring_printf(cmd, "rm %s -rf %s", */
            /*                 verbose? "-v": "", */
            /*                 HERE_SWITCH_BAZEL_ROOT); // .obazl.d/opam/here */
            /* /\* HERE_OPAM_ROOT); // .obazl.d/opam *\/ */
            /* run_cmd(utstring_body(cmd), true); */
            /* utstring_free(cmd); */
        } else {
#if defined(DEBUG_TRACE)
            log_trace(RED HERE_COMPILER_FILE CRESET " not found; trying current switch");
#endif
            if (verbose)
                printf("INFO: " RED HERE_COMPILER_FILE CRESET " not found.\n");
            // get compiler for current switch
            // 'opam var switch' returns switch name, not compiler
            char *current_switch = run_cmd("opam var switch", false);

            compiler_version = get_compiler_version(NULL);
            if (verbose)
                log_debug("current switch compiler version: %s", compiler_version);

            char *compiler_variants = get_compiler_variants(NULL);
            if (verbose)
                log_debug("current switch compiler variants: %s",
                          compiler_variants);

            bool use_current = prompt_use_current_switch(current_switch,
                                                         compiler_version,
                                                         compiler_variants);
            if (!use_current) {
                compiler_version = prompt_compiler_version();
                if (compiler_version == NULL) {
                    if (verbose) {
                        log_info("cancelling @opam//here:reinit");
                        printf("INFO: cancelling @opam//here:reinit\n");
                    }
                    //FIXME: cleanup
                    return 0;
                }
            } else {
                if (compiler_variants != NULL) {
#if defined(DEBUG_TRACE)
                    log_trace("compiler variants: %s; compiler version: %s",
                              compiler_version, compiler_variants);
#endif
                    free(compiler_version);
                    compiler_version = compiler_variants;
#if defined(DEBUG_TRACE)
                    log_trace("set compiler version to: %s",
                              compiler_variants);
#endif
                }
            }
        }
        if (compiler_version) {
            if (verbose) {
                fprintf(stderr, "INFO installing compiler version '%s'\n",
                        compiler_version);
                log_info("installing compiler version '%s'",
                         compiler_version);
            }

            /* first remove old */
            if (verbose) {
                printf("removing " HERE_OPAM_ROOT "\n");
                log_debug("removing " HERE_OPAM_ROOT "\n");
            }

            // rm opam here installation
            run_cmd("rm -rf " HERE_OPAM_ROOT, true); // .opam

            // rm obazl here coswitch
            if (verbose) {
                printf("removing " HERE_SWITCH_BAZEL_ROOT "\n");
                log_debug("removing " HERE_SWITCH_BAZEL_ROOT "\n");
            }
            utstring_renew(cmd);
            utstring_printf(cmd, "rm %s -rf %s",
                            verbose? "-v": "",
                            HERE_SWITCH_BAZEL_ROOT); // .obazl.d/opam/here
            run_cmd(utstring_body(cmd), true);
            utstring_free(cmd);

            /* we got compiler_version from here.compiler, no need
               to check for variant */
            /* char *compiler_variants = get_compiler_variants(NULL); */

            opam_here_opam_init_root();

            opam_here_create_switch(compiler_version);

            free(compiler_version);

            if (access(HERE_COSWITCH_ROOT "/here.packages", R_OK) == 0) {
                if ( !dry_run ) {
                    opam_import(NULL);
                } else {
                    if (verbose)
                        printf("here.packages not found\n");
                }
            }
            return 0;
        }
    }
    return 0;
}

EXPORT int opam_here_opam_init(bool force, char *_compiler_version, char *_opam_switch)
{
#if defined(DEBUG_TRACE)
    log_debug("opam_here_opam_init");
    /* log_debug("  force: %d, switch: %s", force, _compiler_version); */
#endif

    /*
      case 0: init ab initio - no .opam, no .obazl.d
      case 1: .opam/here exists - direct user to :expunge
      case 2: .opam does not exist, but .obazl.d/opam/here.compiler does
     */


    UT_string *cmd;
    utstring_new(cmd);

    bool replace = false;

    if (access(".opam", F_OK) == 0) {
#if defined(DEBUG_TRACE)
        log_debug("found .opam");
#endif

        /* FIXME: tell user to run @opam//here:expunge first? */
        /* check .obazl.d/opam/here.compiler for version */
        char *compiler_version = read_here_compiler_file();
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
        exit(EXIT_FAILURE);
        if (use_here_compiler) {
        } else {
            // do not use here.compiler. Get info for current
            // (effective) switch and promp

            utstring_printf(cmd, "%s", "opam switch --root .opam show");
            char *here_switch = run_cmd(utstring_body(cmd), false);
            if (verbose)
                printf("here switch name: %s\n", here_switch);
            utstring_renew(cmd);
            utstring_printf(cmd,
                            "opam exec --root %s --switch %s -- ocamlc --version",
                            HERE_OPAM_ROOT, HERE_SWITCH_NAME);

            /* char *here_compiler = run_cmd(utstring_body(cmd), false); */
            compiler_version = get_compiler_version(NULL);
            printf("compiler_version: %s\n", compiler_version);

            if (debug) {
                printf("OPAM here-switch already configured at root ./.opam, switch '%s', compiler: '%s'.\n",
                       here_switch, compiler_version);
                log_debug("OPAM here-switch already configured at root ./.opam, switch '%s', compiler: '%s'.",
                          here_switch, compiler_version);
            }
            free(here_switch);
            /* free(here_compiler); */

            replace = prompt_yn("Replace?");

            if (replace) {
                if (verbose) {
                    printf("removing " HERE_OPAM_ROOT "\n");
                    log_debug("removing " HERE_OPAM_ROOT "\n");
                }
                utstring_renew(cmd);
                utstring_printf(cmd, "rm %s -rf %s",
                                verbose? "-v": "",
                                HERE_SWITCH_BAZEL_ROOT); // .obazl.d/opam/here
                /* HERE_OPAM_ROOT); // .obazl.d/opam */
                run_cmd(utstring_body(cmd), true);
            } else {
                if (verbose)
                    printf("INFO: cancelling here-switch init\n");
                utstring_free(cmd);
                return -1;
            }
        }
    }
    /* else { */
#if defined(DEBUG_TRACE)
        log_debug("NOT found: .opam");
#endif
        /* if we got this far, then there is no here-switch at ./.opam */

        /* FIXME: prevent use of system compiler for project switch */
        /* UT_string *compiler_version; */
        /* utstring_new(compiler_version); */

        /* frontend guarantees only one of _compiler_version or
           _opam_switch is defined */

        if (_compiler_version != NULL) {
            /* utstring_printf(compiler_version, "%s", _compiler_version); */
            /* compiler_version = _compiler_version; */

            opam_here_opam_init_root();

            opam_here_create_switch(_compiler_version);

            free(_compiler_version);

            // do not import here.packages if -c passed

            return 0;

        }
        else if (_opam_switch != NULL) {
            printf("using opam switch %s\n", _opam_switch);
            /* utstring_renew(cmd); */
            /* utstring_printf(cmd, "opam exec %s --switch %s -- ocamlc --version", */
            /*                 opam_cmds_verbose? "-v" : "", */
            /*                 _opam_switch); */
            /* char *switch_compiler = run_cmd(utstring_body(cmd), false); */
            if (debug)
                printf("_opam_switch: %s\n", _opam_switch);
            char *compiler_version = get_compiler_version(_opam_switch);
            if (compiler_version == NULL) {
                log_error("Switch %s not found.", _opam_switch);
                printf("here: switch %s not found.\n", _opam_switch);
                exit(EXIT_FAILURE);
            }
            printf("Using compiler version %s from opam switch %s\n",
                   compiler_version, _opam_switch);

            printf("xxxx2\n");
            char *compiler_variants = get_compiler_variants(NULL);

            opam_here_opam_init_root();

            opam_here_create_switch(compiler_variants
                                    ?compiler_variants
                                    :compiler_version);

            // do not import here.packages if -s passed

            /* free(switch_compiler); */

            utstring_free(cmd);
            return 0;
        } else {
            /* log_debug("-c <version> not passed\n"); */

            char *compiler_version = read_here_compiler_file();
            if (compiler_version) {
                if (verbose) {
                    fprintf(stderr, "INFO installing compiler version '%s' (specified in %s)\n",  compiler_version, HERE_COSWITCH_ROOT HERE_COMPILER);
                    log_info("@opam//init: installing compiler version '%s' (specified in %s)",  compiler_version, HERE_COSWITCH_ROOT HERE_COMPILER);
                }

                /* got compiler verison from here.compiler, no need to
                   check for variants */
                /* char *compiler_variants = get_compiler_variants(NULL); */

                opam_here_opam_init_root();

                opam_here_create_switch(compiler_version);
                /* opam_here_create_switch(compiler_variants */
                /*                         ?compiler_variants */
                /*                         :compiler_version); */

                free(compiler_version);

                if (access(HERE_COSWITCH_ROOT "/here.packages", R_OK) == 0)
                    opam_import(NULL);
                else
                    if (dry_run)
                        printf("here.packages not found\n");

                return 0;

            } else {
                /* if (verbose) */
                printf(HERE_COSWITCH_ROOT HERE_COMPILER " not found.\n");
                // get compiler for current switch
                // 'opam var switch' returns switch name, not compiler
                char *current_switch = run_cmd("opam var switch", false);
                printf("current switch %s\n", current_switch);

                compiler_version = get_compiler_version(current_switch);
                /* compiler_version = run_cmd("opam exec -- ocamlc --version"); */
                if (verbose)
                    log_debug("current switch compiler version: %s", compiler_version);

                printf("xxxx4\n");
                char *compiler_variants = get_compiler_variants(current_switch);
                if (verbose)
                    log_debug("current switch compiler variants: %s",
                              compiler_variants);


                bool use_current = prompt_use_current_switch(current_switch,
                                                             compiler_version,
                                                             compiler_variants);
                if (!use_current) {
                    compiler_version = prompt_compiler_version();
                    if (compiler_version == NULL) {
                        if (verbose) {
                            log_info("cancelling");
                            printf("INFO: cancelling\n");
                        }
                        //FIXME: cleanup
                        return 0;
                    }
                }

                /* exit(EXIT_FAILURE); /\* TESTING *\/ */

                opam_here_opam_init_root();

                /* opam_here_create_switch(compiler_version); */
                opam_here_create_switch(compiler_variants
                                        ?compiler_variants
                                        :compiler_version);

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

        /* if (opam_here_opam_init_root() != 0) { */
        /*     free(compiler_version); */
        /*     exit(EXIT_FAILURE); */
        /* } */

        /* opam_here_create_switch(compiler_version); */
        /* free(compiler_version); */

        /* free(compiler_version); */
        return 0;
}

/* **************************************************************** */
EXPORT int opam_here_create_switch(char *compiler_version)
{
#if defined(DEBUG_TRACE)
    log_debug("opam_here_create_switch");
#endif
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
        opam_cmds_verbose? "--verbose": NULL,
        opam_cmds_debug? "--debug": NULL,
        NULL
    };

    int argc = (sizeof(switch_argv) / sizeof(switch_argv[0])) - 1;
    result = spawn_cmd(exe, argc, switch_argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: run_cmd(opam switch --root .opam create here)\n");
    }

    /* if (!dry_run) { */
    /*     write_here_compiler_file(compiler_version); */
    /* } */
    return result;
}

/* ********************* */
EXPORT int opam_here_opam_init_root(void)
{
#if defined(DEBUG_TRACE)
    log_debug("opam_here_opam_init_root");
#endif
    char *exe;
    int result;

    if (verbose) {
        printf("initializing opam root at: .opam\n");
        log_info("initializing opam root at: .opam");
    }
    exe = "opam";
    char *init_argv[] = {
        "opam", "init",
        opam_cmds_verbose? "--verbose" : "--quiet",
        /* opam_cmds_debug? "--debug": "", */
        "--cli=2.1",
        "--root=./.opam",
        "--bare",
        "--no-setup", "--no-opamrc",
        "--bypass-checks",
        "--yes",
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

