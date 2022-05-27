#!/bin/sh

set -x

# required env vars: SRCDIR, RE2C

# DEBUG_FLAGS="-DDEBUG -DDEBUG_TRACE "

# CFLAGS="-O3"

SRCS="${SRCDIR}/driver.c ${SRCDIR}/meta_entries.c ${SRCDIR}/meta_flags.c ${SRCDIR}/meta_fs.c ${SRCDIR}/meta_packages.c ${SRCDIR}/meta_properties.c ${SRCDIR}/meta_settings.c ${SRCDIR}/meta_values.c ${SRCDIR}/opam_bootstrap.c ${SRCDIR}/emit_build_bazel.c"

OBJS="${SRCDIR}/driver.o ${SRCDIR}/meta_entries.o ${SRCDIR}/meta_flags.o ${SRCDIR}/meta_packages.o ${SRCDIR}/meta_properties.o ${SRCDIR}/meta_settings.o ${SRCDIR}/meta_values.o ${SRCDIR}/meta_fs.o ${SRCDIR}/log.o ${SRCDIR}/metalexer.o ${SRCDIR}/metaparser.o ${SRCDIR}/emit_build_bazel.o ${SRCDIR}/opam_bootstrap.o"

cc ${CFLAGS} ${DEBUG_FLAGS} ${SRCDIR}/lemon.c -o ${SRCDIR}/lemon

cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/log.c -o ${SRCDIR}/log.o

cc ${CFLAGS} ${DEBUG_FLAGS} ${SRCDIR}/makeheaders.c -o ${SRCDIR}/makeheaders

## ${HOME}/.local/bin/re2c ${SRCDIR}/metalexer.re -o ${SRCDIR}/metalexer.c
${RE2C} ${SRCDIR}/metalexer.re -o ${SRCDIR}/metalexer.c

${SRCDIR}/lemon -q -m ${SRCDIR}/metaparser.y -T${SRCDIR}/lempar.c -D${SRCDIR}/

${SRCDIR}/makeheaders ${SRCDIR}/metaparser.c ${SRCDIR}/metalexer.c ${SRCS}

cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/metalexer.c -o ${SRCDIR}/metalexer.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/metaparser.c -o ${SRCDIR}/metaparser.o

cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/driver.c -o ${SRCDIR}/driver.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/meta_entries.c -o ${SRCDIR}/meta_entries.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/meta_flags.c -o ${SRCDIR}/meta_flags.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/meta_fs.c -o ${SRCDIR}/meta_fs.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/meta_packages.c -o ${SRCDIR}/meta_packages.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/meta_properties.c -o ${SRCDIR}/meta_properties.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/meta_settings.c -o ${SRCDIR}/meta_settings.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/meta_values.c -o ${SRCDIR}/meta_values.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/emit_build_bazel.c -o ${SRCDIR}/emit_build_bazel.o
cc ${CFLAGS} ${DEBUG_FLAGS} -c ${SRCDIR}/opam_bootstrap.c -o ${SRCDIR}/opam_bootstrap.o

cc ${OBJS} ${LDFLAGS} -o ${SRCDIR}/opam_bootstrap
