/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
int ParseFallback(int iToken);
typedef union meta_token meta_token;
#define ParseTOKENTYPE  union meta_token* 
typedef struct obzl_meta_package obzl_meta_package;
#define ParseARG_PDECL , struct obzl_meta_package *the_root_pkg
void Parse(void *yyp,int yymajor,ParseTOKENTYPE yyminor ParseARG_PDECL);
typedef struct obzl_meta_values obzl_meta_values;
obzl_meta_values *obzl_meta_values_new_tokenized(char *valstr);
obzl_meta_values *obzl_meta_values_new(char *valstr);
typedef struct obzl_meta_setting obzl_meta_setting;
enum obzl_meta_opcode_e { OP_SET, OP_UPDATE };
typedef enum obzl_meta_opcode_e obzl_meta_opcode_e;
struct obzl_meta_setting *obzl_meta_setting_new(char *flags,enum obzl_meta_opcode_e opcode,obzl_meta_values *values);
typedef struct obzl_meta_flags obzl_meta_flags;
struct obzl_meta_setting {
    obzl_meta_flags *flags;     /* FIXME: NULL if no flags breaks obzl_meta_flags_count on settings */
    /* UT_array *flags;       /\* array of struct obzl_meta_flag *\/ */
    enum obzl_meta_opcode_e opcode;
    obzl_meta_values *values;
    /* UT_array *values;            /\* array of strings *\/ */
};
typedef struct obzl_meta_property obzl_meta_property;
struct obzl_meta_property *obzl_meta_property_new(char *name);
typedef struct obzl_meta_entry obzl_meta_entry;
struct obzl_meta_entry *handle_primitive_prop(int token_type,union meta_token *token);
struct obzl_meta_entry *handle_simple_prop(union meta_token *token,enum obzl_meta_opcode_e opcode,union meta_token *word);
#include <stdbool.h>
#include "utarray.h"
typedef struct obzl_meta_entries obzl_meta_entries;
void normalize_entries(obzl_meta_entries *entries,obzl_meta_entry *_entry);
extern UT_icd entry_icd;
#if DEBUG_TRACE
void dump_entry(int indent,struct obzl_meta_entry *entry);
void dump_package(int indent,struct obzl_meta_package *pkg);
void dump_entries(int indent,struct obzl_meta_entries *entries);
#endif
#if defined(YYCOVERAGE)
int ParseCoverage(FILE *out);
#endif
#if defined(YYTRACKMAXSTACKDEPTH)
int ParseStackPeak(void *p);
#endif
#if !defined(Parse_ENGINEALWAYSONSTACK)
void ParseFree(void *p,void(*freeProc)(void *));
#endif
void ParseFinalize(void *p);
#if !defined(YYMALLOCARGTYPE)
#define YYMALLOCARGTYPE size_t
#endif
#define ParseCTX_PDECL
#if !defined(Parse_ENGINEALWAYSONSTACK)
void *ParseAlloc(void *(*mallocProc)(YYMALLOCARGTYPE)ParseCTX_PDECL);
#endif
void ParseInit(void *yypRawParser ParseCTX_PDECL);
#if !defined(NDEBUG)
void ParseTrace(FILE *TraceFILE,char *zTracePrompt);
#endif
#define ParseCTX_STORE
#define ParseCTX_FETCH
#define ParseCTX_PARAM
#define ParseCTX_SDECL
#define ParseARG_STORE yypParser->the_root_pkg=the_root_pkg;
#define ParseARG_FETCH  struct obzl_meta_package *the_root_pkg=yypParser->the_root_pkg;
#define ParseARG_PARAM ,the_root_pkg
#define ParseARG_SDECL  struct obzl_meta_package *the_root_pkg;
enum obzl_meta_entry_type_e { OMP_PROPERTY, OMP_PACKAGE };
typedef enum obzl_meta_entry_type_e obzl_meta_entry_type_e;
struct obzl_meta_entry {
    enum obzl_meta_entry_type_e type;
    union {
        struct obzl_meta_property *property;
        struct obzl_meta_package  *package;
    };
};
struct obzl_meta_entries {
    UT_array *list;          /* list of obzl_meta_entry */
};
struct obzl_meta_values {
    UT_array *list;             /* list of strings  */
};
typedef struct obzl_meta_flag obzl_meta_flag;
#include "uthash.h"
#include "utstring.h"
struct obzl_meta_flag {
    bool polarity;
    char *s;
};
typedef struct obzl_meta_settings obzl_meta_settings;
struct obzl_meta_property {
    char     *name;
    /* UT_array *settings;         /\* array of struct obzl_meta_setting *\/ */
    obzl_meta_settings *settings;         /* array of struct obzl_meta_setting */
};
struct obzl_meta_package {
    char *name;
    char *directory;
    char *metafile;
    obzl_meta_entries *entries;          /* list of struct obzl_meta_entry */
};
union meta_token {
    char *s;
};
#define RPAREN                         17
#define LPAREN                         16
#define PLUSEQ                         15
#define EQ                             14
#define DQ                             13
#define ERROR                          12
#define WARNING                        11
#define PPX_RUNTIME_DEPS               10
#define REQUIRES                        9
#define WORDS                           8
#define WORD                            7
#define FLAGS                           6
#define VNAME                           5
#define DIRECTORY                       4
#define DESCRIPTION                     3
#define VERSION                         2
#define PACKAGE                         1
#define INTERFACE 0
struct obzl_meta_flags {
    UT_array *list;
};
struct obzl_meta_settings {
    UT_array *list;             /* list of obzl_meta_setting* */
};
