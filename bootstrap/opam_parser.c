#include <assert.h>
#if INTERFACE
#include <stdbool.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

#if INTERFACE
#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>
#endif
#endif

#include <unistd.h>

#include "log.h"
#if EXPORT_INTERFACE
#include "utarray.h"
#include "uthash.h"
#include "utstring.h"
#endif

#include "opam_parser.h"

/* UT_string *package; */

int line;
int col;

#if EXPORT_INTERFACE
struct opam_parse_state_s {
    struct opam_lexer_s *lexer;
    struct opam_package_s *pkg;
};
#endif

EXPORT struct opam_parse_state_s *opam_parser_init(struct opam_lexer_s *lexer,
                                            struct opam_package_s *pkg)
{
    struct opam_parse_state_s *ps = calloc(sizeof(struct opam_parse_state_s), 1);
    ps->lexer = lexer;
    ps->pkg = pkg;
    ps->pkg->entries = NULL;   /* important! */
    return ps;
}

EXPORT void opam_parse_state_free(opam_parse_state_s *parser)
{
    // log_debug("parser_free %s", parser->lexer->fname);
    opam_lexer_free(parser->lexer);
    // do not free pkg, client responsible
    free(parser);
}

EXPORT struct opam_package_s *opam_parse_file(const char *fname)
{
    log_set_quiet(false);

#if defined(DEBUG_PARSERS)
    log_info("opam_parse_file: %s", fname);
#endif
    FILE *f, *ftrace;

    /* log_debug("CWD: %s", getcwd(NULL, 0)); */

    /* int r = access(fname, F_OK); */
    /* log_debug("access %s ? %d", fname, r); */

    f = fopen(fname, "r");
    if (f == NULL) {
        log_error("fopen failure for %s", fname);
        perror(fname);
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

    int token_type;
    union opam_token_u otok; // = malloc(sizeof(union opam_token_u));

    struct opam_lexer_s * opam_lexer = malloc(sizeof(struct opam_lexer_s));
    opam_lexer_init(fname, opam_lexer, buffer);

    /* client must free */
    struct opam_package_s
        *opam_pkg = calloc(sizeof(struct opam_package_s), 1);
    opam_pkg->fname = strdup(fname);

    void* pOpamParser = ParseAlloc (malloc);
    struct opam_parse_state_s
        *opam_parse_state = opam_parser_init(opam_lexer, opam_pkg);

    /* tracing - FIXME: add flag */
#if defined(YYTRACE)
    char *trace_file = "lemontrace.log";
    ftrace = fopen(trace_file, "w");
    if (ftrace == NULL) {
        log_error("fopen failure for %s", trace_file);
        perror(trace_file);
        /* log_error("Value of errno: %d", errnum); */
        /* log_error("fopen error %s", strerror( errnum )); */
        /* exit(EXIT_FAILURE); */
        return NULL;
    }

    ParseTrace(ftrace, "debug");
#endif

    log_set_quiet(false);
    log_set_level(LOG_TRACE);
    /* log_info("starting parse"); */

    while ( (token_type = get_next_opam_token(opam_lexer, &otok)) != 0 ) {
#if defined(YYDEBUG)
        log_debug("token type: %d: %s",
                  token_type, opam_token_names[token_type]);
        switch(token_type) {
        case DESCRIPTION:
        case FILTER:
        case KEYWORD:
        case LOGOP:
        case OPAM_VERSION:
        case PKGNAME:
        case RELOP:
        case STRING:
        case SYNOPSIS:
        case TERM:
        case TERM_STRING:
        case TERM_VARIDENT:
        case VARIDENT:
        case VERSION:
            log_debug("\ts: %s", (char*)otok.s); break;
        /* default: */
        /*     log_debug("other: %d", tok); break; */
        }
#endif
        Parse(pOpamParser, token_type, otok, opam_parse_state);
    }
    Parse(pOpamParser, 0, otok, opam_parse_state);
    ParseFree(pOpamParser, free );
    opam_parse_state_free(opam_parse_state);
    free(buffer);
    return opam_pkg;
}
