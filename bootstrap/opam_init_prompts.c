#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_init_prompts.h"

bool prompt_use_current(char *current_switch, char *compiler_version)
{
    bool use_current = false;
    char ok[2];
    fprintf(stdout,
            "Current OPAM switch name is '%s', configured with compiler version %s\n",
            current_switch, compiler_version);
    fprintf(stdout,
            "Configure here-switch with compiler version %s? [Yn] ",
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
                    fprintf(stdout,
                            "Please enter y or n (or <enter> for default)\n");
                    fprintf(stdout,
                            "Configure here-switch with compiler version %s? [Yn] ",
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
