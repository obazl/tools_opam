#include <dirent.h>
#include <unistd.h>

#include "log.h"
#include "opam_xdg_list.h"

/*
  list XDG coswitches
 */
EXPORT void opam_xdg_list(char *coswitch_name)
{
    if (debug) {
        printf("opam_xdg_list, coswitch: %s\n", coswitch_name);
        printf("XDG_DATA_HOME: %s\n", utstring_body(xdg_data_home));
        printf("XDG_COSWITCH_ROOT: %s\n", utstring_body(xdg_coswitch_root));
    }

    if (coswitch_name != 0) {
        printf("ignoring %s: list not yet implemented for particular coswitch\n", coswitch_name);
    }

    if (access(utstring_body(xdg_coswitch_root), F_OK) != 0) {
        log_error("XDG coswitch root '%s' not found.\n",
                 utstring_body(xdg_coswitch_root));
        fprintf(stderr, "ERROR: XDG coswitch root '%s' not found.\n",
                utstring_body(xdg_coswitch_root));
        exit(EXIT_FAILURE);
    } else {
        if (NULL == opam_switch_name) {
            if (verbose)
                printf("No coswitch requested (-s); listing all\n");
        } else {
            if (verbose)
                printf("Coswitch %s requested\n", coswitch_name);
        }

        DIR *d = opendir(utstring_body(xdg_coswitch_root));
        if (d == NULL) {
            fprintf(stderr, "Unable to opendir: %s\n",
                    utstring_body(xdg_coswitch_root));
            return;
        }

        struct dirent *direntry;
        while ((direntry = readdir(d)) != NULL) {
            /* if(direntry->d_type==DT_REG){ */
            /* } */
            if (direntry->d_name[0] != '.') {
                switch (direntry->d_type) {
                case DT_REG:
                    printf("\t%s/" WHT "%s" CRESET "\n",
                           utstring_body(xdg_coswitch_root),
                           direntry->d_name);
                    break;
                case DT_DIR:
                    /* printf("\t" BLU "%s\n", direntry->d_name); */
                    printf("\t%s/" BLU "%s" CRESET "\n",
                           utstring_body(xdg_coswitch_root),
                           direntry->d_name);
                    break;
                case DT_LNK:
                    printf("\t" MAG "%s\n", direntry->d_name);
                    break;
                default:
                        printf("\t" RED "%s\n", direntry->d_name);
                }
            }
        }
        closedir(d);
        // stublibs
        // opam pkgs
    }
}

