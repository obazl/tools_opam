/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include <stdbool.h>
#include "utarray.h"
typedef struct obzl_meta_entries obzl_meta_entries;
#if DEBUG_TRACE
void dump_entries(int indent,struct obzl_meta_entries *entries);
#endif
typedef struct obzl_meta_package obzl_meta_package;
#if DEBUG_TRACE
void dump_package(int indent,struct obzl_meta_package *pkg);
#endif
typedef struct obzl_meta_property obzl_meta_property;
#if DEBUG_TRACE
void dump_property(int indent,struct obzl_meta_property *prop);
#endif
typedef struct obzl_meta_entry obzl_meta_entry;
#if DEBUG_TRACE
void dump_entry(int indent,struct obzl_meta_entry *entry);
#endif
obzl_meta_package *obzl_meta_entry_package(obzl_meta_entry *e);
obzl_meta_property *obzl_meta_entry_property(obzl_meta_entry *e);
enum obzl_meta_entry_type_e { OMP_PROPERTY, OMP_PACKAGE };
typedef enum obzl_meta_entry_type_e obzl_meta_entry_type_e;
enum obzl_meta_entry_type_e obzl_meta_entry_type(obzl_meta_entry *e);
obzl_meta_entry *obzl_meta_entry_new();
void normalize_entries(obzl_meta_entries *entries,obzl_meta_entry *_entry);
obzl_meta_property *obzl_meta_entries_property(obzl_meta_entries *_entries,char *_name);
obzl_meta_entry *obzl_meta_entries_nth(obzl_meta_entries *_entries,int _i);
int obzl_meta_entries_count(obzl_meta_entries *_entries);
void entry_dtor(void *_elt);
void entry_copy(void *_dst,const void *_src);
extern UT_icd entry_icd;
#define EXPORT
struct obzl_meta_entries {
    UT_array *list;          /* list of obzl_meta_entry */
};
struct obzl_meta_package {
    char *name;
    char *directory;
    char *metafile;
    obzl_meta_entries *entries;          /* list of struct obzl_meta_entry */
};
typedef struct obzl_meta_settings obzl_meta_settings;
struct obzl_meta_property {
    char     *name;
    /* UT_array *settings;         /\* array of struct obzl_meta_setting *\/ */
    obzl_meta_settings *settings;         /* array of struct obzl_meta_setting */
};
struct obzl_meta_entry {
    enum obzl_meta_entry_type_e type;
    union {
        struct obzl_meta_property *property;
        struct obzl_meta_package  *package;
    };
};
#define LOCAL static
#define INTERFACE 0
struct obzl_meta_settings {
    UT_array *list;             /* list of obzl_meta_setting* */
};
