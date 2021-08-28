/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#if defined(LINUX                    /* FIXME */)
#include <linux/limits.h>
#endif
#if !(defined(LINUX                    /* FIXME */))
#include <limits.h>
#endif
char *obzl_meta_version();
#if !defined(Parse_ENGINEALWAYSONSTACK)
void ParseFree(void *p,void(*freeProc)(void *));
#endif
typedef union meta_token meta_token;
#define ParseTOKENTYPE  union meta_token* 
typedef struct obzl_meta_package obzl_meta_package;
#define ParseARG_PDECL , struct obzl_meta_package *the_root_pkg
void Parse(void *yyp,int yymajor,ParseTOKENTYPE yyminor ParseARG_PDECL);
#define FLAGS                           6
typedef struct meta_lexer meta_lexer;
int get_next_token(struct meta_lexer *lexer,union meta_token *mtok);
union meta_token {
    char *s;
};
#if !defined(YYMALLOCARGTYPE)
#define YYMALLOCARGTYPE size_t
#endif
#define ParseCTX_PDECL
#if !defined(Parse_ENGINEALWAYSONSTACK)
void *ParseAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)ParseCTX_PDECL);
#endif
void lexer_init(struct meta_lexer *lexer,const char *filename,const char *input);
struct meta_lexer {
    const char *filename;
    const char *tok;
    const char *cursor;
    const char *limit;
    const char *marker;
};
struct obzl_meta_package *obzl_meta_parse_file(char *fname);
bool is_empty(const char *s);
#define LOCAL static
LOCAL char *package_name_from_file_name(char *fname);
extern struct obzl_meta_package *MAIN_PKG;
typedef struct obzl_meta_entries obzl_meta_entries;
struct obzl_meta_package {
    char *name;
    char *directory;
    char *metafile;
    obzl_meta_entries *entries;          /* list of struct obzl_meta_entry */
};
typedef struct logging logging;
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
extern struct logging logger;
#define EXPORT
#define WORDS                           8
#define WORD                            7
#define WARNING                        11
#define VNAME                           5
#define VERSION                         2
#define RPAREN                         17
#define REQUIRES                        9
#define PLUSEQ                         15
#define PACKAGE                         1
#define LPAREN                         16
#define ERROR                          12
#define EQ                             14
#define DQ                             13
#define DIRECTORY                       4
#define DESCRIPTION                     3
extern char *token_names[256];
#define TOKEN_NAME(x) (char*)#x
#define EXPORT_INTERFACE 0
#include "uthash.h"
#include <stdbool.h>
#include "utarray.h"
extern int errnum;
extern int errnum;
extern int errnum;
#define INTERFACE 0
struct obzl_meta_entries {
    UT_array *list;          /* list of obzl_meta_entry */
};
