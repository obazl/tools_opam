#include <stdbool.h>
#include <stdio.h>

#if EXPORT_INTERFACE
#include "utarray.h"
#include "uthash.h"
#include "utstring.h"
#endif

#include "log.h"
#include "meta_flags.h"

static int indent = 2;
static int delta = 2;
static char *sp = " ";

/* flag == findlib "predicate" */

#if INTERFACE
struct config_flag {
    char name[32];              /* key */
    char repo[16];
    char package[64];
    char target[32];
    char label[64];
    UT_hash_handle hh;
};
#endif

struct config_flag *the_flag_table; /* FIXME: obsolete? */

UT_array *pos_flags;            /* string */
UT_array *neg_flags;            /* string */

#if INTERFACE
struct config_setting {
    char name[128];              /* key */
    char label[128];
    obzl_meta_flags *flags;
    UT_hash_handle hh;
};
#endif
struct config_setting *the_config_settings;

/* ****************************************************************
   struct obzl_meta_flag_s
   **************************************************************** */
#if INTERFACE
struct obzl_meta_flag {
    bool polarity;
    char *s;
};

struct obzl_meta_flags {
    UT_array *list;
};
#endif

UT_icd flag_icd = {sizeof(struct obzl_meta_flag), NULL, flag_copy, flag_dtor};

int strsort(const void *_a, const void *_b)
{
    const char *a = *(const char* const *)_a;
    const char *b = *(const char* const *)_b;
    /* printf("strsort: %s =? %s\n", a, b); */
    return strcmp(a,b);
}

/* **************************************************************** */
EXPORT int obzl_meta_flags_count(obzl_meta_flags *_flags)
{
    if (_flags == NULL)
        return 0;
    if (_flags->list) {
        int ct = utarray_len(_flags->list);
        return ct;
    } else {
        return 0;
    }
}

EXPORT obzl_meta_flag *obzl_meta_flags_nth(obzl_meta_flags *_flags, int _i)
{
    return utarray_eltptr(_flags->list, _i);
}

/* **************************************************************** */
EXPORT char *obzl_meta_flag_name(obzl_meta_flag *flag)
{
    return flag->s;
}

EXPORT bool obzl_meta_flag_polarity(obzl_meta_flag *flag)
{
    return flag->polarity;
}

void flag_copy(void *_dst, const void *_src) {
#if DEBUG_FLAGS
    log_trace("flag_copy");
#endif
    struct obzl_meta_flag *dst = (struct obzl_meta_flag*)_dst;
    struct obzl_meta_flag *src = (struct obzl_meta_flag*)_src;
    dst->polarity = src->polarity;
    dst->s = src->s ? strdup(src->s) : NULL;
}

void flag_dtor(void *_elt) {
    struct obzl_meta_flag *elt = (struct obzl_meta_flag*)_elt;
    if (elt->s) free(elt->s);
}

/* ****************
   obzl_meta_flags (was: UT_array flags)
   **************** */
obzl_meta_flags *obzl_meta_flags_new(void)
{
#if DEBUG_FLAGS
    log_trace("obzl_meta_flags_new");
#endif
    obzl_meta_flags *new_flags = (obzl_meta_flags*)calloc(sizeof(obzl_meta_flags),1);
    utarray_new(new_flags->list, &flag_icd);
    return new_flags;
}

obzl_meta_flags *obzl_meta_flags_new_copy(obzl_meta_flags *old_flags)
{
#if DEBUG_FLAGS
    /* log_trace("obzl_meta_flags_new_copy %p", old_flags); */
#endif
    if (old_flags == NULL) {
        return NULL;
    }

    /* UT_array *new_flags; */
    obzl_meta_flags *new_flags = (obzl_meta_flags*)calloc(sizeof(obzl_meta_flags),1);
    utarray_new(new_flags->list, &flag_icd);
    struct obzl_meta_flag *old_flag = NULL;
    /* struct obzl_meta_flag *new_flag; */
    while ( (old_flag=(struct obzl_meta_flag*)utarray_next(old_flags->list, old_flag))) {
        /* log_trace("old_flag: %s", old_flag->s); */
        /* new_flag = (struct obzl_meta_flag*)malloc(sizeof(struct obzl_meta_flag)); */
        /* flag_copy(new_flag, old_flag); */
        /* log_trace("new_flag: %s", new_flag->s); */
        utarray_push_back(new_flags->list, old_flag);
    }
    return new_flags;
}

/* EXPORT UT_array *obzl_meta_flags_new_tokenized(char *flags) */
EXPORT obzl_meta_flags *obzl_meta_flags_new_tokenized(char *flags)
{
#if DEBUG_FLAGS
    log_trace("obzl_meta_flags_new_tokenized(%s)", flags);
#endif
    if (flags == NULL) {
        return NULL;
    }
    /* UT_array *new_flags; */
    obzl_meta_flags *new_flags = (obzl_meta_flags*)calloc(sizeof(obzl_meta_flags),1);
    utarray_new(new_flags->list, &flag_icd);
    char *token, *sep = " ,\n";
    token = strtok(flags, sep);
    struct obzl_meta_flag *new_flag;
    while( token != NULL ) {
        /* printf("Flag token: %s", token); */
        new_flag = (struct obzl_meta_flag*)malloc(sizeof(struct obzl_meta_flag));
        new_flag->polarity = token[0] == '-'? false : true;
        new_flag->s = token[0] == '-'? strdup(++token) : strdup(token);
        /* printf("new flag: %s", new_flag->s); */
        utarray_push_back(new_flags->list, new_flag);
        token = strtok(NULL, sep);
    }
    /* dump_flags(8, new_flags); */
    return new_flags;
}

/* void flags_dtor(UT_array *old_flags) { */
void flags_dtor(obzl_meta_flags *old_flags) {
    struct obzl_meta_flag *old_flag = NULL;
    while ( (old_flag=(struct obzl_meta_flag*)utarray_next(old_flags->list, old_flag))) {
        free(old_flag->s);
    }
    utarray_free(old_flags->list);
    free(old_flags);
}

/* is one of the flags deprecated? */
bool obzl_meta_flags_deprecated(obzl_meta_flags *_flags)                     {
    if (_flags) {
        int ct = obzl_meta_flags_count(_flags);
        struct obzl_meta_flag *a_flag = NULL;
        for (int i=0; i < ct; i++) {
            a_flag = obzl_meta_flags_nth(_flags, i);
            if (strncmp(a_flag->s, "mt", 2) == 0) {
                return true;
            }
        }
    }
    return false;
}

bool obzl_meta_flags_has_flag(obzl_meta_flags *_flags, char *_flag, bool polarity)
{
    if (_flags) {
        int ct = obzl_meta_flags_count(_flags);
        struct obzl_meta_flag *a_flag = NULL;
        for (int i=0; i < ct; i++) {
            a_flag = obzl_meta_flags_nth(_flags, i);
            if ( a_flag->polarity == polarity ) {
                if (strncmp(a_flag->s, _flag, 32) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

char *obzl_meta_flags_to_comment(obzl_meta_flags *flags)
{
#ifdef DEBUG_FLAGS
    log_trace("%*obzl_meta_flags_to_string", indent, sp);
#endif
    /* char *buf = (char*)calloc(512, 1); */
    UT_string *buf;
    utstring_new(buf);

    if (flags == NULL) {
        /* printf("%*sflags: none\n", indent, sp); */
        return utstring_body(buf);
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                /* printf("%*sflags ct: 0\n", indent, sp); */
                return utstring_body(buf);
            } else {
                /* log_trace("%*sflags ct: %d", indent, sp, utarray_len(flags->list)); */
                struct obzl_meta_flag *a_flag = NULL;
                while ( (a_flag=(struct obzl_meta_flag*)utarray_next(flags->list, a_flag))) {
                    /* printf("%*s%s (polarity: %d)\n", delta+indent, sp, a_flag->s, a_flag->polarity); */
                    if ( !a_flag->polarity )
                        utstring_printf(buf, "%s", "-");
                    utstring_printf(buf, "%s", a_flag->s); // mystrcat(buf, a_flag->s);
                    utstring_printf(buf, "%s", ", "); // mystrcat(buf, ", ");
                }
                /* printf("buf: %s\n", buf); */
                return utstring_body(buf);
            }
        } else {
            /* printf("%*sflags none: 0\n", indent, sp); */
            /* log_debug("%*sflags: none", indent, sp); */
            return utstring_body(buf);
        }
    }
}

/*
  has flags:
      rc: true:
      _cname arg is set to condition name
  not has fl gs:
      rc: false
  flag ppx_driver is ignored
 */
bool obzl_meta_flags_to_selection_label(obzl_meta_flags *flags,
                                         UT_string *_cname)
{
#ifdef DEBUG_FLAGS
    log_trace("%*sobzl_meta_flags_to_condition_name", indent, sp);
#endif
    /* char *buf = (char*)calloc(512, 1); */
    /* UT_string *buf; */
    /* utstring_new(buf); */
    utstring_clear(_cname);

    struct obzl_meta_flag *a_flag;

    if (flags == NULL) {
        return false;
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                /* printf("%*sflags ct: 0\n", indent, sp); */
                return false;
            } else {
                if (utarray_len(flags->list) == 1) {
                    a_flag = obzl_meta_flags_nth(flags, 0);
                    if (strncmp(a_flag->s, "byte", 4) == 0) {
                        utstring_printf(_cname, "@rules_ocaml//build/mode:bytecode");
                        return true; // _cname;
                    }
                    if (strncmp(a_flag->s, "native", 6) == 0) {
                        utstring_printf(_cname, "@rules_ocaml//build/mode:native");
                        return true; // _cname;
                    }
                    if (strncmp(a_flag->s, "ppx_driver", 10) == 0) {
                        /* we do not treat 'ppx_driver' as a flag?? */
                        /* for no_ppx_driver target -ppx_driver is default */
                        if ( !a_flag->polarity ) /* '-' prefix */
                            utstring_printf(_cname, "//conditions:default");
                     /* utstring_printf(_cname, ":ppx_driver_disabled"); */
                        else
                            utstring_printf(_cname, ":ppx_driver_enabled");
                        /* utstring_printf(_cname, "//conditions:default"); */
                        /* utstring_printf(_cname, ":custom_ppx_enabled"); */
                        return true;
                    }

                    /* ignore thread-related flags */
                    /* this handles mt, mt_posix, mt_vm; we assume no
                       other flags start with "mt" */
                    if (strncmp(a_flag->s, "mt", 2) == 0) return false;

                    /* FIXME: rename? @rules_ocaml//cfg/cfg is cli build settings */
                    /* for selecting plugin, mode */
                    /* utstring_printf(_cname, "%s", "@rules_ocaml//cfg/cfg:"); */
                    /* if ( !a_flag->polarity ) /\* '-' prefix *\/ */
                    /*     utstring_printf(_cname, "no_"); */
                    /* utstring_printf(_cname, "%s", a_flag->s); */

                    return true; // _cname;
                } else {
                    /* compound condition */
                    int ct = utarray_len(flags->list); // obzl_meta_flags_count(flags);
                    log_trace("%*sflags ct: %d", indent, sp, ct);
                    /* char config_name[128]; */
                    /* config_name[0] = '\0'; */
                    UT_string *config_name;
                    utstring_new(config_name);
                    struct obzl_meta_flag *a_flag = NULL;
                    int saw_ppx_driver = 0;
                    for (int i=0; i < ct; i++) {
                        a_flag = obzl_meta_flags_nth(flags, i);
                        if (strncmp(a_flag->s, "ppx_driver", 10) == 0) {
                            /* we do not treat 'ppx_driver' as a flag */
                            saw_ppx_driver++;
                            continue;
                        }
                        if (strncmp(a_flag->s, "byte", 4) == 0) {
                            if (saw_ppx_driver>0) {
                                utstring_printf(_cname, "@rules_ocaml//build/mode:bytecode");
                                return true;
                            }
                        }
                        if (strncmp(a_flag->s, "native", 6) == 0) {
                            if (saw_ppx_driver>0) {
                                utstring_printf(_cname, "@rules_ocaml//build/mode:native");
                                return true;
                            }
                        }
                        if (strncmp(a_flag->s, "custom_ppx", 10) == 0) {
                            if ( !a_flag->polarity ) {
                                utstring_printf(_cname, "custom_ppx_disabled");
                                /* utstring_printf(_cname, "//conditions:default"); */
                                return true;
                            } else {
                                /* empirically, only '-custom_ppx' ever occurs */
                                log_error("Unexpected positive flag 'custom_ppx'");
                            }
                        }

                        /* ignore thread-related flags */
                        /* this handles mt, mt_posix, mt_vm; we assume no
                           other flags start with "mt" */
                        if (strncmp(a_flag->s, "mt", 2) == 0) return false;

                        if (i - saw_ppx_driver > 0) utstring_printf(config_name, "%s", "_"); // mystrcat(config_name, "_");
                        log_debug("%*s%s (polarity: %d)", delta+indent, sp, a_flag->s, a_flag->polarity);
                        if ( !a_flag->polarity ) /* '-' prefix */
                            if (saw_ppx_driver == 0)
                                utstring_printf(config_name, "%s", "no_"); // mystrcat(config_name, "no_");
                        utstring_printf(config_name, "%s", a_flag->s); // mystrcat(config_name, a_flag->s);
                    }

                    /* some packages use plugin and toploop flags in
                       the archive property. we treat these as
                       targets, not selectables */
                    if (strncmp(a_flag->s, "plugin", 6) == 0) return false;
                    if (strncmp(a_flag->s, "toploop", 8) == 0) return false;


                    /* register compound flags, so we can generate config_setting rules */
                    /* struct config_setting *a_condition; */
                    /* utstring_printf(_cname, "@rules_ocaml//cfg/cfg:%s", config_name); */
                    /* HASH_FIND_STR(the_config_settings, config_name, a_condition);  /\* already in the hash? *\/ */
                    /* if (a_condition == NULL) { */
                    /*     a_condition = calloc(sizeof(struct config_setting), 1); */
                    /*     strncpy(a_condition->name, config_name, 128); */
                    /*     strncpy(a_condition->label, utstring_body(_cname), 128); */
                    /*     a_condition->flags = flags; */
                    /*     HASH_ADD_STR(the_config_settings, name, a_condition); */
                    /* } */
                    /* printf("_cname: %s\n", _cname); */
                    return true; // _cname;
                }
            }
        } else {
            /* printf("%*sflags none: 0\n", indent, sp); */
            /* log_debug("%*sflags: none", indent, sp); */
            return false; // _cname;
        }
    }
}

bool obzl_meta_flags_to_cmtag(obzl_meta_flags *flags,
                              UT_string *_cname)
{
#ifdef DEBUG_FLAGS
    log_trace("%*sobzl_meta_flags_to_cmtag", indent, sp);
#endif
    /* char *buf = (char*)calloc(512, 1); */
    /* UT_string *buf; */
    /* utstring_new(buf); */
    utstring_clear(_cname);

    struct obzl_meta_flag *a_flag;

    if (flags == NULL) {
        return false;
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                /* printf("%*sflags ct: 0\n", indent, sp); */
                return false;
            } else {
                if (utarray_len(flags->list) == 1) {
                    a_flag = obzl_meta_flags_nth(flags, 0);
                    if (strncmp(a_flag->s, "byte", 4) == 0) {
                        utstring_printf(_cname, "cma");
                        return true; // _cname;
                    }
                    if (strncmp(a_flag->s, "native", 6) == 0) {
                        utstring_printf(_cname, "cmxa");
                        return true; // _cname;
                    }
                    if (strncmp(a_flag->s, "ppx_driver", 10) == 0) {
                        /* we do not treat 'ppx_driver' as a flag?? */
                        /* for no_ppx_driver target -ppx_driver is default */
                        if ( !a_flag->polarity ) /* '-' prefix */
                            utstring_printf(_cname, "//conditions:default");
                     /* utstring_printf(_cname, ":ppx_driver_disabled"); */
                        else
                            utstring_printf(_cname, ":ppx_driver_enabled");
                        /* utstring_printf(_cname, "//conditions:default"); */
                        /* utstring_printf(_cname, ":custom_ppx_enabled"); */
                        return true;
                    }

                    /* ignore thread-related flags */
                    /* this handles mt, mt_posix, mt_vm; we assume no
                       other flags start with "mt" */
                    if (strncmp(a_flag->s, "mt", 2) == 0) return false;

                    /* FIXME: rename? @rules_ocaml//cfg/cfg is cli build settings */
                    /* for selecting plugin, mode */
                    /* utstring_printf(_cname, "%s", "@rules_ocaml//cfg/cfg:"); */
                    /* if ( !a_flag->polarity ) /\* '-' prefix *\/ */
                    /*     utstring_printf(_cname, "no_"); */
                    /* utstring_printf(_cname, "%s", a_flag->s); */

                    return true; // _cname;
                } else {
                    /* compound condition */
                    int ct = utarray_len(flags->list); // obzl_meta_flags_count(flags);
                    log_trace("%*sflags ct: %d", indent, sp, ct);
                    /* char config_name[128]; */
                    /* config_name[0] = '\0'; */
                    UT_string *config_name;
                    utstring_new(config_name);
                    struct obzl_meta_flag *a_flag = NULL;
                    int saw_ppx_driver = 0;
                    for (int i=0; i < ct; i++) {
                        a_flag = obzl_meta_flags_nth(flags, i);
                        if (strncmp(a_flag->s, "ppx_driver", 10) == 0) {
                            /* we do not treat 'ppx_driver' as a flag */
                            saw_ppx_driver++;
                            continue;
                        }
                        if (strncmp(a_flag->s, "byte", 4) == 0) {
                            if (saw_ppx_driver>0) {
                                utstring_printf(_cname, "cma");
                                return true;
                            }
                        }
                        if (strncmp(a_flag->s, "native", 6) == 0) {
                            if (saw_ppx_driver>0) {
                                utstring_printf(_cname, "cmxa");
                                return true;
                            }
                        }
                        if (strncmp(a_flag->s, "custom_ppx", 10) == 0) {
                            if ( !a_flag->polarity ) {
                                utstring_printf(_cname, "custom_ppx_disabled");
                                /* utstring_printf(_cname, "//conditions:default"); */
                                return true;
                            } else {
                                /* empirically, only '-custom_ppx' ever occurs */
                                log_error("Unexpected positive flag 'custom_ppx'");
                            }
                        }

                        /* ignore thread-related flags */
                        /* this handles mt, mt_posix, mt_vm; we assume no
                           other flags start with "mt" */
                        if (strncmp(a_flag->s, "mt", 2) == 0) return false;

                        if (i - saw_ppx_driver > 0) utstring_printf(config_name, "%s", "_"); // mystrcat(config_name, "_");
                        log_debug("%*s%s (polarity: %d)", delta+indent, sp, a_flag->s, a_flag->polarity);
                        if ( !a_flag->polarity ) /* '-' prefix */
                            if (saw_ppx_driver == 0)
                                utstring_printf(config_name, "%s", "no_"); // mystrcat(config_name, "no_");
                        utstring_printf(config_name, "%s", a_flag->s); // mystrcat(config_name, a_flag->s);
                    }

                    /* some packages use plugin and toploop flags in
                       the archive property. we treat these as
                       targets, not selectables */
                    if (strncmp(a_flag->s, "plugin", 6) == 0) return false;
                    if (strncmp(a_flag->s, "toploop", 8) == 0) return false;


                    /* register compound flags, so we can generate config_setting rules */
                    /* struct config_setting *a_condition; */
                    /* utstring_printf(_cname, "@rules_ocaml//cfg/cfg:%s", config_name); */
                    /* HASH_FIND_STR(the_config_settings, config_name, a_condition);  /\* already in the hash? *\/ */
                    /* if (a_condition == NULL) { */
                    /*     a_condition = calloc(sizeof(struct config_setting), 1); */
                    /*     strncpy(a_condition->name, config_name, 128); */
                    /*     strncpy(a_condition->label, utstring_body(_cname), 128); */
                    /*     a_condition->flags = flags; */
                    /*     HASH_ADD_STR(the_config_settings, name, a_condition); */
                    /* } */
                    /* printf("_cname: %s\n", _cname); */
                    return true; // _cname;
                }
            }
        } else {
            /* printf("%*sflags none: 0\n", indent, sp); */
            /* log_debug("%*sflags: none", indent, sp); */
            return false; // _cname;
        }
    }
}

/* **************************************************************** */
EXPORT void initialize_config_flags()
{
    utarray_new(pos_flags, &ut_str_icd);
    utarray_new(neg_flags, &ut_str_icd);

    /* char name[32];              /\* key *\/ */
    /* char repo[16]; */
    /* char package[64]; */
    /* char target[32]; */
    /* char label[64]; */

    struct config_flag *a_flag;

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "byte", 4);
    strncpy(a_flag->repo, "@ocaml", 6);
    strncpy(a_flag->package, "mode", 4);
    strncpy(a_flag->target, "bytecode", 8);
    strncpy(a_flag->label, "@rules_ocaml//build/mode:bytecode", 21);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "native", 6);
    strncpy(a_flag->repo, "@ocaml", 6);
    strncpy(a_flag->package, "mode", 4);
    strncpy(a_flag->target, "native", 6);
    strncpy(a_flag->label, "@rules_ocaml//build/mode:native", 19);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "mt", 2);
    strncpy(a_flag->repo, "@ocaml", 5);
    strncpy(a_flag->package, "cfg/mt", 6);
    strncpy(a_flag->target, "default", 7);
    strncpy(a_flag->label, "@rules_ocaml//cfg/cfg/mt:default", 21);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "mt_posix", 8);
    strncpy(a_flag->repo, "@ocaml", 5);
    strncpy(a_flag->package, "cfg/mt", 6);
    strncpy(a_flag->target, "posix", 5);
    strncpy(a_flag->label, "@rules_ocaml//cfg/cfg/mt:posix", 19);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "mt_vm", 5);
    strncpy(a_flag->repo, "@ocaml", 5);
    strncpy(a_flag->package, "cfg/mt", 6);
    strncpy(a_flag->target, "vm", 2);
    strncpy(a_flag->label, "@rules_ocaml//cfg/cfg/mt:vm", 16);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "gprof", 5);
    strncpy(a_flag->repo, "@ocaml", 5);
    strncpy(a_flag->package, "cfg", 3);
    strncpy(a_flag->target, "gprof", 5);
    strncpy(a_flag->label, "@rules_ocaml//cfg/cfg:gprof", 16);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "ppx_driver", 10);
    strncpy(a_flag->repo, "@ocaml", 5);
    strncpy(a_flag->package, "cfg", 3);
    strncpy(a_flag->target, "driver", 6);
    strncpy(a_flag->label, "@rules_ocaml//cfg/cfg:ppx_driver", 21);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "custom_ppx", 10);
    strncpy(a_flag->repo, "@ocaml", 5);
    strncpy(a_flag->package, "cfg", 3);
    strncpy(a_flag->target, "custom", 6);
    strncpy(a_flag->label, "@rules_ocaml//cfg/cfg:ppx_custom", 21);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    /* these seem to be associated with camlp4; ignore for now: */
    /* toploop, create_toploop, preprocessor, syntax */
}

void register_flags(obzl_meta_flags *_flags)
{
    char **p;
    for (int i=0; i < obzl_meta_flags_count(_flags); i++) {
        obzl_meta_flag *flag = obzl_meta_flags_nth(_flags, i);
        char *flag_s = flag->s;
        if ( !strncmp(flag_s, "byte", 4) ) continue;
        if ( !strncmp(flag_s, "native", 6) ) continue;

        log_debug("registering flag: %s (%d)", flag_s, flag->polarity);

        utarray_sort(pos_flags,strsort);
        p = NULL;
        if (flag->polarity) { /* pos */
            log_debug("registering pos flag %s", flag->s);
            p = utarray_find(pos_flags, &flag_s, strsort);
            if ( p == NULL ) {
                log_debug("%s not found in pos_flags table; pushing.", flag_s);
                utarray_push_back(pos_flags, &flag_s);
            } else {
                log_debug("found %s in pos_flags table", flag_s);
            }
            continue;
        }
        /* else neg: */
        utarray_sort(neg_flags,strsort);
        log_debug("registering neg flag %s", flag->s);
        p = utarray_find(neg_flags, &flag_s, strsort);
        if ( p == NULL ) {
            log_debug("%s not found in neg_flags table; pushing.", flag_s);
            utarray_push_back(neg_flags, &flag_s);
        } else {
            log_debug("found %s in neg_flags table", flag_s);
        }
    }
}

EXPORT void dispose_flag_table(void)
{
    /* log_debug("predefined flags:"); */
    struct config_flag *s, *tmp;
    HASH_ITER(hh, the_flag_table, s, tmp) {
        /* log_debug("\t%s", s->label); */
        HASH_DEL(the_flag_table, s);
        free(s);
    }

    /* log_debug("accumulated config settings:"); */
    struct config_setting *cd, *ctmp;
    HASH_ITER(hh, the_config_settings, cd, ctmp) {
        /* log_debug("\t%s: %s", cd->name, cd->label); */
        HASH_DEL(the_config_settings, cd);
        free(cd);
    }
}

/* void register_condition_name(char *_name, obzl_meta_flags *_flags) */
/* { */
/*     struct config_setting *a_condition; */
/*     a_condition = calloc(sizeof(struct config_setting), 1); */
/*     strncpy(a_condition->name, _name, 128); */
/*     a_condition->flags = _flags; */
/*     HASH_ADD_STR(the_config_settings, name, a_condition); */
/* } */

/* **************************************************************** */
#if DEBUG_DUMP
void dump_flags(int indent, obzl_meta_flags *flags)
{
    indent++;            /* account for width of log label */
    /* log_trace("%*sdump_flags", indent, sp); */
    if (flags == NULL) {
        log_trace("%*sflags: none", indent, sp);
        return;
    } else {
        if ( flags->list ) {
            if (utarray_len(flags->list) == 0) {
                log_trace("%*sflags: none.", indent, sp);
                return;
            } else {
                log_trace("%*sflags ct: %d", indent, sp, utarray_len(flags->list));
                struct obzl_meta_flag *a_flag = NULL;
                while ( (a_flag=(struct obzl_meta_flag*)utarray_next(flags->list, a_flag))) {
                    log_debug("%*s%s (polarity: %d)", delta+indent, sp, a_flag->s, a_flag->polarity);
                }
            }
        } else {
            log_debug("%*sflags: none", indent, sp);
        }
    }
}
#endif
