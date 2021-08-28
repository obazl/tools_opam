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
obzl_meta_property *obzl_meta_package_property(obzl_meta_package *_pkg,char *_name);
typedef struct obzl_meta_settings obzl_meta_settings;
struct obzl_meta_property {
    char     *name;
    /* UT_array *settings;         /\* array of struct obzl_meta_setting *\/ */
    obzl_meta_settings *settings;         /* array of struct obzl_meta_setting */
};
bool obzl_meta_package_has_subpackages(obzl_meta_package *_pkg);
bool obzl_meta_package_has_plugins(obzl_meta_package *_pkg);
typedef struct obzl_meta_entry obzl_meta_entry;
obzl_meta_entry *obzl_meta_entries_nth(obzl_meta_entries *_entries,int _i);
int obzl_meta_entries_count(obzl_meta_entries *_entries);
enum obzl_meta_entry_type_e { OMP_PROPERTY, OMP_PACKAGE };
typedef enum obzl_meta_entry_type_e obzl_meta_entry_type_e;
struct obzl_meta_entry {
    enum obzl_meta_entry_type_e type;
    union {
        struct obzl_meta_property *property;
        struct obzl_meta_package  *package;
    };
};
bool obzl_meta_package_has_archives(obzl_meta_package *_pkg);
obzl_meta_entries *obzl_meta_package_entries(obzl_meta_package *_pkg);
char *obzl_meta_package_src(obzl_meta_package *_pkg);
char *obzl_meta_package_dir(obzl_meta_package *_pkg);
char *obzl_meta_package_name(obzl_meta_package *_pkg);
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
#define INTERFACE 0
struct obzl_meta_settings {
    UT_array *list;             /* list of obzl_meta_setting* */
};
