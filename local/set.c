#include <unistd.h>

#include "bootstrap.h"

/* @opam//local:set */
int main(int argc, char *argv[])
{
    char *opts = "hdDvV";
    int opt;

    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
        case '?':
            fprintf(stderr, "uknown opt: %c\n", optopt);
            exit(EXIT_FAILURE);
            break;
        case ':':
            fprintf(stderr, "uknown option: %c", optopt);
            exit(EXIT_FAILURE);
            break;
        case 'h':
            display_manpage("man1", "@opam_local_set.1");
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }
    /* if (argc > 1) { */
    /*     display_manpage("man1", "@opam_local_set.1"); */
    /*     return 0; */
    /* } */

    optind = 1;
    return opam_main(argc, argv, LOCL);
}
