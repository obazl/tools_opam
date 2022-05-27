#include <unistd.h>

#include "bootstrap.h"

/* @opam//here/opam */

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
            display_manpage("man1", "@opam_here_opam.1");
            exit(EXIT_SUCCESS);
            break;
        }
    }
    display_manpage("man1", "@opam_here_opam.1");
}
