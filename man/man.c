#include <unistd.h>

#include "bootstrap.h"

/*
  @opam//man -- <page>
  display manpage <page>
*/

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
            display_manpage("man1", "man");
            exit(EXIT_SUCCESS);
            break;
        default:
            return opam_main(argc, argv, LOCL);
        }
    }
    char *s, *tmp;
    for (int i = optind; i < argc; i++) {
        if (argv[i] != NULL) {
            printf("arg[%d]: %s\n", i, argv[i]);
            s = argv[i];
            while(s) {
                tmp = strchr(s, '/');
                if (tmp == NULL) {
                    tmp = strchr(s, '-');
                    if (tmp == NULL)
                        break;
                }
                *tmp = '_';
                s = tmp;
            }
            display_manpage("man1", argv[i]);
            exit(EXIT_SUCCESS);
        }
    }
    /* default, if no args passed */
    display_manpage("man1", "man");
}
