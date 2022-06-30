#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_here_create.h"

EXPORT int opam_here_create()
/* (bool force, char *_compiler_version, char *_opam_switch) */
{
#if defined(DEBUG_TRACE)
    log_debug("opam_here_create");
    /* log_debug("  force: %d, switch: %s", force, _compiler_version); */
#endif

    _describe_here_dir_switch();

    if (access(".opam", F_OK) == 0) {
        _describe_here_installation();
        return 0;
    }

    if (access("_opam", F_OK) == 0) {
        printf("FOUND _opam\n");
        return 0;
    }

    /* Fresh install - no existing .opam */

    /* FIXME: prevent use of system compiler for project switch */
    /* UT_string *compiler_version; */
    /* utstring_new(compiler_version); */

    /* char *compiler_version = read_here_compiler_file(); */

    /* bool use_here_compiler = (obazl_config.here_compiler==NULL) */
    /*     ?false :true; */
    /* if (compiler_version != NULL) { */
    /*     printf("Your here switch is configured to use compiler version: %s" */
    /*            " (specified in %s)\n", */
    /*            compiler_version, HERE_COMPILER_FILE); */
    /*     use_here_compiler = prompt_yn("Reconfigure using same version?" */
    /*                             " (if no, you will be prompted for" */
    /*                             " a different version)\n"); */
    /*     printf("ANSWER: %d\n", use_here_compiler); */
    /* } */

    /* if (compiler_version) { */
    //if (use_here_compiler) {
    if (obazl_config.compiler_version) {
        if (verbose) {
            fprintf(stderr, "INFO installing compiler version '%s' (specified in %s)\n",  obazl_config.compiler_version, ".obazlrc");
            log_info("@opam//init: installing compiler version '%s' (specified in %s)",  obazl_config.compiler_version, OBAZL_INI_FILE);
        }

        /* exit(EXIT_FAILURE); /\* TESTING *\/ */

        opam_here_opam_init_root();

        opam_here_create_switch(obazl_config.compiler_version);

        /* free(compiler_version); */

        if (access(HERE_COSWITCH_ROOT "/here.packages", R_OK) == 0)
            opam_import(NULL);
        else
            if (dry_run)
                printf("here.packages not found\n");

        return 0;

    } else {
        printf("No [here] compiler version specified in %s\n",
               OBAZL_INI_FILE);
        char *current_switch = run_cmd("opam var switch", false);
#if defined(DEBUG_TRACE)
        log_trace("current switch: %s", current_switch);
#endif
        if (verbose && debug)
            printf("Current switch %s\n", current_switch);

        char *compiler_version = get_compiler_version(current_switch);
        if (verbose && debug) {
            printf("Compiler version: %s (effective switch: %s)\n",
                   compiler_version, current_switch);
            log_debug("Compiler version: %s (effective switch: %s)",
                      compiler_version, current_switch);
        }

        char *compiler_variants = get_compiler_variants(current_switch);
        if (verbose && debug) {
            printf("Compiler variants: %s (effective switch: %s)\n",
                   compiler_variants, current_switch);
            log_debug("Compiler variants: %s (effective switch: %s)",
                      compiler_variants, current_switch);
        }

        /* options:
1. Clone current switch
2. use compiler from current switch but do not clone
3. Clone other switch
4. Specify compiler version for minimal switch
5. take compiler version from .obazlrc
        */
        bool use_current = prompt_use_current_switch(current_switch,
                                                     compiler_version,
                                                     compiler_variants);
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

        opam_here_opam_init_root();

        opam_here_create_switch(compiler_variants
                                ?compiler_variants
                                :obazl_config.compiler_version);

        // do not import here.packages
        /* if (access(HERE_COSWITCH_ROOT "/here.packages", R_OK) == 0) */
        /*     opam_import(NULL); */

        free(current_switch);
        /* free(compiler_version); */

        if (use_current)
            return 1;
        else
            return 0;
    }

    /* prev impl: passed -c or -s */
    /* if (_compiler_version != NULL) { */
    /*     /\* utstring_printf(compiler_version, "%s", _compiler_version); *\/ */
    /*     /\* compiler_version = _compiler_version; *\/ */

    /*     opam_here_opam_init_root(); */

    /*     opam_here_create_switch(_compiler_version); */

    /*     free(_compiler_version); */

    /*     // do not import here.packages if -c passed */

    /*     return 0; */

    /* } */
    /* else if (_opam_switch != NULL) { */
    /*     printf("using opam switch %s\n", _opam_switch); */
    /*     utstring_renew(cmd); */
    /*     utstring_printf(cmd, "opam exec %s --switch %s -- ocamlc --version", */
    /*                     verbose? "-v" : "", */
    /*                     _opam_switch); */
    /*     char *switch_compiler = run_cmd(utstring_body(cmd)); */
    /*     if (switch_compiler == NULL) { */
    /*         log_error("Switch %s not found.", _opam_switch); */
    /*         printf("here: switch %s not found.\n", _opam_switch); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /*     printf("Using compiler version %s from opam switch %s\n", */
    /*            switch_compiler, _opam_switch); */

    /*     opam_here_opam_init_root(); */

    /*     opam_here_create_switch(switch_compiler); */

    /*     // do not import here.packages if -s passed */

    /*     free(switch_compiler); */

    /*     utstring_free(cmd); */
    /*     return 0; */
    /* } else { */
    /* } */

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

int _describe_here_installation()
{
#if defined(DEBUG_TRACE)
    log_trace("_describe_here_installation");
#endif

    UT_string *cmd;
    utstring_new(cmd);
    /* bool replace = false; */

    utstring_printf(cmd, "%s", "opam switch --root .opam show");
    char *here_switch = run_cmd(utstring_body(cmd), false);
    utstring_renew(cmd);
    utstring_printf(cmd,
                    "opam exec --root %s --switch %s -- ocamlc --version",
                    HERE_OPAM_ROOT, HERE_SWITCH_NAME);

    char *compiler_version = get_compiler_version(NULL);

    char *compiler_variants = get_compiler_variants(NULL);

    if (debug) {
        log_debug("OPAM here-switch already configured at root ./.opam, switch '%s', compiler: '%s'.",
                  here_switch, compiler_version);
    }

    printf(YEL "INFO: " CRESET "OPAM here-switch already configured:\n");
    printf("  root:     ./.opam\n");
    printf("  switch:   '%s'\n", here_switch);
    printf("  compiler: '%s'.\n", compiler_version);
    if (compiler_variants)
        printf("  variants: '%s'.\n", compiler_variants);



    printf("\n");
    printf(YEL "INFO: " CRESET ".obazlrc config specs:\n");
    printf("  here root:     '%s'\n", obazl_config.here_root);
    printf("  here switch:   '%s'\n", obazl_config.here_switch);
    if (obazl_config.compiler_version) {
    printf("  compiler: '%s'\n", obazl_config.compiler_version);
    }

    printf("\n");
    printf("To recreate, first run @opam//here:expunge\n");

    free(here_switch);
    /* free(compiler_version); */

    /* char do_replace[2]; */
    /* printf("Replace? [yN] "); */
    /* while(1) { */
    /*     memset(do_replace, '\0', 2); */
    /*     fflush(stdin); */
    /*     fgets(do_replace, 2, stdin); */
    /*     if (do_replace[0] == '\n') { */
    /*         break; // default: no */
    /*     } else { */
    /*         if ( (do_replace[0] == 'y') || (do_replace[0] == 'Y') ) { */
    /*             /\* printf("Replacing y\n"); *\/ */
    /*             replace = true; */
    /*             break; */
    /*         } else { */
    /*             if ( (do_replace[0] == 'n') || (do_replace[0] == 'N') ) { */
    /*                 break; */
    /*             } else { */
    /*                 printf("Please enter y or n (or <enter> for default)\n"); */
    /*                 printf("Replace? [Yn] "); */
    /*             } */
    /*         } */
    /*     } */
    /* } */
    /* if (replace) { */
    /*     if (verbose) { */
    /*         printf("removing " HERE_OPAM_ROOT "\n"); */
    /*         log_debug("removing " HERE_OPAM_ROOT "\n"); */
    /*     } */
    /*     utstring_renew(cmd); */
    /*     utstring_printf(cmd, "rm %s -rf %s", */
    /*                     verbose? "-v": "", */
    /*                     HERE_OPAM_ROOT); */
    /*     run_cmd(utstring_body(cmd), true); */
    /*     utstring_free(cmd); */
    /*     return 0; */
    /* } else { */
    /*     printf("cancelling here-switch init\n"); */
    /*     utstring_free(cmd); */
    /*     return -1; */
    /* } */
    return 0;
}

/* opam "here dir switch" is created by 'opam switch .'
   its name is the dir path $(PWD)
   its impl is $(PWD)/_opam
   OPAM cmds will detect ./_opam and always use it as current switch

   e.g.
 opam switch create . --packages=ocaml-variants.4.14.0+options,ocaml-option-flambda
 */
int _describe_here_dir_switch()
{
#if defined(DEBUG_TRACE)
    log_trace("_describe_here_dir_switch");
#endif

    if (access("_opam", F_OK) != 0) {
        return 0;
    }

    UT_string *cmd;
    utstring_new(cmd);
    /* bool replace = false; */

    utstring_printf(cmd, "%s", "opam switch show");
    char *here_switch = run_cmd(utstring_body(cmd), false);
    utstring_renew(cmd);
    utstring_printf(cmd, "opam var ocaml:version");
    char *compiler_version = run_cmd(utstring_body(cmd), false);

    utstring_printf(cmd, "opam var ocaml:compiler");
    char *compiler_variants = run_cmd(utstring_body(cmd), false);
    utstring_free(cmd);

    /* char *compiler_version = get_compiler_version(NULL); */

    /* char *compiler_variants = get_compiler_variants(NULL); */

    printf(YEL "INFO: " CRESET "_opam:\n");
    printf("  here switch:   '%s'\n", here_switch);
    printf("  compiler:      '%s'\n", compiler_version);
    printf("  variants:      '%s'\n", compiler_variants);

    printf("\n");
    printf("To recreate, first run @opam//here:expunge\n");
    return 0;
}
