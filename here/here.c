#include <unistd.h>

#include "bootstrap.h"

int main(int argc, char *argv[])
{
    char *opts = "hvVdD";
    int opt;
    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
        case '?':
            fprintf(stderr, "uknown opt: %c", optopt);
            exit(EXIT_FAILURE);
            break;
        case ':':
            fprintf(stderr, "uknown option: %c", optopt);
            exit(EXIT_FAILURE);
            break;
        case 'h':
            /* _print_usage(); */
            display_manpage("man1", "@opam_here.1");
            exit(EXIT_SUCCESS);
            break;
        default:
            return opam_main(argc, argv, true);
        }
    }
    /* default, if no args passed */
    display_manpage("man1", "@opam_here.1");
}
