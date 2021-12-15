#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_init_here.h"

bool prompt_use_current(char *current_switch, char *compiler_version)
{
    bool use_current = false;
    char ok[2];
    printf("Current switch is %s, configured with compiler %s\n",
           current_switch, compiler_version);
    printf("Configure here-switch with compiler version %s? [Yn] ",
           compiler_version);

    while(1) {
        memset(ok, '\0', 2);
        fflush(stdin);
        fgets(ok, 2, stdin);
        /* printf("ok[0]: %c\n", ok[0]); */
        if (ok[0] == '\n') {
            use_current = true;
            break;
        } else {
            if ( (ok[0] == 'y') || (ok[0] == 'Y') ) {
                /* printf("Replacing y\n"); */
                use_current = true;
                break;
            } else {
                if ( (ok[0] == 'n') || (ok[0] == 'N') ) {
                    use_current = false;
                    break;
                } else {
                    printf("Please enter y or n (or <enter> for default)\n");
                    printf("Configure here-switch with compiler version %s? [Yn] ",
                           compiler_version);
                }
            }
        }
    }
    return use_current;
}

char *prompt_compiler_version(void)
{
    char version[128]; version[0] = '\0';
    printf("Which compiler version do you want to install? ");
    fflush(stdin);
    fgets(version, 127, stdin);

    return strndup(version, strlen(version));
}

/* opam_init_here
   init a new opam installation rooted at ./opam, switch 'obazl',
   and install pkgs

   Default: install here.compiler and import here.packages
   Alternative: -c compiler version, -s switch (use its compiler)
   Fallback: prompt user
*/
EXPORT int opam_init_here(bool force, char *_compiler_version, char *_opam_switch)
{
    /* log_debug("opam_init_project_switch"); */
    /* log_debug("  force: %d, switch: %s", force, _compiler_version); */

    bool replace = false;

    if (access(".opam", F_OK) == 0) {

        char *here_switch = run_cmd("opam switch --root .opam show");
        char *here_compiler = run_cmd("opam exec --root .opam --switch obazl -- ocamlc --version");

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
            run_cmd("rm -rf ./.opam");
        } else {
            printf("cancelling init\n");
            return -1;
        }
    }

    /* if we got this far, then there is no here-switch at ./.opam */

    /* FIXME: prevent use of system compiler for project switch */
    /* UT_string *compiler_version; */
    /* utstring_new(compiler_version); */

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
        utstring_new(cmd);
        utstring_printf(cmd, "opam exec --switch %s -- ocamlc --version",
                        _opam_switch);
        char *switch_compiler = run_cmd(utstring_body(cmd));
        if (switch_compiler == NULL) {
            log_error("Switch %s not found.", _opam_switch);
            printf("obazl: switch %s not found.\n", _opam_switch);
            exit(EXIT_FAILURE);
        }
        printf("Using compiler version %s from opam switch %s\n",
               switch_compiler, _opam_switch);

        _opam_init();

        _opam_create_switch(switch_compiler);

        // do not import here.packages if -s passed

        free(switch_compiler);

        exit(EXIT_SUCCESS);


    } else {
        printf("-c <version> not passed\n");

        compiler_version = read_here_compiler_file();
        if (compiler_version) {
            log_info("@opam//init: installing compiler version '%s' (specified in %s)",  OBAZL_OPAM_ROOT HERE_COMPILER, compiler_version);

            _opam_init();

            _opam_create_switch(compiler_version);

            free(compiler_version);

            opam_import(NULL); // import .obazl.d/opam/here.packages

            exit(EXIT_SUCCESS);

        } else {

            printf(OBAZL_OPAM_ROOT HERE_COMPILER " not found.\n");
            // get compiler for current switch
            // 'opam var switch' returns switch name, not compiler
            char *current_switch = run_cmd("opam var switch");
            /* printf("current switch %s\n", current_switch); */

            compiler_version = run_cmd("opam exec -- ocamlc --version");
            printf("current switch ocamlc version: %s\n", compiler_version);

            bool use_current = prompt_use_current(current_switch, compiler_version);
            if (!use_current) {
                compiler_version = prompt_compiler_version();
            }
            printf("Installing compiler version %s\n", compiler_version);

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

    exe = "opam";
    char *init_argv[] = {
        "opam", "init",
        "--cli=2.1",
        "--root=./.opam",
        "--bare",
        "--no-setup", "--no-opamrc",
        "--bypass-checks",
        "--yes", "--quiet",
        /* "--dry-run", */
        NULL
    };

    int argc = (sizeof(init_argv) / sizeof(init_argv[0])) - 1;
    result = spawn_cmd(exe, argc, init_argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch obazl)\n");
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

    char *switch_argv[] = {
        "opam", "switch",
        "--cli=2.1",
        "--root=./.opam",
        /* "--description", utstring_body(desc), */
        "create", "obazl",
        compiler_version,
        NULL
    };

    int argc = (sizeof(switch_argv) / sizeof(switch_argv[0])) - 1;
    result = spawn_cmd(exe, argc, switch_argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: run_cmd(opam switch --root .opam create obazl)\n");
    }
    return result;
}
