#include <stdio.h>

#include "bootstrap.h"

/* @opam//xdg/config
   generates bazel code from opam installation
*/
int main(int argc, char *argv[])
{
    return opam_main(argc, argv, false);
}
