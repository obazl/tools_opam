#include <stdio.h>

#include "bootstrap.h"

/* @opam//ingest
   generates bazel code from opam installation
*/
int main(int argc, char *argv[])
{
    /* NB: argv[0] is here/config/config */
    return opam_main(argc, argv);
}
