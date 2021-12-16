#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_import.h"

char *prompt_import_manifest(void)
{
    char manifest[128]; manifest[0] = '\0';
    printf("Name of file to import? ");
    fflush(stdin);
    fgets(manifest, 127, stdin);

    // rm newline
    return strndup(manifest, strlen(manifest) - 1);
}

/*
  If no -m <manifest> passed, import .obazl.d/opam/here.packages
  Fallback: prompt
 */
EXPORT void opam_import(char *manifest)
{
    /* log_debug("opam_import: %s", manifest); */
    /* printf("opam_import: %s\n", manifest); */

    UT_string *manifest_name;
    utstring_new(manifest_name);

    if (manifest) {

        utstring_printf(manifest_name, "%s", manifest);

    } else {

        if (access(OBAZL_OPAM_ROOT "/here.packages", R_OK) == 0) {
            utstring_printf(manifest_name, "%s",
                            OBAZL_OPAM_ROOT "/here.packages");
        } else {
            char *m = prompt_import_manifest();
            if (m)
                utstring_printf(manifest_name, "%s", m);
            else {
                printf("Error\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    printf("Importing %s\n", utstring_body(manifest_name));

    char *exe;
    int result;

    if (access(ROOT_DIRNAME, F_OK) != 0) {
        if (!dry_run) {
            log_error("Project-local OPAM root '.opam' not found.");
            printf("Project-local OPAM root '.opam' not found.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        exe = "opam";
        char *argv[] = {
            "opam", "switch",
            "--cli=2.1",
            "--root=./" ROOT_DIRNAME,
            "--switch", HERE_SWITCH_NAME,
            "--yes",
            "import",
            utstring_body(manifest_name),
            NULL
        };
        utstring_body(manifest_name);
        /* printf("Importing %s\n", utstring_body(manifest_name)); */

        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: run_cmd(opam import --root .opam --switch _here %s)\n", utstring_body(manifest_name));
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("import result: %s\n", result); */
        }
    }
}

