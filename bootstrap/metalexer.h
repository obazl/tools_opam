/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
typedef struct meta_lexer meta_lexer;
void lexer_init(struct meta_lexer *lexer,const char *filename,const char *input);
#define PPX_RUNTIME_DEPS               10
#define DIRECTORY                       4
#define PACKAGE                         1
#define REQUIRES                        9
#define WORDS                           8
#define PLUSEQ                         15
#define FLAGS                           6
#define WORD                            7
#define EQ                             14
#define VNAME                           5
#define RPAREN                         17
#define LPAREN                         16
typedef union meta_token meta_token;
int get_next_token(struct meta_lexer *lexer,union meta_token *mtok);
struct meta_lexer {
    const char *filename;
    const char *tok;
    const char *cursor;
    const char *limit;
    const char *marker;
};
union meta_token {
    char *s;
};
extern int curr_tag;
#if !defined(MAX_DEPS)
#define MAX_DEPS 64
#endif
extern const char *deps[MAX_DEPS];
#define BUFSIZE 1024
#define EXPORT_INTERFACE 0
