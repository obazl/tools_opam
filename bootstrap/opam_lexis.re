// opam lexer
// re2c $INPUT -o $OUTPUT -i

/* NB: constants for terminals generated from opam_parser.y.
   makeheaders arranges so that any needed here are included in
   opam_lexis.h. Isn't that sweet?
 */

#include <assert.h>
#include <errno.h>
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>

#include "log.h"
#include "opam_lexis.h"

#if EXPORT_INTERFACE
#define BUFSIZE 1024
#ifndef MAX_DEPS
#define MAX_DEPS 64
#endif
#endif

const char *deps[MAX_DEPS];
int opam_curr_tag = 0;

/* enable start conditions */
/*!types:re2c */

#if EXPORT_INTERFACE
#include <stdbool.h>
union opam_token_u {
    char c;
    int i;
    char *s;
    bool boolval;
};

struct opam_lexer_s
{
    const char *filename;
    const char *tok;
    const char *cursor;
    const char *limit;
    const char *marker;
    int mode;                   /* i.e. start condition */
};

#endif


void opam_lexer_init(const char*filename,
                     struct opam_lexer_s *lexer,
                     const char *input)
{
    lexer->filename = filename;
    lexer->cursor = input;
}

void opam_lexer_free(opam_lexer_s *lexer)
{
    /* log_debug("lexer_free: %s", lexer->fname); */
    /* utarray_free(lexer->indent_kws); */
}

/* static void mtag(const char *t) */
// static void mtag(int t)
// {
//     /* fprintf(stderr, "mtag ctor idx: %d, s: %.22s\n", opam_curr_tag, t); */
//     /* deps[opam_curr_tag++] = t; */
// }


/* static void print_tags() { */
/*     /\* fprintf(stderr, "printing %d tags:\n", opam_curr_tag/2); *\/ */
/*     for (int i=0; i < opam_curr_tag/2 ; i++) { */
/*         fprintf(stderr, "\tVALTOK: '%*s'\n", (int)(deps[i*2+1] - deps[i*2]), deps[i*2]); */
/*     } */
/*     /\* fprintf(stderr, "done\n"); *\/ */
/* } */

#define YYMTAGP(t) mtag(YYCURSOR)
#define YYMTAGN(t) mtag(NULL)

int get_next_opam_token(struct opam_lexer_s *lexer, union opam_token_u *otok)
{
#if defined(LEXDEBUG_FVF)
    log_debug("yycinit: %d, yycdepends: %d, yycfpf %d, yycfvf: %d",
              yycinit, yycdepends, yycfpf, yycfvf);
    // only set lexer->mode on initial call
    static bool start = true;
    if (start) {
        lexer->mode = yycfpf;
        log_debug("start mode: %d", lexer->mode);
        start = false;
    }
#elif defined (LEXDEBUG_FPF)
    log_debug("yycinit: %d, yycdepends: %d, yycfpf %d, yycfvf: %d",
              yycinit, yycdepends, yycfpf, yycfvf);
    static bool start = true;
    if (start) {
        lexer->mode = yycfpf;
        log_debug("start mode: %d", lexer->mode);
        start = false;
    }
#elif defined (LEXDEBUG_FILTERS)
    log_debug("yycinit: %d, yycdepends: %d, yycfpf %d, yycfvf: %d",
              yycinit, yycdepends, yycfpf, yycfvf);
    static bool start = true;
    if (start) {
        lexer->mode = yycfvf;
        log_debug("start mode: %d", lexer->mode);
        start = false;
    }
#else
    /* static bool start = true; */
    /* if (start) { */
    /*     lexer->mode = yycinit; */
    /*     log_debug("start mode: %d", lexer->mode); */
    /*     start = false; */
    /* } */
#endif

    /* stags */
    const char *s1, *s2;        /* dq strings */
    /* const char *pkg1, *pkg2; */
    /* const char *pred1, *pred2; */
    /* const char *txt1, *txt2; */

    /* mtags */
    /* int f1, f2; */                 /* flags */
    /* int dep1, dep2; */
    /* const char *vtok1, *vtok2; */

    /* int h1, h2; */
    /*!stags:re2c format = "const char *@@;"; */
    /*!mtags:re2c format = "int @@;"; */
loop:
    opam_curr_tag = 0;
    lexer->tok = lexer->cursor;
    /*!mtags:re2c format = "@@ = -1;"; */
    /*!re2c
      re2c:api:style = free-form;

      re2c:define:YYCTYPE = char;
      re2c:define:YYCURSOR = lexer->cursor;
      re2c:define:YYLIMIT = lexer->limit;
      re2c:define:YYMARKER = lexer->marker;
      re2c:define:YYGETCONDITION = "lexer->mode";
      re2c:define:YYSETCONDITION = "lexer->mode = @@;";

      //re2c:define:YYMTAGP = mtag;
      //re2c:define:YYMTAGN = mtag;
      re2c:yyfill:enable  = 0;

      re2c:flags:tags = 1;

        end    = "\x00";
        eol    = "\n";
        ws     = [ \t]*;
        listws = [ \t\n,]*;
        wsnl   = [ \t\n]*;

        Dq      = "\"";
        TQ      = "\"\"\"";

        EQ      = wsnl "=" wsnl;

        PATH    = [a-zA-Z0-9+/._]+; /* TODO: regexp for path strings, for DIRECTORY variable */

    // reserve UPCASE for token constants
        Letter  = [a-zA-Z];
        Digit   = [0-9];

        Int =  "-"? Digit+;

        // <string> ::= ( (") { <char> }* (") ) | ( (""") { <char> }* (""") )
        String = (TQ [^"]* TQ) | (Dq [^"]* Dq);

        //<identchar>     ::= <letter> | <digit>  | "_" | "-"
        Identchar = Letter | Digit | "_" | "-";

        // <ident> ::= { <identchar> }* <letter> { <identchar> }*
        Ident = Identchar* Letter Identchar*;

        // <varident>      ::= [ ( <ident> | "_" ) { "+" ( <ident> | "_" ) }* : ] <ident>
        Varident = (Ident | "_") ("+" (Ident | "_"))* Ident;

        // for debugging we may want to start with any start condition.
        // this <> seems to be necessary to initialize all yyc* vars
        <> {
            /* printf("yycinit: %d, yycdepends: %d, yycfvf: %d\n", */
            /*        yycinit, yycdepends, yycfvf); */
            /* lexer->mode = yycdepends; */
        }

        /* <init> "&"  { return AMP; } */
        <init> "(" { return LPAREN; }
        <init> ")" { return RPAREN; }
        <init> "\[" { return LBRACKET; }
        <init> "\]" { return RBRACKET; }
        <init> "{" { return LBRACE; }
        <init> "}" { return RBRACE; }

        <init> ":" { return COLON; }
        <init> "," { return COMMA; }

        <*> @s1 ("=" | "!=" | "<" | "<=" | ">" | ">=") @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            log_debug("mode %d: RELOP %s", lexer->mode, otok->s);
            return RELOP;
        }
        <*> "!" { return BANG; }
        <*> "?" { return QMARK; }
        <*> @s1 ("&" | "|") @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return LOGOP;
        }

        // <string> ::= ( (") { <char> }* (") ) | ( (""") { <char> }* (""") )

        /* opam-version: "2.0" */
        <init> @s1 ("opam-version"
             | "version"
             | "maintainer"
             | "authors"
             | "name"
             | "license"
             | "homepage"
             | "doc"
             | "bug-reports"
             | "dev-repo"
             | "tags"
             | "patches"
             | "substs"
             /* | "install" */
             | "build-doc"
             /* | "run-test" */
             | "remove"
             /* | "depends" */
             | "depopts"
             | "depexts"
             | "synopsis"
             | "description"
             /* | "build" */
             ) @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return KEYWORD;
        }

        <init> "build" => build {
            log_debug("build, mode: %d", lexer->mode);
            return BUILD;
        }
        <init> "install" => build {
            log_debug("install, mode: %d", lexer->mode);
            return INSTALL;
        }
        <init> "run-test" => build {
            log_debug("run-test, mode: %d", lexer->mode);
            return RUN_TEST;
        }

        <build> ":" { return COLON; }
        <build> "[" => buildlist {
            log_debug("<build> lbracket, mode %d", lexer->mode);
            return LBRACKET; }
        <build> "]" => init { return RBRACKET; }
        <buildlist> "[" => term {
            log_debug("<buildlist> lbracket, mode %d", lexer->mode);
            return LBRACKET; }
        <buildlist> "]" => init {
            // always marks end of build fld so back to <init>
            return RBRACKET;
        }
        <buildlist> "{" => termlist_filter {
            log_debug("<buildlist> lbrace mode %d", lexer->mode);
            return LBRACE;
        }

        // <term> ::= <string> | <varident>
        <term> "\"" @s1 ([^"] | "\\\"" )* @s2 "\"" {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return TERM_STRING;
        }
        <term> @s1 Varident @s2 {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return TERM_VARIDENT;
        }
        <term> "{" => term_filter {
                log_debug("<term> lbrace mode %d", lexer->mode);
                return LBRACE; }
        <term_filter> "}" => term {
                log_debug("<term_filter> rbrace mode %d", lexer->mode);
                return RBRACE; }

        <termlist_filter> "}" => buildlist {
                log_debug("<termlist_filter> rbrace mode %d", lexer->mode);
                return RBRACE; }

        <term> "[" { return LBRACKET; }
        <term> "]" => buildlist { return RBRACKET; }

        <init> "conflicts" => depends {
            return CONFLICTS;
        }
        <init> "depends" => depends {
            return DEPENDS;
        }

        <depends> ":" { return COLON; }
        <depends> "[" => fpf { return LBRACKET; }

        <fpf> Dq @s1 Ident @s2 Dq {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return PKGNAME;
        }
        <fpf> "(" { return LPAREN; }
        <fpf> ")" => init { return RPAREN; }
        <fpf> "]" => init { return RBRACKET; }
        <fpf> "{" => fvf {
            log_debug("mode %d: LBRACE", lexer->mode);
            return LBRACE;
        }

        <fvf> "}" => fpf { return RBRACE; }

        <fvf> "(" { return LPAREN; }
        <fvf> ")" { return RPAREN; }

        <fvf> @s1 ("&" | "|") @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return LOGOP;
        }

        <fvf> Dq @s1 ( Identchar | "+" | "." | "~" )+ @s2 Dq {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return VERSION;
        }

                <fvf,termlist_filter,term_filter> @s1 Varident @s2 {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return VARIDENT;
        }
        <fvf,termlist_filter,term_filter> @s1 String @s2 {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return STRING;
        }
        <fvf,termlist_filter,term_filter> @s1 Int @s2 {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return FILTER;
        }
        <fvf> @s1 ("true" | "false") @s2 {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return BOOL;
        }

        // STRINGS
        // <string> ::= ( (") { <char> }* (") ) | ( (""") { <char> }* (""") )
        // triple-quoted - this doesn't quite get it right
        <init> "\"\"\"" => triplequote { return TQ; }
        <triplequote> "\"\"\"" => init { return TQ; }
        <triplequote> @s1 "\"" ([^"] | "\\\"" )* "\"" @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return STRING;
        }
        <triplequote> @s1 [^"]* @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return STRING3;
        }

        // single-quoted
        <init> "\"" @s1 ([^"] | "\\\"" )+ @s2 "\"" {
                otok->s = strndup(s1, (size_t)(s2-s1));
                return STRING;
            }
        /* "," { goto loop; } */
        <*> wsnl { goto loop; }

        <*> "#" .* eol {
            /* return COMMENT; */
            goto loop;          /* skip comments */
        }

        <init> "(\*" .* "\*)" {
            // comment
            goto loop;
        }

/* **************************************************************** */
        /* @s1 Varident @s2 { */
        <init> @s1 Ident @s2 {
            otok->s = (char*) strndup(s1, (size_t)(s2 - s1));
            return VARIDENT;
        }

        <init> @s1 "=" @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return RELOP;
        }

        /* <init> "\"" { return DQ; } */
        /* <init> "'" { return SQ; } */

        <*>*         {
            fprintf(stderr, "ERROR lexing: %s: %s\n",
                    lexer->filename, lexer->tok);
            exit(-1);
        }

        <*> end       {
            log_debug("lexing completed");
            return 0;
        }

        <*> ws | eol {
            // printf("looping\n");
            goto loop;
        }

    */
}
