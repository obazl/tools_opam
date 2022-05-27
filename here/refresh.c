#include <unistd.h>

#include "bootstrap.h"

/* @opam//here:remove -- pkg */
int main(int argc, char *argv[])
{
#define DEBUG_PROFILE
#if defined(DEBUG_PROFILE)
    printf("here:refresh.main\n");
#endif
    bool verbose = false;
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
            display_manpage("man1", "@opam_here_refresh.1");
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }

    optind = 1;
    return opam_main(argc, argv, true);
}
