#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_init_prompts.h"

bool prompt_use_current_switch(char *current_switch,
                               char *compiler_version,
                               char *compiler_variants)
{
    bool use_current = false;
    /* char ok[2]; */
    fprintf(stdout,
            "Current OPAM switch name is "
            RED "'%s'" CRESET ", configured with:\n",
            current_switch);
    fprintf(stdout, "\tcompiler version:  " RED "%s" CRESET "\n",
            compiler_version);
    if (compiler_variants != NULL)
        fprintf(stdout, "\tcompiler variants: " RED "%s" CRESET "\n",
                compiler_variants);

    use_current = prompt_yn("Configure here-switch with this configuration?");

    /* fprintf(stdout, */
    /*         "Configure here-switch with this configuration %s? [Yn] ", */
    /*         compiler_version); */

    /* while(1) { */
    /*     memset(ok, '\0', 2); */
    /*     fflush(stdin); */
    /*     fgets(ok, 2, stdin); */
    /*     /\* printf("ok[0]: %c\n", ok[0]); *\/ */
    /*     if (ok[0] == '\n') { */
    /*         use_current = true; */
    /*         break; */
    /*     } else { */
    /*         if ( (ok[0] == 'y') || (ok[0] == 'Y') ) { */
    /*             /\* printf("Replacing y\n"); *\/ */
    /*             use_current = true; */
    /*             break; */
    /*         } else { */
    /*             if ( (ok[0] == 'n') || (ok[0] == 'N') ) { */
    /*                 use_current = false; */
    /*                 break; */
    /*             } else { */
    /*                 fprintf(stdout, */
    /*                         "Please enter y or n (or <enter> for default)\n"); */
    /*                 fprintf(stdout, */
    /*                         "Configure here-switch with compiler version %s? [Yn] ", */
    /*                         compiler_version); */
    /*             } */
    /*         } */
    /*     } */
    /* } */
    return use_current;
}

char *prompt_compiler_version(void)
{
    char version[128]; version[0] = '\0';
    printf("Which compiler version do you want to install? (<enter> to cancel) ");
    fflush(stdin);
    fgets(version, 127, stdin);

    if (version[0] == '\n') {
        return NULL;
    } else {
        // remove newline
        return strndup(version, strlen(version) - 1);
    }
}

EXPORT bool prompt_yn(char *prompt) {
    int ch;
    printf("%s ", prompt);
    while(1) {
        fflush(stdin);
        printf("[Yn] ");
        ch = getchar();
        if (ch == '\n') {
            return true;
        } else {
            if ( (ch == 'y') || (ch == 'Y') ) {
                return true;
            } else {
                if ( (ch == 'n') || (ch == 'N') ) {
                    return false;
                } else {
                    fprintf(stdout, "Please enter y or n ");
                }
            }
        }
    }
}
