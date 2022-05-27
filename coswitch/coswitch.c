#include <unistd.h>

#include "bootstrap.h"

int main(int argc, char *argv[])
{
    char *opts = "h";
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
            display_manpage("man1", "@opam_coswitch.1");
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }
    /* default, if no args passed */
    return opam_main(argc, argv, false);
    /* display_manpage("man1", "@opam_coswitch.1"); */
}
