#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if INTERFACE
#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>
#endif
#endif

#include "log.h"
#include "driver.h"

int errnum;

#if EXPORT_INTERFACE
#define TOKEN_NAME(x) (char*)#x
#endif

 char *token_names[256] = {
     [DESCRIPTION] = TOKEN_NAME(description),
     [DIRECTORY] = TOKEN_NAME(directory),
     [DQ] = TOKEN_NAME(dq),
     [EQ] = TOKEN_NAME(eq),
     [ERROR] = TOKEN_NAME(error),
     [LPAREN] = TOKEN_NAME(lparen),
     [PACKAGE] = TOKEN_NAME(package),
     [PLUSEQ] = TOKEN_NAME(pluseq),
     [REQUIRES] = TOKEN_NAME(requires),
     [RPAREN] = TOKEN_NAME(rparen),
     [VERSION] = TOKEN_NAME(version),
     [VNAME] = TOKEN_NAME(vname),
     [WARNING] = TOKEN_NAME(warning),
     [WORD]    = TOKEN_NAME(word),
     [WORDS]    = TOKEN_NAME(words),
 };

#if EXPORT_INTERFACE
struct logging {
    int verbosity;
    int log_level;
    int parse_verbosity;
    int parse_log_level;
    int lex_verbosity;
    int lex_log_level;
    bool quiet;
    bool log_color;
};
#endif

EXPORT struct logging logger;

/* char THE_METAFILE[PATH_MAX]; */
struct obzl_meta_package *MAIN_PKG;

LOCAL char *package_name_from_file_name(char *fname)
{
    char *bn = basename(fname);
    int x = strlen(bn) - 5;
    if ( strncmp(&bn[x], ".META", 5) == 0) {
        bn[x] = '\0';
        return bn;
    } else {
        return basename(dirname(fname));
    }
}

bool is_empty(const char *s)
{
  while (*s) {
    if (!isspace(*s))
      return false;
    s++;
  }
  return true;
}

EXPORT struct obzl_meta_package *obzl_meta_parse_file(char *fname)
{
/* #ifdef DEBUG */
/*     log_set_quiet(false); */
/* #else */
/*     log_set_quiet(true); */
/* #endif */
    /* log_info("obzl_meta_parse_file: %s", fname); */
    FILE *f;

    f = fopen(fname, "r");
    if (f == NULL) {
        /* errnum = errno; */
        /* log_error("fopen failure for %s", fname); */
        /* log_error("Value of errno: %d", errnum); */
        /* log_error("fopen error %s", strerror( errnum )); */
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    const size_t fsize = (size_t) ftell(f);
    if (fsize == 0) {
        fclose(f);
        errno = -1;
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    char *buffer = (char*) malloc(fsize + 1);
    fread(buffer, 1, fsize, f);
    buffer[fsize] = 0;
    fclose(f);

    if (is_empty(buffer)) {
        fclose(f);
        errno = -2;
        return NULL;
    }

    /* THE_METAFILE[0] = '\0'; */
    /* mystrcat(THE_METAFILE, fname); */

    struct meta_lexer * lexer = malloc(sizeof(struct meta_lexer));
    lexer_init(lexer, fname, buffer);

    void* pParser = ParseAlloc (malloc);
    /* InitParserState(ast); */
    /* ParseTrace(stdout, "trace_"); */
    int tok;
    union meta_token *mtok = malloc(sizeof(union meta_token));

    /* if (logger.lex_verbosity == 0) */
    /*     log_set_quiet(true); */
    /* else */
    /*     log_set_quiet(logger.quiet); */
    log_set_level(logger.lex_log_level);

    /* log_set_quiet(false); */
    /* log_set_level(LOG_TRACE); */
    /* log_info("starting"); */
    /* log_set_quiet(true); */

    MAIN_PKG = (struct obzl_meta_package*)calloc(sizeof(struct obzl_meta_package), 1);
    MAIN_PKG->name      = package_name_from_file_name(fname);
    MAIN_PKG->directory = MAIN_PKG->name; // dirname(fname);
    MAIN_PKG->metafile  = fname;

    while ( (tok = get_next_token(lexer, mtok)) != 0 ) {
        /* log_set_quiet(true); */
#if defined(DEBUG_LEX)
        switch(tok) {
        case DIRECTORY:
            log_trace("lex DIRECTORY: %s", mtok->s); break;
        case FLAGS:
            log_trace("lex FLAGS: %s", mtok->s); break;
        case VNAME:
            log_trace("lex VNAME: %s", mtok->s); break;
        case WORD:
            log_trace("lex WORD: %s", mtok->s); break;
        case WORDS:
            log_trace("lex WORDS: %s", mtok->s); break;
        case DQ:
            log_trace("DQ"); break;
        case EQ:
            log_trace("lex EQ"); break;
        case PLUSEQ:
            log_trace("lex PLUSEQ"); break;
        case LPAREN:
            log_trace("lex LPAREN"); break;
        case RPAREN:
            log_trace("lex RPAREN"); break;
        case VERSION:
            log_trace("lex VERSION: %s", mtok->s);
            break;
        case DESCRIPTION:
            log_trace("lex DESCRIPTION: %s", mtok->s);
            break;
        case REQUIRES:
            log_trace("lex REQUIRES"); break;
        case PACKAGE:
            log_trace("lex PACKAGE: %s", mtok->s); break;
        case WARNING:
            log_trace("WARNING"); break;
        case ERROR:
            log_trace("ERROR"); break;
        default:
            log_trace("other: %d", tok); break;
        }
#endif
        Parse(pParser, tok, mtok, MAIN_PKG); // , &sState);

        mtok = malloc(sizeof(union meta_token));
        /* if (logger.lex_verbosity == 0) */
        /*     log_set_quiet(false); */
        /* else */
        /*     log_set_quiet(logger.quiet); */
        /*     log_set_level(logger.lex_log_level); */
    }

    /*     if (logger.parse_verbosity == 0) */
    /*         log_set_quiet(false); */
    /*     else */
    /*         log_set_quiet(logger.quiet); */
    /*         log_set_level(logger.parse_log_level); */
    /* log_set_quiet(true); */

    log_trace("lex: end of input");

    Parse(pParser, 0, mtok, MAIN_PKG); // , &sState);
    ParseFree(pParser, free );

    /* if (logger.verbosity == 0) */
    /*     log_set_quiet(false); */
    /* else */
    /*     log_set_quiet(logger.quiet); */
    /* log_set_level(logger.log_level); */

    /* log_set_quiet(false); */

    /* log_trace("PARSED %s", fname); */

    free(buffer);
    return MAIN_PKG;
}

EXPORT char *obzl_meta_version()
{
    return "0.1.0";
}
