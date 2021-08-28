/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
typedef struct obzl_meta_package obzl_meta_package;
char *obzl_meta_package_src(obzl_meta_package *_pkg);
char *obzl_meta_package_dir(obzl_meta_package *_pkg);
char *obzl_meta_package_name(obzl_meta_package *_pkg);
#define EXPORT
#include <stdio.h>
typedef struct obzl_meta_entries obzl_meta_entries;
void handle_directory_property(FILE *ostream,int level,char *_repo,char *_pkg_src,char *_pkg_name,obzl_meta_entries *_entries);
#include "utarray.h"
#include "uthash.h"
#include "utstring.h"
typedef struct config_setting config_setting;
extern struct config_setting *the_config_settings;
extern UT_array *neg_flags;
extern UT_array *pos_flags;
typedef struct obzl_meta_flags obzl_meta_flags;
struct config_setting {
    char name[128];              /* key */
    char label[128];
    obzl_meta_flags *flags;
    UT_hash_handle hh;
};
typedef struct config_flag config_flag;
struct config_flag {
    char name[32];              /* key */
    char repo[16];
    char package[64];
    char target[32];
    char label[64];
    UT_hash_handle hh;
};
void emit_bazel_config_setting_rules(char *pfx);
#include <stdbool.h>
char *mkdir_r(char *base,char *path);
extern char outdir[PATH_MAX];
void emit_bazel_flags_mt(char *pfx);
void emit_build_bazel(char *_tgtroot,char *_repo,char *_pkg_prefix,char *_pkg_path,struct obzl_meta_package *_pkg);
typedef struct obzl_meta_entry obzl_meta_entry;
obzl_meta_entry *obzl_meta_entries_nth(obzl_meta_entries *_entries,int _i);
int obzl_meta_entries_count(obzl_meta_entries *_entries);
enum obzl_meta_entry_type_e { OMP_PROPERTY, OMP_PACKAGE };
typedef enum obzl_meta_entry_type_e obzl_meta_entry_type_e;
typedef struct obzl_meta_property obzl_meta_property;
struct obzl_meta_entry {
    enum obzl_meta_entry_type_e type;
    union {
        struct obzl_meta_property *property;
        struct obzl_meta_package  *package;
    };
};
void emit_bazel_subpackages(char *_tgtroot,char *_repo,char *_pkg_prefix,char *_pkg_path,struct obzl_meta_package *_pkg);
typedef struct obzl_meta_setting obzl_meta_setting;
int obzl_meta_setting_has_flag(obzl_meta_setting *_setting,char *_flag,bool polarity);
void emit_bazel_dummy_target(FILE *ostream,int level,char *_repo,char *_pkg_src,char *_pkg_path,char *_pkg_name,obzl_meta_entries *_entries);
void emit_bazel_path_attrib(FILE *ostream,int level,char *repo,char *_pkg_src,char *pkg,obzl_meta_entries *_entries);
char *obzl_meta_flags_to_comment(obzl_meta_flags *flags);
bool obzl_meta_flags_has_flag(obzl_meta_flags *_flags,char *_flag,bool polarity);
typedef struct obzl_meta_settings obzl_meta_settings;
int obzl_meta_settings_flag_count(obzl_meta_settings *_settings,char *_flag,bool polarity);
void emit_bazel_plugin_rule(FILE *ostream,int level,char *_repo,char *_pkg_src,char *_pkg_path,char *_pkg_name,obzl_meta_entries *_entries);
void emit_bazel_deps_adjunct(FILE *ostream,int level,char *repo,char *_pkg_prefix,obzl_meta_entries *_entries);
void emit_bazel_deps(FILE *ostream,int level,char *repo,char *pkg,obzl_meta_entries *_entries);
void emit_bazel_archive_rule(FILE *ostream,int level,char *_repo,char *_pkg_src,char *_pkg_path,char *_pkg_name,obzl_meta_entries *_entries);
void emit_bazel_ppx_dummy_deps(FILE *ostream,int level,char *repo,char *pkg,obzl_meta_entries *_entries);
void emit_bazel_ppx_dummy_rule(FILE *ostream,int level,char *_repo,char *_pkg_src,char *_pkg_path,char *_pkg_name,obzl_meta_entries *_entries);
void emit_bazel_metadatum(FILE *ostream,int level,char *repo,char *pkg,obzl_meta_entries *_entries,char *_property,char *_attrib);
typedef char *obzl_meta_value;
typedef struct obzl_meta_values obzl_meta_values;
obzl_meta_value *obzl_meta_values_nth(obzl_meta_values *_values,int _i);
int obzl_meta_values_count(obzl_meta_values *_values);
bool obzl_meta_flags_to_condition_name(obzl_meta_flags *flags,UT_string *_cname);
void register_flags(obzl_meta_flags *_flags);
char *obzl_meta_property_name(obzl_meta_property *prop);
obzl_meta_settings *obzl_meta_property_settings(obzl_meta_property *prop);
char *mystrcat(char *dest,char *src);
obzl_meta_value obzl_meta_property_value(obzl_meta_property *prop);
obzl_meta_property *obzl_meta_entries_property(obzl_meta_entries *_entries,char *_name);
struct obzl_meta_property {
    char     *name;
    /* UT_array *settings;         /\* array of struct obzl_meta_setting *\/ */
    obzl_meta_settings *settings;         /* array of struct obzl_meta_setting */
};
struct obzl_meta_entries {
    UT_array *list;          /* list of obzl_meta_entry */
};
void emit_bazel_attribute(FILE *ostream,int level,char *_repo,char *_pkg_src,char *_pkg_path,char *_pkg_name,obzl_meta_entries *_entries,char *property);
obzl_meta_flags *obzl_meta_setting_flags(obzl_meta_setting *_setting);
obzl_meta_setting *obzl_meta_settings_nth(obzl_meta_settings *_settings,int _i);
typedef struct obzl_meta_flag obzl_meta_flag;
obzl_meta_flag *obzl_meta_flags_nth(obzl_meta_flags *_flags,int _i);
struct obzl_meta_flag {
    bool polarity;
    char *s;
};
int obzl_meta_flags_count(obzl_meta_flags *_flags);
int obzl_meta_settings_count(obzl_meta_settings *_settings);
obzl_meta_values *obzl_meta_setting_values(obzl_meta_setting *_setting);
struct obzl_meta_settings {
    UT_array *list;             /* list of obzl_meta_setting* */
};
struct obzl_meta_flags {
    UT_array *list;
};
enum obzl_meta_opcode_e { OP_SET, OP_UPDATE };
typedef enum obzl_meta_opcode_e obzl_meta_opcode_e;
struct obzl_meta_setting {
    obzl_meta_flags *flags;     /* FIXME: NULL if no flags breaks obzl_meta_flags_count on settings */
    /* UT_array *flags;       /\* array of struct obzl_meta_flag *\/ */
    enum obzl_meta_opcode_e opcode;
    obzl_meta_values *values;
    /* UT_array *values;            /\* array of strings *\/ */
};
obzl_meta_values *resolve_setting_values(obzl_meta_setting *_setting,obzl_meta_flags *_flags,obzl_meta_settings *_settings);
struct obzl_meta_values {
    UT_array *list;             /* list of strings  */
};
struct obzl_meta_package {
    char *name;
    char *directory;
    char *metafile;
    obzl_meta_entries *entries;          /* list of struct obzl_meta_entry */
};
void emit_bazel_hdr(FILE *ostream,int level,char *repo,char *pkg_prefix,obzl_meta_package *_pkg);
extern bool stdlib_root;
#define EXPORT_INTERFACE 0
