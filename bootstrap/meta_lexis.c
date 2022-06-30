/* Generated by re2c 2.0.3 on Thu Dec 30 22:07:21 2021 */
#line 1 "bootstrap/meta_lexis.re"
// findlib META lexer
// re2c $INPUT -o $OUTPUT -i
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
    /* int f1, f2; */                /* flags */
    /* int dep1, dep2; */
    /* const char *vtok1, *vtok2; */

    /* int h1, h2; */
    const char *yyt1;const char *yyt2;const char *yyt3;
    int yyt4;int yyt5;
loop:
    curr_tag = 0;
    lexer->tok = lexer->cursor;
    yyt4 = -1;yyt5 = -1;
    
#line 90 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
{
	char yych;
	unsigned int yyaccept = 0;
	yych = *lexer->cursor;
	switch (yych) {
	case 0x00:	goto yy2;
	case '\t':
	case '\n':
	case ' ':	goto yy6;
	case '"':	goto yy9;
	case '#':	goto yy10;
	case '(':	goto yy12;
	case ')':	goto yy14;
	case '+':	goto yy16;
	case ',':	goto yy17;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'q':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		yyt2 = lexer->cursor;
		goto yy19;
	case '=':	goto yy22;
	case 'd':
		yyt2 = lexer->cursor;
		goto yy25;
	case 'p':
		yyt2 = lexer->cursor;
		goto yy26;
	case 'r':
		yyt2 = lexer->cursor;
		goto yy27;
	default:	goto yy4;
	}
yy2:
	++lexer->cursor;
#line 230 "bootstrap/meta_lexis.re"
	{
        /* printf("ending\n"); */
        return 0;
        }
#line 189 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy4:
	++lexer->cursor;
yy5:
#line 226 "bootstrap/meta_lexis.re"
	{
            fprintf(stderr, "ERROR lexing: %s: %s\n", lexer->filename, lexer->cursor);
            exit(-1);
        }
#line 198 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy6:
	yyaccept = 0;
	yych = *(lexer->marker = ++lexer->cursor);
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy6;
	case '(':	goto yy28;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'q':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		yyt2 = lexer->cursor;
		goto yy19;
	case '=':	goto yy22;
	case 'd':
		yyt2 = lexer->cursor;
		goto yy25;
	case 'p':
		yyt2 = lexer->cursor;
		goto yy26;
	case 'r':
		yyt2 = lexer->cursor;
		goto yy27;
	default:	goto yy8;
	}
yy8:
#line 120 "bootstrap/meta_lexis.re"
	{ goto loop; }
#line 286 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy9:
	yyaccept = 1;
	yych = *(lexer->marker = ++lexer->cursor);
	switch (yych) {
	case 0x00:	goto yy5;
	case ' ':
	case '(':
	case ')':
	case '+':
	case ',':
	case '=':
		yyt1 = lexer->cursor;
		goto yy32;
	case '"':	goto yy34;
	default:
		yyt1 = lexer->cursor;
		goto yy30;
	}
yy10:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\n':	goto yy36;
	default:	goto yy10;
	}
yy12:
	yyaccept = 2;
	yych = *(lexer->marker = ++lexer->cursor);
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
	case ',':
		yyt1 = lexer->cursor;
		goto yy38;
	case ')':
		yyt2 = lexer->cursor;
		mtag(yyt5);
		mtag(yyt4);
		yyt1 = lexer->cursor;
		goto yy40;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		mtag(yyt4);
		yyt1 = lexer->cursor;
		goto yy43;
	default:	goto yy13;
	}
yy13:
#line 191 "bootstrap/meta_lexis.re"
	{ return LPAREN; }
#line 400 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy14:
	++lexer->cursor;
#line 192 "bootstrap/meta_lexis.re"
	{ return RPAREN; }
#line 405 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy16:
	yych = *++lexer->cursor;
	switch (yych) {
	case '=':	goto yy45;
	default:	goto yy5;
	}
yy17:
	++lexer->cursor;
#line 119 "bootstrap/meta_lexis.re"
	{ goto loop; }
#line 416 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy19:
	yych = *++lexer->cursor;
yy20:
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
		yyt3 = lexer->cursor;
		goto yy47;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy19;
	default:
		yyt3 = lexer->cursor;
		goto yy21;
	}
yy21:
	s1 = yyt2;
	s2 = yyt3;
#line 165 "bootstrap/meta_lexis.re"
	{
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return VNAME;
        }
#line 503 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy22:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy22;
	default:	goto yy24;
	}
yy24:
#line 224 "bootstrap/meta_lexis.re"
	{ return EQ; }
#line 515 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy25:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'i':	goto yy49;
	default:	goto yy20;
	}
yy26:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'a':	goto yy50;
	case 'p':	goto yy51;
	default:	goto yy20;
	}
yy27:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'e':	goto yy52;
	default:	goto yy20;
	}
yy28:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
	case ',':
		yyt1 = lexer->cursor;
		goto yy38;
	case ')':
		yyt2 = lexer->cursor;
		mtag(yyt5);
		mtag(yyt4);
		yyt1 = lexer->cursor;
		goto yy40;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		mtag(yyt4);
		yyt1 = lexer->cursor;
		goto yy43;
	default:	goto yy29;
	}
yy29:
	lexer->cursor = lexer->marker;
	switch (yyaccept) {
	case 0:
		goto yy8;
	case 1:
		goto yy5;
	case 2:
		goto yy13;
	case 3:
		yyt3 = lexer->cursor;
		goto yy21;
	default:
		goto yy21;
	}
yy30:
	yych = *++lexer->cursor;
	switch (yych) {
	case 0x00:	goto yy29;
	case ' ':
	case '(':
	case ')':
	case '+':
	case ',':
	case '=':	goto yy32;
	case '"':	goto yy53;
	default:	goto yy30;
	}
yy32:
	yych = *++lexer->cursor;
	switch (yych) {
	case 0x00:	goto yy29;
	case '"':	goto yy55;
	default:	goto yy32;
	}
yy34:
	++lexer->cursor;
	s1 = lexer->cursor - 1;
	s2 = lexer->cursor - 1;
#line 180 "bootstrap/meta_lexis.re"
	{
            mtok->s = NULL;
            return WORD;
        }
#line 664 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy36:
	++lexer->cursor;
#line 122 "bootstrap/meta_lexis.re"
	{
            /* return COMMENT; */
            goto loop;          /* skip comments */
        }
#line 672 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy38:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
	case ',':	goto yy38;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		mtag(yyt4);
		goto yy43;
	default:	goto yy29;
	}
yy40:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy40;
	default:	goto yy42;
	}
yy42:
	s1 = yyt1;
	/* f1 = yyt4; */
	/* f2 = yyt5; */
	s2 = yyt2;
#line 131 "bootstrap/meta_lexis.re"
	{
        /* we leave it to the parser to tokenize, now that we know each flag is syntactically correct */
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return FLAGS;

        }
#line 769 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy43:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
	case ',':
		mtag(yyt5);
		goto yy57;
	case ')':
		yyt2 = lexer->cursor;
		mtag(yyt5);
		goto yy40;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy43;
	default:	goto yy29;
	}
yy45:
	++lexer->cursor;
#line 223 "bootstrap/meta_lexis.re"
	{ return PLUSEQ; }
#line 854 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy47:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy47;
	default:	goto yy21;
	}
yy49:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'r':	goto yy59;
	default:	goto yy20;
	}
yy50:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'c':	goto yy60;
	default:	goto yy20;
	}
yy51:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'x':	goto yy61;
	default:	goto yy20;
	}
yy52:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'q':	goto yy62;
	default:	goto yy20;
	}
yy53:
	++lexer->cursor;
	s1 = yyt1;
	s2 = lexer->cursor - 1;
#line 170 "bootstrap/meta_lexis.re"
	{
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return WORD;
        }
#line 896 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy55:
	++lexer->cursor;
	s1 = yyt1;
	s2 = lexer->cursor - 1;
#line 175 "bootstrap/meta_lexis.re"
	{
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return WORDS;
        }
#line 906 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy57:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
	case ',':	goto yy57;
	case ')':
		yyt2 = lexer->cursor;
		goto yy40;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		mtag(yyt4);
		goto yy43;
	default:	goto yy29;
	}
yy59:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'e':	goto yy63;
	default:	goto yy20;
	}
yy60:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'k':	goto yy64;
	default:	goto yy20;
	}
yy61:
	yych = *++lexer->cursor;
	switch (yych) {
	case '_':	goto yy65;
	default:	goto yy20;
	}
yy62:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'u':	goto yy66;
	default:	goto yy20;
	}
yy63:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'c':	goto yy67;
	default:	goto yy20;
	}
yy64:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'a':	goto yy68;
	default:	goto yy20;
	}
yy65:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'r':	goto yy69;
	default:	goto yy20;
	}
yy66:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'i':	goto yy70;
	default:	goto yy20;
	}
yy67:
	yych = *++lexer->cursor;
	switch (yych) {
	case 't':	goto yy71;
	default:	goto yy20;
	}
yy68:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'g':	goto yy72;
	default:	goto yy20;
	}
yy69:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'u':	goto yy73;
	default:	goto yy20;
	}
yy70:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'r':	goto yy74;
	default:	goto yy20;
	}
yy71:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'o':	goto yy75;
	default:	goto yy20;
	}
yy72:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'e':	goto yy76;
	default:	goto yy20;
	}
yy73:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'n':	goto yy77;
	default:	goto yy20;
	}
yy74:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'e':	goto yy78;
	default:	goto yy20;
	}
yy75:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'r':	goto yy79;
	default:	goto yy20;
	}
yy76:
	yyaccept = 3;
	yych = *(lexer->marker = ++lexer->cursor);
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
		yyt3 = lexer->cursor;
		goto yy80;
	case '"':	goto yy82;
	default:	goto yy20;
	}
yy77:
	yych = *++lexer->cursor;
	switch (yych) {
	case 't':	goto yy83;
	default:	goto yy20;
	}
yy78:
	yych = *++lexer->cursor;
	switch (yych) {
	case 's':	goto yy84;
	default:	goto yy20;
	}
yy79:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'y':	goto yy86;
	default:	goto yy20;
	}
yy80:
	yyaccept = 4;
	yych = *(lexer->marker = ++lexer->cursor);
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy80;
	case '"':	goto yy82;
	default:	goto yy21;
	}
yy82:
	yych = *++lexer->cursor;
	switch (yych) {
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		yyt1 = lexer->cursor;
		goto yy87;
	default:	goto yy29;
	}
yy83:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'i':	goto yy89;
	default:	goto yy20;
	}
yy84:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
		yyt1 = lexer->cursor;
		goto yy90;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy19;
	default:
		yyt1 = lexer->cursor;
		goto yy85;
	}
yy85:
	s2 = yyt1;
	s1 = yyt1 - 8;
#line 127 "bootstrap/meta_lexis.re"
	{ return REQUIRES; }
#line 1287 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy86:
	yyaccept = 3;
	yych = *(lexer->marker = ++lexer->cursor);
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
		yyt3 = lexer->cursor;
		goto yy92;
	case '=':	goto yy94;
	default:	goto yy20;
	}
yy87:
	yych = *++lexer->cursor;
	switch (yych) {
	case '"':
		yyt2 = lexer->cursor;
		goto yy96;
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy87;
	default:	goto yy29;
	}
yy89:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'm':	goto yy99;
	default:	goto yy20;
	}
yy90:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy90;
	default:	goto yy85;
	}
yy92:
	yyaccept = 4;
	yych = *(lexer->marker = ++lexer->cursor);
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy92;
	case '=':	goto yy94;
	default:	goto yy21;
	}
yy94:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy94;
	case '"':	goto yy100;
	default:	goto yy29;
	}
yy96:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy96;
	default:	goto yy98;
	}
yy98:
	s1 = yyt1;
	s2 = yyt2;
#line 186 "bootstrap/meta_lexis.re"
	{
            mtok->s = strndup(s1, (size_t)(s2 - s1));
            return PACKAGE;
        }
#line 1421 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy99:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'e':	goto yy101;
	default:	goto yy20;
	}
yy100:
	yych = *++lexer->cursor;
	switch (yych) {
	case '+':
	case '.':
	case '/':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
		yyt1 = lexer->cursor;
		goto yy102;
	default:	goto yy29;
	}
yy101:
	yych = *++lexer->cursor;
	switch (yych) {
	case '_':	goto yy104;
	default:	goto yy20;
	}
yy102:
	yych = *++lexer->cursor;
	switch (yych) {
	case '"':
		yyt2 = lexer->cursor;
		goto yy105;
	case '+':
	case '.':
	case '/':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy102;
	default:	goto yy29;
	}
yy104:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'd':	goto yy108;
	default:	goto yy20;
	}
yy105:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy105;
	default:	goto yy107;
	}
yy107:
	s1 = yyt1;
	s2 = yyt2;
#line 149 "bootstrap/meta_lexis.re"
	{
        mtok->s = strndup(s1, (size_t)(s2-s1));
            return DIRECTORY;
        }
#line 1603 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy108:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'e':	goto yy109;
	default:	goto yy20;
	}
yy109:
	yych = *++lexer->cursor;
	switch (yych) {
	case 'p':	goto yy110;
	default:	goto yy20;
	}
yy110:
	yych = *++lexer->cursor;
	switch (yych) {
	case 's':	goto yy111;
	default:	goto yy20;
	}
yy111:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':
		yyt1 = lexer->cursor;
		goto yy113;
	case '-':
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy19;
	default:
		yyt1 = lexer->cursor;
		goto yy112;
	}
yy112:
	s2 = yyt1;
	s1 = yyt1 - 16;
#line 129 "bootstrap/meta_lexis.re"
	{ return PPX_RUNTIME_DEPS; }
#line 1704 "bazel-out/darwin-fastbuild/bin/bootstrap/meta_lexis.c"
yy113:
	yych = *++lexer->cursor;
	switch (yych) {
	case '\t':
	case '\n':
	case ' ':	goto yy113;
	default:	goto yy112;
	}
}
#line 239 "bootstrap/meta_lexis.re"

}

void meta_lexer_init(struct meta_lexer_s *lexer, const char*filename, const char *input)
{
    lexer->filename = filename;
    lexer->cursor = input;
}
