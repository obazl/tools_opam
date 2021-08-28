#include <stdbool.h>
#include <stdio.h>
#include "utarray.h"
#include "uthash.h"

#include "log.h"
#include "meta_settings.h"

static int indent = 2;
static int delta = 2;
static char *sp = " ";


#if INTERFACE
enum obzl_meta_opcode_e { OP_SET, OP_UPDATE };
struct obzl_meta_setting {
    obzl_meta_flags *flags;     /* FIXME: NULL if no flags breaks obzl_meta_flags_count on settings */
    /* UT_array *flags;       /\* array of struct obzl_meta_flag *\/ */
    enum obzl_meta_opcode_e opcode;
    obzl_meta_values *values;
    /* UT_array *values;            /\* array of strings *\/ */
};

struct obzl_meta_settings {
    UT_array *list;             /* list of obzl_meta_setting* */
};
#endif

UT_icd obzl_meta_setting_icd = {
    sizeof(obzl_meta_setting), NULL,
    obzl_meta_setting_copy,
    obzl_meta_setting_dtor
};

/* **************************************************************** */
EXPORT int obzl_meta_settings_count(obzl_meta_settings *_settings)
{
    return utarray_len(_settings->list);
}

EXPORT int obzl_meta_settings_flag_count(obzl_meta_settings *_settings, char *_flag, bool polarity)
{
    int ct = 0;
    obzl_meta_setting *setting = NULL;
    obzl_meta_flags *flags;
    for(setting  = utarray_front(_settings->list);
        setting != NULL;
        setting  = utarray_next(_settings->list, setting)) {

        if (obzl_meta_setting_has_flag(setting, _flag, polarity)) {
            ct++;
        }
    }
    return ct;
}

EXPORT int obzl_meta_setting_has_flag(obzl_meta_setting *_setting, char *_flag, bool polarity)
{
    int ct;
    obzl_meta_flags *flags = obzl_meta_setting_flags(_setting);
    return obzl_meta_flags_has_flag(flags, _flag, polarity);
}

EXPORT obzl_meta_setting *obzl_meta_settings_nth(obzl_meta_settings *_settings, int _i)
{
    return utarray_eltptr(_settings->list, _i);
}

EXPORT obzl_meta_flags *obzl_meta_setting_flags(obzl_meta_setting *_setting)
{
    return _setting->flags;
}

EXPORT enum obzl_meta_opcode_e obzl_meta_setting_opcode(obzl_meta_setting *_setting)
{
    return _setting->opcode;
}

EXPORT obzl_meta_values *obzl_meta_setting_values(obzl_meta_setting *_setting)
{
    return _setting->values;
}

struct obzl_meta_setting *obzl_meta_setting_new(char *flags,
                                                enum obzl_meta_opcode_e opcode,
                                                obzl_meta_values *values)
{
#if DEBUG_TRACE
    /* log_trace("obzl_meta_setting_new()"); //, flags: %s", flags); */
#endif
    struct obzl_meta_setting *new_setting = (struct obzl_meta_setting*)malloc(sizeof(struct obzl_meta_setting));
    if (flags == NULL)
        new_setting->flags  = NULL;
    else
        new_setting->flags  = obzl_meta_flags_new_tokenized(flags);
    new_setting->opcode = opcode;
    new_setting->values = values;
#if DEBUG_TRACE
    log_trace("obzl_meta_setting_new done; dumping:");
    dump_setting(0, new_setting);
#endif
    return new_setting;
}

struct obzl_meta_settings *obzl_meta_settings_new()
{
#if DEBUG_TRACE
    /* log_trace("obzl_meta_settings_new()"); */
#endif
    struct obzl_meta_settings *new_settings = (struct obzl_meta_settings*)malloc(sizeof(struct obzl_meta_settings));
    utarray_new(new_settings->list, &obzl_meta_setting_icd);
#if DEBUG_TRACE
    /* dump_settings(0, new_settings); */
    /* log_trace("obzl_meta_settings_new() done"); */
#endif
    return new_settings;
}

/* void obzl_meta_setting_copy(obzl_meta_setting *dst, const obzl_meta_setting *src) { */
void obzl_meta_setting_copy(void *_dst, const void *_src) {
#if DEBUG_TRACE
    log_trace("obzl_meta_setting_copy(dst=%p,  src=%p)", _dst, _src);
#endif
    struct obzl_meta_setting *dst = (struct obzl_meta_setting*)_dst;
    struct obzl_meta_setting *src = (struct obzl_meta_setting*)_src;

    /* dump_setting(0, src); */

    /* dst->flags = obzl_meta_flags_new(); */
    if (src->flags != NULL) {
        dst->flags  = obzl_meta_flags_new_copy(src->flags);
    } else {
        dst->flags = NULL;
    }

    dst->opcode = (enum obzl_meta_opcode_e)src->opcode;

    if ( src->values != NULL) {
        dst->values = obzl_meta_values_new_copy(src->values);
    } else {
        dst->values = NULL;
        /* utarray_new(dst->values->list, &ut_str_icd); */
        /* char **old_str = NULL; */
        /* while ( (old_str=(char **)utarray_next(src->values->list, old_str))) { */
        /*     /\* log_trace("copying value: %s", *old_str); *\/ */
        /*     utarray_push_back(dst->values->list, old_str); */
        /* } */
    /* } else { */
    /*     log_trace("no vals..."); */
    }
}

void obzl_meta_setting_dtor(void *_elt) {
    struct obzl_meta_setting *elt = (struct obzl_meta_setting*)_elt;
    flags_dtor(elt->flags);
}

void obzl_meta_settings_dtor(void *_elt) {
    struct obzl_meta_settings *elt = (struct obzl_meta_settings*)_elt;
    obzl_meta_setting *setting = NULL;
    while ( (setting=(obzl_meta_setting*)utarray_next(elt->list, setting))) {
        obzl_meta_setting_dtor(setting);
    }
    free(elt);
}

#if DEBUG_TRACE
void dump_setting(int indent, struct obzl_meta_setting *setting)
{
    log_trace("%*ssetting:", indent, sp);
    dump_flags(2*delta+indent, setting->flags);
    log_debug("%*sopcode: %d", delta+indent, sp, setting->opcode);
    dump_values(2*delta+indent, setting->values);
    /* log_trace("%*sdump_setting() finished", indent, sp); */
}
#endif

#if DEBUG_TRACE
void dump_settings(int indent, obzl_meta_settings *settings)
{
    log_trace("%*ssettings:", indent, sp);
    obzl_meta_setting *setting = NULL;
    for(setting  = utarray_front(settings->list);
        setting != NULL;
        setting  = utarray_next(settings->list, setting)) {
        dump_setting(delta+indent, setting);
    }
}
#endif
