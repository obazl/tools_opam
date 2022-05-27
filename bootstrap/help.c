#include <unistd.h>

#include "utstring.h"

#include "help.h"

EXPORT void display_manpage(char *section, char *manpage) {

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
