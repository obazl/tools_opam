/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include "utarray.h"
#include "uthash.h"
#include "utstring.h"
typedef struct obzl_meta_flags obzl_meta_flags;
#if DEBUG_TRACE
void dump_flags(int indent,obzl_meta_flags *flags);
#endif
bool obzl_meta_flags_to_condition_name(obzl_meta_flags *flags,UT_string *_cname);
#include <stdbool.h>
char *mystrcat(char *dest,char *src);
char *obzl_meta_flags_to_comment(obzl_meta_flags *flags);
bool obzl_meta_flags_has_flag(obzl_meta_flags *_flags,char *_flag,bool polarity);
void flags_dtor(obzl_meta_flags *old_flags);
obzl_meta_flags *obzl_meta_flags_new_tokenized(char *flags);
obzl_meta_flags *obzl_meta_flags_new_copy(obzl_meta_flags *old_flags);
obzl_meta_flags *obzl_meta_flags_new(void);
typedef struct obzl_meta_flag obzl_meta_flag;
bool obzl_meta_flag_polarity(obzl_meta_flag *flag);
char *obzl_meta_flag_name(obzl_meta_flag *flag);
obzl_meta_flag *obzl_meta_flags_nth(obzl_meta_flags *_flags,int _i);
int obzl_meta_flags_count(obzl_meta_flags *_flags);
#define EXPORT
void flag_dtor(void *_elt);
void flag_copy(void *_dst,const void *_src);
extern UT_icd flag_icd;
struct obzl_meta_flag {
    bool polarity;
    char *s;
};
typedef struct config_setting config_setting;
extern struct config_setting *the_config_settings;
struct obzl_meta_flags {
    UT_array *list;
};
struct config_setting {
    char name[128];              /* key */
    char label[128];
    obzl_meta_flags *flags;
    UT_hash_handle hh;
};
#define INTERFACE 0
#define EXPORT_INTERFACE 0
