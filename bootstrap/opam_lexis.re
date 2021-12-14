// opam lexer
// re2c $INPUT -o $OUTPUT -i

/* NB: constants for terminals generated from opam_parser.y.
   makeheaders arranges so that any needed here are included in
   opam_lexis.h. Isn't that sweet?
 */

#include <assert.h>
#include <errno.h>
#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>

#include "opam_lexis.h"

#if EXPORT_INTERFACE
#define BUFSIZE 1024
#ifndef MAX_DEPS
#define MAX_DEPS 64
#endif
#endif

const char *deps[MAX_DEPS];
int opam_curr_tag = 0;

#if EXPORT_INTERFACE
union opam_token
{
    char *s;
};

struct opam_lexer
{
    const char *filename;
    const char *tok;
    const char *cursor;
    const char *limit;
    const char *marker;
};
#endif

/* static void mtag(const char *t) */
static void mtag(int t)
{
    /* fprintf(stderr, "mtag ctor idx: %d, s: %.22s\n", opam_curr_tag, t); */
    /* deps[opam_curr_tag++] = t; */
}


/* static void print_tags() { */
/*     /\* fprintf(stderr, "printing %d tags:\n", opam_curr_tag/2); *\/ */
/*     for (int i=0; i < opam_curr_tag/2 ; i++) { */
/*         fprintf(stderr, "\tVALTOK: '%*s'\n", (int)(deps[i*2+1] - deps[i*2]), deps[i*2]); */
/*     } */
/*     /\* fprintf(stderr, "done\n"); *\/ */
/* } */

#define YYMTAGP(t) mtag(YYCURSOR)
#define YYMTAGN(t) mtag(NULL)

int get_next_opam_token(struct opam_lexer *lexer, union opam_token *otok)
{
    /* const char *YYMARKER; */

    /* stags */
    const char *s1, *s2;        /* dq strings */
    /* const char *pkg1, *pkg2; */
    /* const char *pred1, *pred2; */
    /* const char *txt1, *txt2; */

    /* mtags */
    int f1, f2;                 /* flags */
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
      re2c:define:YYCTYPE = char;
      re2c:define:YYCURSOR = lexer->cursor;
      re2c:define:YYLIMIT = lexer->limit;
      re2c:define:YYMARKER = lexer->marker;
      //re2c:define:YYMTAGP = mtag;
      //re2c:define:YYMTAGN = mtag;
      re2c:yyfill:enable  = 0;

      re2c:flags:tags = 1;

        end    = "\x00";
        eol    = "\n";
        ws     = [ \t]*;
        listws = [ \t\n,]*;
        wsnl   = [ \t\n]*;

        DQ      = "\"";
        TQ      = "\"\"\"";

        EQ      = wsnl "=" wsnl;

        PATH    = [a-zA-Z0-9+/._]+; /* TODO: regexp for path strings, for DIRECTORY variable */

        LETTER  = [a-zA-Z];
        DIGIT   = [0-9];

        ">=" { return RELOP_GE; }

        "&"  { return AMP; }

        // <string> ::= ( (") { <char> }* (") ) | ( (""") { <char> }* (""") )
        String = (DQ [^"]* DQ) | (TQ [^"]* TQ);

        //<identchar>     ::= <letter> | <digit>  | "_" | "-"
        IdentChar = LETTER | DIGIT | "_" | "-";

        // <ident> ::= { <identchar> }* <letter> { <identchar> }*
        IDENT = IdentChar* LETTER IdentChar*;

        // <varident>      ::= [ ( <ident> | "_" ) { "+" ( <ident> | "_" ) }* : ] <ident>

        "(" { return LPAREN; }
        ")" { return RPAREN; }

        "\[" { return LBRACKET; }
        "\]" { return RBRACKET; }

        "{" { return LBRACE; }
        "}" { return RBRACE; }

        // <string> ::= ( (") { <char> }* (") ) | ( (""") { <char> }* (""") )

        /* opam-version: "2.0" */
        "opam-version:" wsnl DQ @s1 [^"]*  @s2 DQ wsnl {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return OPAM_VERSION;
        }

        // <version> ::= (") { <identchar> | "+" | "." | "~" }+ (")
        wsnl "version:" wsnl @s1 .* @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return VERSION;
        }

        wsnl "synopsis:" wsnl @s1 String @s2 {
            otok->s = strndup(s1, (size_t)(s2-s1));
            return SYNOPSIS;
        }

        wsnl "description:" { return DESCRIPTION; }

        // maintainer: [ <string> ... ] (mandatory)
        "maintainer:" { return MAINTAINER; }

        // authors: [ <string> ... ]
        "authors:" { return AUTHORS; }

        // license: [ <string> ... ]
        "license:" { return LICENSE; }

        // homepage: [ <string> ... ]
        "homepage:" { return HOMEPAGE; }

        // doc: [ <string> ... ]
        "doc:" { return DOC; }

        // bug-reports: [ <string> ... ]
        "bug-reports:" { return BUG_REPORTS; }

        // dev-repo: <string>
        "dev-repo:" { return DEV_REPO; }

        // tags: [ <string> ... ]
        "tags:" { return TAGS; }

        // patches: [ <string> { <filter> } ... ]
        "patches:" { return PATCHES; }

        // substs: [ <string> ... ]
        "substs:" { return SUBSTS; }

        // build: [ [ <term> { <filter> } ... ] { <filter> } ... ]
        //    <term> ::= <string> | <varident>
        "build:" { return BUILD; }

/* <filter> ::= <filter> <logop> <filter> */
/*            | "!" <filter> */
/*            | "?" <filter> */
/*            | ( <filter> ) */
/*            | <filter> <relop> <filter> */
/*            | <varident> */
/*            | <string> */
/*            | <int> */
/*            | <bool> */

        "install:" { return INSTALL; }
        "build-doc:" { return BUILD_DOC; }
        "run-test:" { return RUN_TEST; }
        "remove:" { return REMOVE; }
        "depends:" { return DEPENDS; }
        "depopts:" { return DEPOPTS; }
        "depexts:" { return DEPEXTS; }

        @s1 String @s2 {
            /* fprintf(stderr, "S: %.*s", (int)(s2-21), s1); */
            otok->s = strndup(s1, (size_t)(s2-s1));
            return STRING;
        }

        "true"  { return TRUE; }
        "false" { return FALSE; }

        /* "," { goto loop; } */
        wsnl { goto loop; }

        "#" .* eol {
            /* return COMMENT; */
            goto loop;          /* skip comments */
        }

        "(\*" .* "\*)" {
            // comment
            goto loop;
        }

        /* wsnl @s1 "requires" @s2 wsnl { return REQUIRES; } */


        /* wsnl "(" @s1 (listws #f1 FLAG #f2 listws)* @s2 ")" wsnl { */
        /* /\* we leave it to the parser to tokenize, now that we know each flag is syntactically correct *\/ */
        /*     otok->s = strndup(s1, (size_t)(s2 - s1)); */
        /*     return FLAGS; */

        /* } */

        /* wsnl "description" wsnl EQ wsnl DQ @s1 TEXT @s2 DQ wsnl { */
        /* otok->s = strndup(s1, (size_t)(s2-s1)); */
        /*     return DESCRIPTION; */
        /* } */

        /* wsnl "error" wsnl EQ wsnl DQ @s1 TEXT @s2 DQ wsnl { */
        /* otok->s = strndup(s1, (size_t)(s2-s1)); */
        /*     return ERROR; */
        /* } */

        /* wsnl "directory" wsnl EQ wsnl DQ @s1 PATH @s2 DQ wsnl { */
        /* otok->s = strndup(s1, (size_t)(s2-s1)); */
        /*     return DIRECTORY; */
        /* } */

        /* wsnl "warning" wsnl EQ wsnl DQ @s1 TEXT @s2 DQ wsnl { */
        /* otok->s = strndup(s1, (size_t)(s2-s1)); */
        /*     return WARNING; */
        /* } */

/* **************************************************************** */
        Varident = (IDENT | "_")? ("+" (IDENT | "_"))* IDENT;

        /* @s1 Varident @s2 { */
        @s1 IDENT @s2 {
            otok->s = strndup(s1, (size_t)(s2 - s1));
            return VARIDENT;
        }

        "=" { return EQ; }

        *         {
            fprintf(stderr, "ERROR lexing: %s: %s\n", lexer->filename, lexer->cursor);
            exit(-1);
        }
        end       {
        /* printf("ending\n"); */
        return 0;
        }
        ws | eol {
        // printf("looping\n");
        goto loop;
        }

    */
}

void opam_lexer_init(struct opam_lexer *lexer, const char*filename, const char *input)
{
    lexer->filename = filename;
    lexer->cursor = input;
}
