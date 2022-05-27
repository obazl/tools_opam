#!/bin/sh

rm *.o
rm metaparser.c metalexer.c
rm config.h
rm driver.h
rm emit_build_bazel.h
rm lexer.h
# do not rm log.h
rm meta*.h
rm obazl.h
rm opam.h
# do not rm ut*h
rm lemon makeheaders
rm opam_bootstrap

