#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"

/* #if EXPORT_INTERFACE */
#include "utarray.h"
#include "uthash.h"
#include "utstring.h"
/* #endif */

#include "opam_lexer.h"

#if EXPORT_INTERFACE
#define TOKEN_NAME(x) (char*)#x
#endif

char *opam_token_names[256] = {
    [AMP]      = TOKEN_NAME(amp),
    [AUTHORS]      = TOKEN_NAME(authors),
    [AVAILABLE]      = TOKEN_NAME(available),
    [BUG_REPORTS]  = TOKEN_NAME(bug_reports),
    [BUILD]      = TOKEN_NAME(build),
    [BUILD_DOC]      = TOKEN_NAME(build_doc),
    [BUILD_ENV]      = TOKEN_NAME(build_env),
    [COLON]        = TOKEN_NAME(colon),
    [CONFLICTS]      = TOKEN_NAME(conflicts),
    [CONFLICT_CLASS]      = TOKEN_NAME(conflict_class),
    [DEPENDS]      = TOKEN_NAME(depends),
    [DEPEXTS]      = TOKEN_NAME(depexts),
    [DEPOPTS]      = TOKEN_NAME(depopts),
    [DESCRIPTION]      = TOKEN_NAME(description),
    [DEV_REPO]      = TOKEN_NAME(dev_repo),
    [DOC]          = TOKEN_NAME(doc),
    [DQ]           = TOKEN_NAME(dq),
    [EQ]           = TOKEN_NAME(eq),
    [ERROR]        = TOKEN_NAME(error),
    [EXTRA_FILES]      = TOKEN_NAME(extra_files),
    [EXTRA_SOURCE]      = TOKEN_NAME(extra_source),
    [FALSE]        = TOKEN_NAME(false),
    [FEATURES]      = TOKEN_NAME(features),
    [FLAGS]      = TOKEN_NAME(flags),
    [HOMEPAGE]     = TOKEN_NAME(homepage),
    [INSTALL]      = TOKEN_NAME(install),
    [LBRACE]      = TOKEN_NAME(lbrace),
    [LBRACKET]      = TOKEN_NAME(lbracket),
    [LICENSE]      = TOKEN_NAME(license),
    [LPAREN]       = TOKEN_NAME(lparen),
    [MAINTAINER]   = TOKEN_NAME(maintainer),
    [MESSAGES]      = TOKEN_NAME(messages),
    [OPAM_VERSION] = TOKEN_NAME(opam_version),
    [PACKAGE]      = TOKEN_NAME(package),
    [PATCHES]      = TOKEN_NAME(patches),
    [PIN_DEPENDS]      = TOKEN_NAME(pin_depends),
    [POST_MESSAGES]      = TOKEN_NAME(post_messages),
    [RBRACE]      = TOKEN_NAME(rbrace),
    [RBRACKET]      = TOKEN_NAME(rbracket),
    [RELOP_GE]      = TOKEN_NAME(relop_ge),
    [REMOVE]      = TOKEN_NAME(remove),
    [RPAREN]       = TOKEN_NAME(rparen),
    [RUN_TEST]      = TOKEN_NAME(run_test),
    [SETENV]      = TOKEN_NAME(setenv),
    [STRING]       = TOKEN_NAME(string),
    [SUBSTS]      = TOKEN_NAME(substs),
    [SYNOPSIS]      = TOKEN_NAME(synopsis),
    [TAGS]      = TOKEN_NAME(tags),
    [TRUE]         = TOKEN_NAME(true),
    [URL]      = TOKEN_NAME(url),
    [VARIDENT]     = TOKEN_NAME(varident),
    [VERSION]      = TOKEN_NAME(version),
    NULL
};

bool is_empty(const char *s)
{
  while (*s) {
    if (!isspace(*s))
      return false;
    s++;
  }
  return true;
}

/* see sealark_parsers.c */
/* EXPORT UT_array *opam_lex_file(char *fname) */
EXPORT void opam_lex_file(char *fname)
{
    log_set_quiet(false);

    /* UT_array *token_list; */
    /* utarray_new(token_list, &node_icd); */
/* nodelist: UT_array of node_s */
// sealark_nodes.c:
/* EXPORT UT_icd node_icd = {sizeof(struct node_s), NULL, sealark_alias_node, sealark_node_free}; */


    /* log_info("_lex_file: %s", fname); */
    FILE *f;

    f = fopen(fname, "r");
    if (f == NULL) {
        perror(fname);
        log_error("fopen failure for %s", fname);
        /* log_error("Value of errno: %d", errnum); */
        /* log_error("fopen error %s", strerror( errnum )); */
        exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_END);
    const size_t fsize = (size_t) ftell(f);
    if (fsize == 0) {
        fclose(f);
        errno = -1;
        exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_SET);
    char *buffer = (char*) malloc(fsize + 1);
    fread(buffer, 1, fsize, f);
    buffer[fsize] = 0;
    fclose(f);

    if (is_empty(buffer)) {
        fclose(f);
        errno = -2;
        exit(EXIT_FAILURE);
    }

    struct opam_lexer * lexer = malloc(sizeof(struct opam_lexer));
    opam_lexer_init(lexer, fname, buffer);
    /* struct bf_lexer_s * lexer = malloc(sizeof(struct bf_lexer_s)); */
    /* lexer_init(fname, lexer, buffer); */

    int tok;
    union opam_token *otok = malloc(sizeof(union opam_token));


    log_set_quiet(false);
    log_set_level(LOG_TRACE);
    /* log_info("starting lex"); */

    while ( (tok = get_next_opam_token(lexer, otok)) != 0 ) {
#if !defined(DEBUG_LEX)
        log_debug("token type: %d: %s", tok, opam_token_names[tok]);
        switch(tok) {
        case DESCRIPTION:
        case OPAM_VERSION:
        case STRING:
        case SYNOPSIS:
        case VARIDENT:
        case VERSION:
            log_debug("\ts: %s", otok->s); break;
        /* default: */
        /*     log_debug("other: %d", tok); break; */
        }
#endif
        /* utarray_push_back(token_list, otok); */

        otok = malloc(sizeof(union opam_token));
    }
    log_trace("opam_lexer: end of input");
    free(buffer);

    /* return token_list; */
}
