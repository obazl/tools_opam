/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
typedef struct obzl_meta_settings obzl_meta_settings;
#if DEBUG_TRACE
void dump_settings(int indent,obzl_meta_settings *settings);
#endif
typedef struct obzl_meta_values obzl_meta_values;
#if DEBUG_TRACE
void dump_values(int indent,obzl_meta_values *values);
#endif
#include "utarray.h"
#include "uthash.h"
#include "utstring.h"
typedef struct obzl_meta_flags obzl_meta_flags;
#if DEBUG_TRACE
void dump_flags(int indent,obzl_meta_flags *flags);
#endif
void obzl_meta_settings_dtor(void *_elt);
void flags_dtor(obzl_meta_flags *old_flags);
obzl_meta_values *obzl_meta_values_new_copy(obzl_meta_values *values);
obzl_meta_flags *obzl_meta_flags_new_copy(obzl_meta_flags *old_flags);
struct obzl_meta_settings *obzl_meta_settings_new();
typedef struct obzl_meta_setting obzl_meta_setting;
#if DEBUG_TRACE
void dump_setting(int indent,struct obzl_meta_setting *setting);
#endif
obzl_meta_flags *obzl_meta_flags_new_tokenized(char *flags);
enum obzl_meta_opcode_e { OP_SET, OP_UPDATE };
typedef enum obzl_meta_opcode_e obzl_meta_opcode_e;
struct obzl_meta_setting *obzl_meta_setting_new(char *flags,enum obzl_meta_opcode_e opcode,obzl_meta_values *values);
obzl_meta_values *obzl_meta_setting_values(obzl_meta_setting *_setting);
enum obzl_meta_opcode_e obzl_meta_setting_opcode(obzl_meta_setting *_setting);
obzl_meta_setting *obzl_meta_settings_nth(obzl_meta_settings *_settings,int _i);
bool obzl_meta_flags_has_flag(obzl_meta_flags *_flags,char *_flag,bool polarity);
obzl_meta_flags *obzl_meta_setting_flags(obzl_meta_setting *_setting);
int obzl_meta_setting_has_flag(obzl_meta_setting *_setting,char *_flag,bool polarity);
int obzl_meta_settings_flag_count(obzl_meta_settings *_settings,char *_flag,bool polarity);
int obzl_meta_settings_count(obzl_meta_settings *_settings);
#define EXPORT
void obzl_meta_setting_dtor(void *_elt);
void obzl_meta_setting_copy(void *_dst,const void *_src);
extern UT_icd obzl_meta_setting_icd;
struct obzl_meta_settings {
    UT_array *list;             /* list of obzl_meta_setting* */
};
struct obzl_meta_values {
    UT_array *list;             /* list of strings  */
};
struct obzl_meta_flags {
    UT_array *list;
};
struct obzl_meta_setting {
    obzl_meta_flags *flags;     /* FIXME: NULL if no flags breaks obzl_meta_flags_count on settings */
    /* UT_array *flags;       /\* array of struct obzl_meta_flag *\/ */
    enum obzl_meta_opcode_e opcode;
    obzl_meta_values *values;
    /* UT_array *values;            /\* array of strings *\/ */
};
#define INTERFACE 0
