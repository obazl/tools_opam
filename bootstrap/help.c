#include <stdbool.h>
#include <unistd.h>

#include "utstring.h"

#include "help.h"

EXPORT void display_manpage(char *section, char *manpage) {

    printf("display_manpage: %s\n", manpage);

    char *exe = "man";
    int result;

    char *runfiles_root = getcwd(NULL, 0);

    printf("runfiles_root: %s\n", runfiles_root);

    UT_string *pagesrc;
    utstring_new(pagesrc);
    utstring_printf(pagesrc,
                    /* "%s/man/%s/%s", */
                    "%s/external/opam/man/%s/@opam_%s.1",
                    runfiles_root,
                    section,
                    manpage);
    printf("page src: %s\n", utstring_body(pagesrc));

    char *argv[] = {
        "man",
        utstring_body(pagesrc),
        NULL
    };

    int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
    if (verbose)
        printf("displaying manpage %s\n", utstring_body(pagesrc));
    result = spawn_cmd_with_stdout(exe, argc, argv);
    if (result != 0) {
        fprintf(stderr, "FAIL: spawn_cmd_with_stdout for man\n");
        exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("install result: %s\n", result); */
    }
    return;
}

EXPORT void execlp_manpage(char *section, char *manpage) {

    printf("display_manpage: %s\n", manpage);

    char *runfiles_root = getcwd(NULL, 0);

    /* printf("runfiles_root: %s\n", runfiles_root); */

    UT_string *src;
    utstring_new(src);
    utstring_printf(src,
                    /* "%s/man/%s/%s", */
                    "%s/external/opam/man/%s/%s",
                    runfiles_root,
                    section,
                    manpage);
    /* int rc = access(utstring_body(src), F_OK); */
    /* if (rc != 0) { */
    /*     fprintf(stderr, "not found: %s\n", utstring_body(src)); */
    /*     return; */
    /* } */
    /* printf("found manpage: %s\n", utstring_body(src)); */

    /* system(utstring_body(src)); */
    execlp("man", "--", utstring_body(src), NULL);
    return;
}
