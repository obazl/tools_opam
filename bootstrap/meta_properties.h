/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#if DEBUG_TRACE
void dump_properties(int indent,UT_array *props);
#endif
typedef struct obzl_meta_settings obzl_meta_settings;
#if DEBUG_TRACE
void dump_settings(int indent,obzl_meta_settings *settings);
#endif
enum obzl_meta_opcode_e { OP_SET, OP_UPDATE };
typedef enum obzl_meta_opcode_e obzl_meta_opcode_e;
typedef struct obzl_meta_entry obzl_meta_entry;
typedef union meta_token meta_token;
struct obzl_meta_entry *handle_simple_prop(union meta_token *token,enum obzl_meta_opcode_e opcode,union meta_token *word);
typedef struct obzl_meta_property obzl_meta_property;
#if DEBUG_TRACE
void dump_property(int indent,struct obzl_meta_property *prop);
#endif
typedef struct obzl_meta_setting obzl_meta_setting;
#if DEBUG_TRACE
void dump_setting(int indent,struct obzl_meta_setting *setting);
#endif
typedef struct obzl_meta_values obzl_meta_values;
struct obzl_meta_setting *obzl_meta_setting_new(char *flags,enum obzl_meta_opcode_e opcode,obzl_meta_values *values);
obzl_meta_values *obzl_meta_values_new(char *valstr);
struct obzl_meta_values {
    UT_array *list;             /* list of strings  */
};
#if defined(LINUX                    /* FIXME */)
#include <linux/limits.h>
#endif
#if !(defined(LINUX                    /* FIXME */))
#include <limits.h>
#endif
extern char *token_names[256];
union meta_token {
    char *s;
};
struct obzl_meta_entry *handle_primitive_prop(int token_type,union meta_token *token);
#include <stdbool.h>
#include "utarray.h"
enum obzl_meta_entry_type_e { OMP_PROPERTY, OMP_PACKAGE };
typedef enum obzl_meta_entry_type_e obzl_meta_entry_type_e;
typedef struct obzl_meta_package obzl_meta_package;
struct obzl_meta_entry {
    enum obzl_meta_entry_type_e type;
    union {
        struct obzl_meta_property *property;
        struct obzl_meta_package  *package;
    };
};
void obzl_meta_settings_dtor(void *_elt);
extern UT_icd obzl_meta_setting_icd;
struct obzl_meta_settings *obzl_meta_settings_new();
struct obzl_meta_property *obzl_meta_property_new(char *name);
obzl_meta_settings *obzl_meta_property_settings(obzl_meta_property *prop);
typedef struct obzl_meta_flags obzl_meta_flags;
struct obzl_meta_setting {
    obzl_meta_flags *flags;     /* FIXME: NULL if no flags breaks obzl_meta_flags_count on settings */
    /* UT_array *flags;       /\* array of struct obzl_meta_flag *\/ */
    enum obzl_meta_opcode_e opcode;
    obzl_meta_values *values;
    /* UT_array *values;            /\* array of strings *\/ */
};
typedef char *obzl_meta_value;
obzl_meta_value obzl_meta_property_value(obzl_meta_property *prop);
char *obzl_meta_property_name(obzl_meta_property *prop);
#define EXPORT
void property_dtor(void *_elt);
void property_copy(void *_dst,const void *_src);
extern UT_icd property_icd;
struct obzl_meta_settings {
    UT_array *list;             /* list of obzl_meta_setting* */
};
struct obzl_meta_property {
    char     *name;
    /* UT_array *settings;         /\* array of struct obzl_meta_setting *\/ */
    obzl_meta_settings *settings;         /* array of struct obzl_meta_setting */
};
#define INTERFACE 0
#include "uthash.h"
#include "utstring.h"
struct obzl_meta_flags {
    UT_array *list;
};
typedef struct obzl_meta_entries obzl_meta_entries;
struct obzl_meta_package {
    char *name;
    char *directory;
    char *metafile;
    obzl_meta_entries *entries;          /* list of struct obzl_meta_entry */
};
struct obzl_meta_entries {
    UT_array *list;          /* list of obzl_meta_entry */
};
