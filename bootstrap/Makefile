# MAKEFLAGS=-s
CFLAGS+=-std=c11 -I../vendored/logc -I../vendored/uthash

ifdef $(DEBUG)
CFLAGS+=-DDEBUG -g -O0
else
CFLAGS+=-O3
endif

ifdef $(DEBUG_TRACE)
CFLAGS+=-DDEBUG_TRACE
endif

# for makeheaders
SRCS=emit_build_bazel.c opam_bootstrap.c \
meta_parser/driver.c \
meta_parser/meta_entries.c \
meta_parser/meta_flags.c \
meta_parser/meta_fs.c \
meta_parser/meta_packages.c \
meta_parser/meta_properties.c \
meta_parser/meta_settings.c \
meta_parser/meta_values.c

emit_build_bazel.o: emit_build_bazel.c emit_build_bazel.h

emit_build_bazel.h: mkhdrs

opam_bootstrap.o: opam_bootstrap.c opam_bootstrap.h

opam_bootstrap.h: mkhdrs

# hdrs := $(patsubst %.c,%.h,$(wildcard *.c))

META_OBJS=meta_parser/driver.o \
meta_parser/meta_entries.o \
meta_parser/meta_flags.o \
meta_parser/meta_packages.o \
meta_parser/meta_properties.o \
meta_parser/meta_settings.o \
meta_parser/meta_values.o \
meta_parser/meta_fs.o

LOG_OBJS=../vendored/logc/log.o

BAZEL_OBJS=emit_build_bazel.o opam_bootstrap.o

PARSER_OBJS=meta_parser/metalexer.o meta_parser/metaparser.o

OBJS=${BAZEL_OBJS} ${LOG_OBJS}

opam_bootstrap: ${OBJS} metaparser
	$(CC) $(OBJS) $(META_OBJS) $(PARSER_OBJS) -o $@ $(LDFLAGS)

# meta: ${META_OBJS}

# parser: ${PARSER_OBJS}

# $(hdrs): mkhdrs

# metalexer.h: metalexer.c mkhdrs

# ## Assumption is that re2c has already been built...
# metalexer.c:
# 	re2c metalexer.re -o metalexer.c

# metaparser.c: metaparser.y lemon
# 	../vendored/lemon/lemon -m metaparser.y -T../vendored/lemon/lempar.c

metaparser:
	$(MAKE) -C meta_parser all

mkhdrs: makeheaders
	../vendored/makeheaders/makeheaders ${SRCS}

# obazl_h: makeheaders metaparser.c metalexer.c *.c
# 	../vendored/makeheaders/makeheaders -h ${SRCS} metalexer.c metaparser.c > obazl.h


# .PHONY: lemon
# lemon:
# 	make -C ../vendored/lemon

# .PHONY: logc
# logc:
# 	make -C ../vendored/logc

# .PHONY: makeheaders
makeheaders:
	$(MAKE) -C ../vendored/makeheaders

clean:
	rm -f *.h *.o
	rm -f opam_bootstrap
