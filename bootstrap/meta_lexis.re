// findlib META lexer
// re2c $INPUT -o $OUTPUT -i
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

#include "meta_lexis.h"

#if EXPORT_INTERFACE
#define BUFSIZE 1024
#ifndef MAX_DEPS
#define MAX_DEPS 64
#endif
#endif

const char *deps[MAX_DEPS];
int curr_tag = 0;

#if EXPORT_INTERFACE
union meta_token
{
    char *s;
};

struct meta_lexer_s
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
    /* fprintf(stderr, "mtag ctor idx: %d, s: %.22s\n", curr_tag, t); */
    /* deps[curr_tag++] = t; */
}


/* static void print_tags() { */
/*     /\* fprintf(stderr, "printing %d tags:\n", curr_tag/2); *\/ */
/*     for (int i=0; i < curr_tag/2 ; i++) { */
/*         fprintf(stderr, "\tVALTOK: '%*s'\n", (int)(deps[i*2+1] - deps[i*2]), deps[i*2]); */
/*     } */
/*     /\* fprintf(stderr, "done\n"); *\/ */
/* } */

#define YYMTAGP(t) mtag(YYCURSOR)
#define YYMTAGN(t) mtag(NULL)

int get_next_meta_token(struct meta_lexer_s *lexer, union meta_token *mtok)
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
    curr_tag = 0;
    lexer->tok = lexer->cursor;
    /*!mtags:re2c format = "@@ = -1;"; */
    /*!re2c
      re2c:define:YYCTYPE = char;
      re2c:define:YYCURSOR = lexer->cursor;
      re2c:define:YYLIMIT = lexer->limit;
      re2c:define:YYMARKER = lexer->marker;
      re2c:define:YYMTAGP = mtag;
      re2c:define:YYMTAGN = mtag;
      re2c:yyfill:enable  = 0;

      re2c:flags:tags = 1;

        end    = "\x00";
        eol    = "\n";
        ws     = [ \t]*;
        listws = [ \t\n,]*;
        wsnl   = [ \t\n]*;

        DQ      = "\"";
        EQ      = wsnl "=" wsnl;

        PATH    = [a-zA-Z0-9+/._]+; /* TODO: regexp for path strings, for DIRECTORY variable */

        TEXT   = [^"]*;

        NAME    = [A-Za-z0-9-_.];
        PKGNAME = (NAME \ [.])+;
        VNAME    = NAME+;
        FLAG    = "-"? VNAME;

        WORD    = [^"(),+=\x00 ]+;
        WORDS   = [^"\x00]+;


        "," { goto loop; }
        wsnl { goto loop; }

        "#" .* eol {
            /* return COMMENT; */
            goto loop;          /* skip comments */
        }

        wsnl @s1 "requires" @s2 wsnl { return REQUIRES; }

        wsnl @s1 "ppx_runtime_deps" @s2 wsnl { return PPX_RUNTIME_DEPS; }

        wsnl "(" @s1 (listws #f1 FLAG #f2 listws)* @s2 ")" wsnl {
        /* we leave it to the parser to tokenize, now that we know each flag is syntactically correct */
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return FLAGS;

        }

        /* wsnl "description" wsnl EQ wsnl DQ @s1 TEXT @s2 DQ wsnl { */
        /* mtok->s = strndup(s1, (size_t)(s2-s1)); */
        /*     return DESCRIPTION; */
        /* } */

        /* wsnl "error" wsnl EQ wsnl DQ @s1 TEXT @s2 DQ wsnl { */
        /* mtok->s = strndup(s1, (size_t)(s2-s1)); */
        /*     return ERROR; */
        /* } */

/* "The value of the "directory" variable is determined with an empty set of actual predicates. The value must be either: an absolute path name of the alternate directory, or a path name relative to the stdlib directory of OCaml (written "+path"), or a normal relative path name (without special syntax)." */
        wsnl "directory" wsnl EQ wsnl DQ @s1 PATH @s2 DQ wsnl {
        mtok->s = strndup(s1, (size_t)(s2-s1));
            return DIRECTORY;
        }

        /* wsnl "version" wsnl EQ wsnl DQ @s1 TEXT @s2 DQ wsnl { */
        /* mtok->s = strndup(s1, (size_t)(s2-s1)); */
        /*     return VERSION; */
        /* } */

        /* wsnl "warning" wsnl EQ wsnl DQ @s1 TEXT @s2 DQ wsnl { */
        /* mtok->s = strndup(s1, (size_t)(s2-s1)); */
        /*     return WARNING; */
        /* } */

/* **************************************************************** */
        wsnl @s1 VNAME @s2 wsnl {
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return VNAME;
        }

        DQ @s1 WORD @s2 DQ {
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return WORD;
        }

        DQ @s1 WORDS @s2 DQ {
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return WORDS;
        }

        DQ @s1 @s2 DQ {
            mtok->s = NULL;
            return WORD;
        }


        wsnl "package" wsnl DQ @s1 PKGNAME @s2 DQ wsnl {
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return PACKAGE;
        }

        "(" { return LPAREN; }
        ")" { return RPAREN; }

        /* LINKOPTS = "linkopts"; */
        /* LINKOPTS { fprintf(stderr, "LINKOPTS\n"); goto loop; } */
        WARNING   = "warning";
        ERROR     = "error";
        /* EXISTS_IF = "exists_if"; */
        /* PPX       = "ppx"; */
        /* PPXOPT    = "ppxopt"; */

        // predicates
        /* BYTE     = "byte"; */
        /* NATIVE   = "native"; */
        /* TOPLOOP  = "toploop"; */
        /* CREATE_TOPLOOP = "toploop"; */
        /* MT       = "mt"; */
        /* MT_POSIX = "mt_posix"; */
        /* MT_VM    = "mt_vm"; */
        /* GPROF   = "gprof"; */
        /* AUTOLINK = "autolink"; */
        /* PREPROCESSOR = "preprocessor"; */
        /* SYNTAX   = "syntax"; */

        // "package" predicates start with "pkg_"

        PPX_RUNTIME_DEPS = "ppx_runtime_deps";

        // dune predicates
        CUSTOM_PPX   = "custom_ppx";
        PPX_DRIVER   = "ppx_driver";

        "+=" { return PLUSEQ; }
        EQ { return EQ; }

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

void meta_lexer_init(struct meta_lexer_s *lexer, const char*filename, const char *input)
{
    lexer->filename = filename;
    lexer->cursor = input;
}
