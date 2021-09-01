/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include "uthash.h"
void log_fn(log_Event *evt);
#include <stdio.h>
typedef struct obzl_meta_package obzl_meta_package;
void emit_build_bazel(char *_tgtroot,char *_repo,char *_pkg_prefix,char *_pkg_path,struct obzl_meta_package *_pkg);
char *obzl_meta_package_name(obzl_meta_package *_pkg);
#include <stdbool.h>
#include "utarray.h"
typedef struct obzl_meta_property obzl_meta_property;
typedef struct obzl_meta_entries obzl_meta_entries;
obzl_meta_property *obzl_meta_entries_property(obzl_meta_entries *_entries,char *_name);
extern bool stdlib_root;
#if defined(LINUX                    /* FIXME */)
#include <linux/limits.h>
#endif
#if !(defined(LINUX                    /* FIXME */))
#include <limits.h>
#endif
struct obzl_meta_package *obzl_meta_parse_file(char *fname);
struct obzl_meta_package {
    char *name;
    char *directory;
    char *metafile;
    obzl_meta_entries *entries;          /* list of struct obzl_meta_entry */
};
#include "utstring.h"
typedef struct obzl_meta_flag obzl_meta_flag;
typedef struct obzl_meta_flags obzl_meta_flags;
obzl_meta_flag *obzl_meta_flags_nth(obzl_meta_flags *_flags,int _i);
struct obzl_meta_flag {
    bool polarity;
    char *s;
};
int obzl_meta_flags_count(obzl_meta_flags *_flags);
void register_flags(obzl_meta_flags *_flags);
typedef struct config_setting config_setting;
extern struct config_setting *the_config_settings;
struct config_setting {
    char name[128];              /* key */
    char label[128];
    obzl_meta_flags *flags;
    UT_hash_handle hh;
};
struct obzl_meta_flags {
    UT_array *list;
};
void register_condition_name(char *_name,obzl_meta_flags *_flags);
void emit_bazel_config_setting_rules(char *pfx);
int handle_lib_meta(char *rootdir,char *pkgdir,char *metafile);
typedef int(*file_handler)(char *rootdir,char *pkg,char *metafile);
int meta_walk(char *srcroot,bool linkfiles,file_handler handle_meta);
char *mkdir_r(char *base,char *path);
char *mystrcat(char *dest,char *src);
extern char outdir[PATH_MAX];
void opam_config(char *_opam_switch,char *outdir);
char *run_cmd(char *cmd);
void initialize_config_flags();
int strsort(const void *_a,const void *_b);
extern struct fileset_s *filesets;
extern struct buildfile_s *buildfiles;
extern char symlink_tgt[PATH_MAX];
extern char symlink_src[PATH_MAX];
extern char coqlib[PATH_MAX];
extern char basedir[PATH_MAX];
extern UT_array *opam_packages;
extern char tgtroot_lib[PATH_MAX];
extern char tgtroot_bin[PATH_MAX];
extern char tgtroot[PATH_MAX];
extern char work_buf[PATH_MAX];
extern UT_array *neg_flags;
extern UT_array *pos_flags;
typedef struct config_flag config_flag;
extern struct config_flag *the_flag_table;
struct config_flag {
    char name[32];              /* key */
    char repo[16];
    char package[64];
    char target[32];
    char label[64];
    UT_hash_handle hh;
};
extern bool g_ppx_pkg;
extern int rc;
extern int rc;
extern int errnum;
extern int errnum;
extern int errnum;
#define INTERFACE 0
struct obzl_meta_entries {
    UT_array *list;          /* list of obzl_meta_entry */
};
typedef struct obzl_meta_settings obzl_meta_settings;
struct obzl_meta_property {
    char     *name;
    /* UT_array *settings;         /\* array of struct obzl_meta_setting *\/ */
    obzl_meta_settings *settings;         /* array of struct obzl_meta_setting */
};
struct obzl_meta_settings {
    UT_array *list;             /* list of obzl_meta_setting* */
};
