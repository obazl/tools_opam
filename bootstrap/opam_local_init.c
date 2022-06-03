#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_local_init.h"

EXPORT void opam_local_init(char *_compiler_version)
{
#if defined(DEBUG_TRACE)
    log_debug("opam_local_init %s", _compiler_version);
    /* log_debug("  force: %d, switch: %s", force, _compiler_version); */
#endif

    /*
      case 0: init ab initio - no _opam
      case 1: .opam/local exists - direct user to :expunge
     */

    UT_string *cmd;
    utstring_new(cmd);

    bool replace = false;

    if (access(LOCAL_SWITCH_DIR, F_OK) == 0) {
#if defined(DEBUG_TRACE)
        log_debug("found " LOCAL_SWITCH_DIR);
        log_debug("exiting");
#endif
        return;
    }

#if defined(DEBUG_TRACE)
    log_debug("NOT found: " LOCAL_SWITCH_DIR "; initializing");
#endif

    /* FIXME: prevent use of system compiler for local switch? */

    if (_compiler_version == NULL) {
        /* no compiler version passed */
        // get compiler for current switch
        // 'opam var switch' returns switch name, not compiler
        char *current_switch = run_cmd("opam var switch", false);
        printf("current switch %s\n", current_switch);

        char *compiler_version = get_compiler_version(current_switch);
        if (verbose)
            log_debug("current switch compiler version: %s",
                      compiler_version);

        char *compiler_variants = get_compiler_variants(current_switch);
        if (verbose)
            log_debug("current switch compiler variants: %s",
                      compiler_variants);

        /* opam_here_create_switch(compiler_version); */
        _opam_local_create_switch(compiler_variants
                                ?compiler_variants
                                :compiler_version);

        // do not import here.packages

        free(current_switch);
        free(compiler_version);
        free(compiler_variants);

        return;
    } else {

        _opam_local_create_switch(_compiler_version);

        return;

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
    return;
}

/* **************************************************************** */
EXPORT void _opam_local_create_switch(char *compiler_version)
{
#if defined(DEBUG_TRACE)
    log_debug("opam_local_create_switch");
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
        "--yes",
        "create", ".",
        compiler_version,
        /* dry_run? "--dry-run" : NULL, */
        /* opam_cmds_verbose? "--verbose": NULL, */
        /* opam_cmds_debug? "--debug": NULL, */
        NULL
    };

    int argc = (sizeof(switch_argv) / sizeof(switch_argv[0])) - 1;
    if (verbose)
        result = spawn_cmd_with_stdout(exe, argc, switch_argv);
    else
        result = spawn_cmd(exe, argc, switch_argv);

    if (result != 0) {
        fprintf(stderr, "FAIL: run_cmd(opam switch --root .opam create here)\n");
    }

    return;
}

/* ********************* */
EXPORT int opam_local_opam_init_root(void)
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

EXPORT int opam_local_reinit(bool force, char *_compiler_version, char *_opam_switch)
{
#if defined(DEBUG_TRACE)
    log_debug("opam_local_reinit");
    /* log_debug("  force: %d, switch: %s", force, _compiler_version); */
#endif

    /*
      case 0: .opam/here exists - direct user to :expunge
      case 1: init ab initio - no .opam, no .obazl.d
      case 2: .opam does not exist, but .obazl.d/opam/here.compiler does
    */


    UT_string *cmd;
    utstring_new(cmd);

    bool replace = false;

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
        bool use_here_compiler = true;
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

            if (access(HERE_COSWITCH_ROOT "/here.packages", R_OK) == 0)
                if ( !dry_run )
                    opam_import(NULL);
                else
                    if (verbose)
                        printf("here.packages not found\n");
            return 0;
        }
    }
}

