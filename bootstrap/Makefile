# for debugging

export DEBUG_FLAGS=-DDEBUG
export DEBUG_FLAGS+=-DDEBUG_DUMP
export DEBUG_FLAGS+=-DDEBUG_ENTRIES
export DEBUG_FLAGS+=-DDEBUG_FLAGS
export DEBUG_FLAGS+=-DDEBUG_PACKAGES
export DEBUG_FLAGS+=-DDEBUG_PROPERTIES
export DEBUG_FLAGS+=-DDEBUG_SETTINGS
export DEBUG_FLAGS+=-DDEBUG_VALUES
export DEBUG_FLAGS+=-DDEBUG_TRACE
export DEBUG_FLAGS+=-DDEBUG_TEST
export DEBUG_FLAGS+=-DDEBUG_YYDEBUG

export CFLAGS=-g

all:
	RE2C=~/.local/bin/re2c SRCDIR=. ./build.sh

clean:
	@-rm -rf *.dSYM
	@-rm -f *.o
	@-rm -f metaparser.c metaparser.h
	@-rm -f lemon makeheaders
	@-rm -f config.h
	@-rm -f driver.h
	@-rm -f frontend.h
	@-rm -f emit_build_bazel.h
	@-rm -f lexer.h
	@-rm -f meta*.h
	@-rm -f obazl.h
	@-rm -f opam.h
	@-rm -rf tmp/
