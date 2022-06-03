#include <unistd.h>

#include "bootstrap.h"

/* @opam//local:init -- pkg */
int main(int argc, char *argv[])
{
    bool verbose = false;
    char *opts = "chdDvV";
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
            display_manpage("man1", "@opam_local_init.1");
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }
    if (argc < 2) {
        display_manpage("man1", "@opam_local_init.1");
        return 0;
    }

    optind = 1;
    return opam_main(argc, argv, LOCL);
}
