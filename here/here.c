#include <stdio.h>

#include "bootstrap.h"

int main(int argc, char *argv[])
{
    printf("Usage: bazel run @opam//here/<cmd>\n\n");
    printf("Manage OPAM 'here switch' - an OPAM 'root' with one switch, installed at --root ./.opam\n\n");
    printf("Commands:\n");
    printf("\tconfig:\t\tconfigure here switch (init + import)\n");
    printf("\tinit:\t\tinitialize OPAM 'here' switch at root .opam\n");
    printf("\tinstall:\tinstall OPAM package in here switch\n");
    printf("\texport:\t\texport here switch to .obazl.d/opam/here.packages\n");
    printf("\timport:\t\timport here switch from .obazl.d/opam/here.packages\n");
    printf("\nFor detailed usage help: bazel run @opam//here/<cmd> -- -h\n\n");
}
