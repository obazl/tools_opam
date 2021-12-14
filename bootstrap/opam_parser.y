%include {
#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

/* static int indent = 2; */
/* static int delta = 2; */
/* static char *sp = " "; */

#if INTERFACE
#ifndef YYMALLOCARGTYPE
#define YYMALLOCARGTYPE size_t
#endif
#endif
}

/* %extra_argument { struct obzl_opam_package *the_root_pkg } */

/* Terminals. lemon will generated #define constants for each symbol;
makeheader will ensure that any file gets the symbols it needs in its
makeheader-generated header. */
%token AMP.
%token AUTHORS.
%token AVAILABLE.
%token BUG_REPORTS.
%token BUILD.
%token BUILD_DOC.
%token BUILD_ENV.
%token COLON.
%token CONFLICTS.
%token CONFLICT_CLASS.
%token DEPENDS.
%token DEPEXTS.
%token DEPOPTS.
%token DESCRIPTION.
%token DEV_REPO.
%token DOC.
%token DQ.
%token EQ.
%token ERROR.
%token EXTRA_FILES.
%token EXTRA_SOURCE.
%token FALSE.
%token FEATURES.
%token FLAGS.
%token HOMEPAGE.
%token IDENT.
%token IDENTCHAR.
%token INSTALL.
%token LBRACE.
%token LBRACKET.
%token LICENSE.
%token LPAREN.
%token MAINTAINER.
%token MESSAGES.
%token OPAM_VERSION.
%token PACKAGE.
%token PATCHES.
%token PIN_DEPENDS.
%token POST_MESSAGES.
%token RBRACE.
%token RBRACKET.
%token REMOVE.
%token RELOP_GE.
%token RPAREN.
%token RUN_TEST.
%token SETENV.
%token STRING.
%token SUBSTS.
%token SYNOPSIS.
%token TAGS.
%token TRUE.
%token URL.
%token VARIDENT.
%token VERSION.

/* **************** */
%token_type { union opam_token* }

%syntax_error {
    log_trace("**************** Syntax error! ****************");
    exit(EXIT_FAILURE);
}

%parse_accept {
    log_trace("Parsing complete");
}

/* **************************************************************** */
/* package ::= IDENT . { */
/* #if YYDEBUG */
/* #endif */
/* } */
/* package ::= VERSION . { */
/* #if YYDEBUG */
/* #endif */
/* } */
/* package ::= OPAM_VERSION . { */
/* #if YYDEBUG */
/* #endif */
/* } */

/* version: "0.15.0" */
version ::= VERSION COLON STR . {
    printf("VERSION\n");
}

/* /\* opam-version: "2.0" *\/ */
/* opam_version ::= OPAM_VERSION COLON STR . { */
/*     printf("OPAM_VERSION\n"); */
/* } */

/* maintainer: "Spiros Eliopoulos <spiros@inhabitedtype.com>" */
/* authors: [ "Spiros Eliopoulos <spiros@inhabitedtype.com>" ] */
/* license: "BSD-3-clause" */

/* ****************** */
/* words(WORDS) ::= WORD(W) . { */
/* #if YYDEBUG */
/*     log_trace("\n"); */
/*     log_trace(">>words ::= WORD"); */
/*     log_trace("\twords lhs(WORDS)"); */
/*     if (W->s == NULL) */
/*         log_trace("\tWORD (W): NULL"); */
/*     else */
/*         log_trace("\tWORD (W): %s", W->s); */
/* #endif */
/*     /\* UT_array *new_values = obzl_meta_values_new_copy(W->s); *\/ */
/*     obzl_meta_values *new_values = obzl_meta_values_new(W->s); */
/*     /\* dump_values(indent, new_values); *\/ */
/*     WORDS = new_values; */
/* } */

/* opcode(A) ::= PLUSEQ(B) . { */
/* #if YYDEBUG */
/*     log_trace("\n"); */
/*     log_trace(">>opcode(A) ::= PLUSEQ(B)"); */
/* #endif */
/*     A = OP_UPDATE; */
/* } */

