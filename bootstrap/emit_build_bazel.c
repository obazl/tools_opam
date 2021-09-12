#if EXPORT_INTERFACE
#include <stdio.h>
#endif

#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>
#endif


#include "log.h"
#include "utarray.h"
#include "utstring.h"

#include "emit_build_bazel.h"

/* **************************************************************** */
static int level = 0;
static int spfactor = 4;
static char *sp = " ";

static int indent = 2;
static int delta = 2;

bool stdlib_root = false;

UT_string *build_bazel_file = NULL;

void emit_bazel_hdr(FILE* ostream, int level, char *repo, char *pkg_prefix, obzl_meta_package *_pkg)
{
    fprintf(ostream, "load(\n");
    fprintf(ostream, "%*s\"@obazl_rules_ocaml//ocaml:rules.bzl\",\n", 4, sp);
    fprintf(ostream, "%*s\"ocaml_import\"\n", 4, sp);
    fprintf(ostream, ")\n");
}

obzl_meta_values *resolve_setting_values(obzl_meta_setting *_setting,
                                         obzl_meta_flags *_flags,
                                         obzl_meta_settings *_settings)
{
    log_debug("resolve_setting_values, opcode: %d", _setting->opcode);
    obzl_meta_values * vals = obzl_meta_setting_values(_setting);
    /* log_debug("vals ct: %d", obzl_meta_values_count(vals)); */
    if (_setting->opcode == OP_SET)
        return vals;

    /* else OP_UPDATE */

    UT_array *resolved_values;
    utarray_new(resolved_values, &ut_str_icd);
    utarray_concat(resolved_values, vals->list);

    /* for each flag, search settings for matching flag */
    int settings_ct = obzl_meta_settings_count(_settings);
    struct obzl_meta_setting *a_setting;

    int flags_ct    = obzl_meta_flags_count(_flags);
    /* printf("\tflags_ct: %d\n", flags_ct); */
    struct obzl_meta_flag *a_flag = NULL;

    for (int i=0; i < flags_ct; i++) {
        a_flag = obzl_meta_flags_nth(_flags, i);
        for (int j=0; j < settings_ct; j++) {
            a_setting = obzl_meta_settings_nth(_settings, j);
            if (a_setting == _setting) continue; /* don't match self */

            obzl_meta_flags *setting_flags = obzl_meta_setting_flags(a_setting);

            if (setting_flags == NULL) {
                /* always match no flags, e.g. 'requires = "findlib.internal"' */
                obzl_meta_values *vs = obzl_meta_setting_values(a_setting);
                /* printf("xconcatenating\n"); fflush(stdout); fflush(stderr); */
                utarray_concat(resolved_values, vs->list);
                continue;
            }

            int ct = obzl_meta_flags_count(setting_flags);
            if (ct > 1) {
                /* only try to match singletons? */
                continue;
            }

            obzl_meta_flag *setting_flag = obzl_meta_flags_nth(setting_flags, 0);
            if (setting_flag->polarity == a_flag->polarity) {
                if (strncmp(setting_flag->s, a_flag->s, 32) == 0) {
                    log_debug("matched flag");
                    /* we have found a setting with exactly one flag, that matches the search flag */
                    /* now we check the setting's opcode - it should always be SET? */
                    /* then add its values list */
                    obzl_meta_values *vs = obzl_meta_setting_values(a_setting);
                    utarray_concat(resolved_values, vs->list);
                    /* dump_setting(4, a_setting); */
                }
            }
        }
    }
    obzl_meta_values *new_values = (obzl_meta_values*)calloc(sizeof(obzl_meta_values),1);
    new_values->list = resolved_values;
    return new_values;
}

void emit_bazel_attribute(FILE* ostream,
                          int level, /* indentation control */
                          char *_repo,
                          char *_pkg_path,
                          char *_pkg_prefix,
                          char *_pkg_name,
                          /* for constructing import label: */
                          char *_imports_path, /* _lib */
                          /* char *_subpkg_dir, */
                          obzl_meta_entries *_entries,
                          char *property) /* = 'archive' or 'plugin' */
{
    log_debug("EMIT_BAZEL_ATTRIBUTE _pkg_name: '%s'; prop: '%s'; imports path: '%s'",
              _pkg_name, property, _imports_path);
    log_debug("    _pkg_path: %s", _pkg_path);
    /*
      NOTE: archive/plugin location is relative to the 'directory' property!
      which may be empty, even for subpackages (e.g. opam-lib subpackages)
     */

    char *directory = NULL;
    bool stdlib = false;
    struct obzl_meta_property *directory_prop = obzl_meta_entries_property(_entries, "directory");

    if ( directory_prop == NULL ) {
        log_warn("Prop 'directory' not found.");
    } else {
        directory = (char*)obzl_meta_property_value(directory_prop);
        log_info("Prop 'directory': %s", directory);
        if (directory == NULL) {
            ; /* from directory = "" */
        } else {
            if ( strncmp(directory, "+", 1) == 0 ) {
                stdlib = true;
                log_debug("Found STDLIB directory '%s' for %s", directory, _pkg_name);
                directory++;
            } else {
                if ( (strlen(directory) == 1)
                     && (strncmp(directory, "^", 1) == 0) ) {
                    stdlib = true;
                    stdlib_root = true;
                    directory[0] = '\0';
                    mystrcat(directory, "ocaml");
                    log_debug("Found STDLIB root directory '%s' for %s", directory, _pkg_name);
                }
            }
        }
    }
    /* log_debug("DIRECTORY: %s", directory); */
    /* log_debug(" PROP: %s", property); */

    struct obzl_meta_property *deps_prop = obzl_meta_entries_property(_entries, property);
    if ( deps_prop == NULL ) {
        log_warn("Prop '%s' not found: %s.", property); //, pkg_name);
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);

    int settings_ct = obzl_meta_settings_count(settings);
    log_info("settings count: %d", settings_ct);
    if (settings_ct == 0) {
        log_info("No settings for %s", obzl_meta_property_name(deps_prop));
        return;
    }

    /* ppx_sexp_value
       requires(ppx_driver), requires(-ppx_driver), and
       ppx(-ppx_driver,-custom_ppx)
     */

    /* dealing with OP_UDATE.
       e.g. ppx_sexp_conv has three settings:
       requires(ppx_driver) = "ppx_sexp_conv.expander ppxlib"
       requires(-ppx_driver) = "ppx_sexp_conv.runtime-lib"
       ppx(-ppx_driver,-custom_ppx) += "ppx_deriving"

       the 3rd must combine the 2nd because the op on the last is '+='
       condition: no_ppx_driver: ppx_sexp_conv.runtime-lib
       condition: no_ppx_driver_no_custom_ppx: ppx_sexp_conv.runtime-lib, ppx_deriving
       if op == UPDATE then for each flag, search flaglist for match and add vals if found
     */

    /* if (g_ppx_pkg) { */
    /*     ; */
    /* } else { */
    /*     ; */
    /* } */

    /* NB: 'property' will be 'archive' or 'plugin' */
    if (settings_ct > 1) {
        fprintf(ostream, "%*s%s = select({\n", level*spfactor, sp, property);
    } else {
        fprintf(ostream, "%*s%s = [\n", level*spfactor, sp, property);
    }

    UT_string *condition_name;
    utstring_new(condition_name);

    obzl_meta_setting *setting = NULL;

    obzl_meta_values *vals;
    obzl_meta_value *v = NULL;

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        log_debug("setting %d", i+1);

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);

        /* skip any setting with deprecated flag, e.g. 'vm' */
        if (obzl_meta_flags_deprecated(flags)) {
            /* log_debug("TTTTTTTTTTTTTTTT DEPRECATED FLAG"); */
            continue;
        }

        if (flags != NULL) register_flags(flags);

        /* if (g_ppx_pkg) {        /\* set by opam_bootstrap.handle_lib_meta *\/ */
        /* } */

        bool has_conditions;
        if (flags == NULL)
            utstring_printf(condition_name, "//conditions:default");
        else
            has_conditions = obzl_meta_flags_to_select_condition(flags, condition_name);

        /* char *condition_comment = obzl_meta_flags_to_comment(flags); */
        /* log_debug("condition_comment: %s", condition_comment); */

        /* FIXME: multiple settings means multiple flags; decide how to handle for deps */
        // construct a select expression on the flags
        if (settings_ct > 1) {
            fprintf(ostream, "        \"%s\"%-4s",
                    utstring_body(condition_name), ": [\n");
            /* condition_comment, /\* FIXME: name the condition *\/ */
        }

        /* now we handle UPDATE settings */
        vals = resolve_setting_values(setting, flags, settings);

        /* now we construct a bazel label for each value */
        UT_string *label;
        utstring_new(label);

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            v = obzl_meta_values_nth(vals, j);
            /* log_info("property val: '%s'", *v); */

            /* char *s = (char*)*v; */
            /* while (*s) { */
            /*     /\* printf("s: %s\n", s); *\/ */
            /*     if(s[0] == '.') { */
            /*         /\* log_info("Hit"); *\/ */
            /*         s[0] = '/'; */
            /*     } */
            /*     s++; */
            /* } */
            utstring_clear(label);
            /* utstring_printf(label, "//:%s", _imports_path); /\* e.g. _lib *\/ */
            log_debug("21 _pkg_path: %s", _pkg_path);
            log_debug("21 _pkg_prefix: %s", _pkg_prefix);
            log_debug("21 _imports_path: %s", _imports_path);
            /* SPECIAL CASE: compiler-libs FIXME: handle this better */
            if (strncmp(_pkg_path, "compiler-libs", 13) == 0) {
                utstring_printf(label, "/%s", "ocaml/compiler-libs");
            } else {
                if (stdlib) {
                    utstring_printf(label,
                                    "//:_lib/ocaml");
                                    /* _imports_path); */
                    /* if (stdlib_root) { */
                    /*     if (strlen(_pkg_path)>0) */
                    /*         utstring_printf(label, "/%s", _pkg_path); */
                    /* } else { */
                    /*     utstring_printf(label, "/%s", directory); */
                    /* } */
                } else {
                    if (directory_prop == NULL) {
                        if (strlen(_pkg_path)>0) {
                            utstring_printf(label,
                                            "//:%s",
                                            _imports_path); //, _pkg_name);
                        }
                    } else {
                        log_debug("new import path seg: %s", _pkg_name);
                        utstring_printf(label, "//:%s/%s",
                                        _imports_path, _pkg_name);
                    }

                    /* log_debug("has dir: %p", directory_prop); */
                    /* if (directory_prop) */
                    /*     utstring_printf(label, "/%s", _pkg_name); */
                    /* else */
                    /*     /\* no _pkg_path means top-level pkg, so add pkg subdir *\/ */
                    /*     if (strlen(_pkg_path) == 0) */
                    /*         utstring_printf(label, "/%s", _pkg_name); */
                }
            }
            utstring_printf(label, "/%s", *v);
            log_debug("label 2: %s", utstring_body(label));

            if (settings_ct > 1) {
                    fprintf(ostream, "            \"%s\",\n", utstring_body(label));
            } else {
                    fprintf(ostream, "\"%s\",\n", utstring_body(label));
            }
            /* if v is .cmxa, then add .a too? */
            int cmxapos = utstring_findR(label, -1, "cmxa", 4);
            /* if (cmxapos > 0) { */
            /*     char *cmxa_a = utstring_body(label); */
            /*     cmxa_a[utstring_len(label) - 4] = 'a'; */
            /*     cmxa_a[utstring_len(label) - 3] = '\0'; */
            /*     if (has_conditions) /\* skip mt, mt_vm, mt_posix *\/ */
            /*         fprintf(ostream, "            \"%s\",\n", cmxa_a); */
            /* } */
            /* if (flags != NULL) /\* skip mt, mt_vm, mt_posix *\/ */
            /*     fprintf(ostream, "%s", "        ],\n"); */
        }
        utstring_free(label);
        if (settings_ct > 1) {
            fprintf(ostream, "\n"); // , (1+level)*spfactor, sp);
            fprintf(ostream, "%*s],\n", (1+level)*spfactor, sp);
        }
        /* free(condition_comment); */
    }
    utstring_free(condition_name);
    if (settings_ct > 1)
        fprintf(ostream, "%*s}),\n", level*spfactor, sp);
    else
        fprintf(ostream, "%*s],\n", level*spfactor, sp);
}

/*
  FIXME: version, description really apply to whole pkg; put them in comments, not rule attributes
 */
void emit_bazel_metadatum(FILE* ostream, int level,
                          char *repo, char *pkg,
                          obzl_meta_entries *_entries,
                          char *_property, char *_attrib)
{
    /* emit version, description properties */

    struct obzl_meta_property *the_prop = obzl_meta_entries_property(_entries, _property);
    if ( the_prop == NULL ) {
        /* char *pkg_name = obzl_meta_package_name(_pkg); */
        /* log_warn("Prop '%s' not found: %s.", _property); //, pkg_name); */
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(the_prop);
    obzl_meta_setting *setting = NULL;

    if (obzl_meta_settings_count(settings) == 0) {
        /* log_info("No settings for property '%s'", _property); */
        return;
    }
    int settings_ct = obzl_meta_settings_count(settings);
    if (settings_ct > 1) {
        log_warn("settings count > 1 for property '%s'; using the first.", settings_ct, _property);
    }

    obzl_meta_values *vals;
    obzl_meta_value *v = NULL;

    setting = obzl_meta_settings_nth(settings, 0);

    /* should not be any flags??? */

    vals = obzl_meta_setting_values(setting);
    /* dump_values(0, vals); */
    log_debug("metadatum %s setting vals ct: %d", _property, obzl_meta_values_count(vals));
    /* if vals ct > 1 warn, should be only 1 */
    v = obzl_meta_values_nth(vals, 0);
    /* log_debug("vals[0]: %s", *v); */
    if (v != NULL)
        /* fprintf(stderr, "    %s = \"%s\",\n", _attrib, *v); */
        fprintf(ostream, "    %s = \"\"\"%s\"\"\",\n", _attrib, *v);
}

/* void emit_bazel_ppx_dummy_rule(FILE* ostream, int level, */
/*                                 char *_repo, */
/*                                 char *_imports_path, /\* opam symlinked input dir, e.g. '_lib' *\/ */
/*                                 char *_pkg_path, */
/*                                 char *_pkg_name, */
/*                                 obzl_meta_entries *_entries) */
/*                                 /\* obzl_meta_package *_pkg) *\/ */
/* { */
/*     /\* fprintf(ostream, "\nocaml_import(\n"); *\/ */
/*     /\* fprintf(ostream, "    name = \"no_ppx_driver\",\n"); // , _pkg_name); *\/ */
/*     /\* emit_bazel_metadatum(ostream, 1, "@opam", _imports_path, _entries, "version", "version"); *\/ */
/*     /\* emit_bazel_metadatum(ostream, 1, "@opam", _imports_path, _entries, "description", "doc"); *\/ */
/*     /\* we won't have 'archive' or 'plugin' entries for -ppx_driver? *\/ */
/*     /\* emit_bazel_attribute(ostream, 1, "@opam", _imports_path, _pkg_path, _pkg_name, _entries, "archive"); *\/ */
/*     /\* for deps we process 'requires(-ppx_driver...)' entries *\/ */
/*     /\* emit_bazel_ppx_dummy_deps(ostream, 1, "@opam", "lib", _entries); *\/ */
/*     /\* fprintf(ostream, "    visibility = [\"//visibility:public\"]\n"); *\/ */
/*     /\* fprintf(ostream, ")\n"); *\/ */

/*     fprintf(ostream, "%s", */
/*             "load(\"@bazel_skylib//rules:common_settings.bzl\", \"bool_flag\")\n" */
/*             "\n" */
/*             "bool_flag( name = \"ppx_driver\", build_setting_default = True )\n" */
/*             "bool_flag( name = \"custom_ppx\", build_setting_default = True )\n" */
/*             "\n"); */

/*     fprintf(ostream, "%s", */
/*             "config_setting(\n" */
/*             "    name = \"ppx_driver_only\",\n" */
/*             "    flag_values = { \":ppx_driver\": \"True\", \":custom_ppx\": \"True\" }\n" */
/*             ")\n\n" */
/*             "config_setting(\n" */
/*             "    name = \"no_ppx_driver_only\",\n" */
/*             "    flag_values = { \":ppx_driver\": \"False\", \":custom_ppx\": \"True\" }\n" */
/*             ")\n\n" */
/*             "config_setting(\n" */
/*             "    name = \"both_disabled\",\n" */
/*             "    flag_values = { \":ppx_driver\": \"False\", \":custom_ppx\": \"False\" }\n" */
/*             ")\n\n" */
/*             ); */
/* } */

void emit_bazel_archive_rule(FILE* ostream,
                             int level,
                             char *_repo,
                             char *_imports_path, /* _lib/... */
                             char *_pkg_path,
                             char *_pkg_prefix,
                             char *_pkg_name,
                             obzl_meta_entries *_entries)
                             /* char *_subpkg_dir) */
                                /* obzl_meta_package *_pkg) */
{
    log_debug("EMIT_BAZEL_ARCHIVE_RULE: _imports_path: %s", _imports_path);
    /* http://projects.camlcity.org/projects/dl/findlib-1.9.1/doc/ref-html/r759.html

The variable "archive" specifies the list of archive files. These files should be given either as (1) plain names without any directory information; they are only searched in the package directory. (2) Or they have the form "+path" in which case the files are looked up relative to the standard library. (3) Or they have the form "@name/file" in which case the files are looked up in the package directory of another package. (4) Or they are given as absolute paths.

The names of the files must be separated by white space and/or commas. In the preprocessor stage, the archive files are passed as extensions to the preprocessor (camlp4) call. In the linker stage (-linkpkg), the archive files are linked. In the compiler stage, the archive files are ignored.

Note that "archive" should only be used for archive files that are intended to be included in executables or loaded into toploops. For modules loaded at runtime there is the separate variable "plugin".
     */

    /* archive flags: byte, native */
    /* pkg threads: mt, mt_vm, mt_posix */
    /* ppx: ppx_driver, -ppx_driver always go together. not a syntactic rule, but seems to be the convention */

    fprintf(ostream, "\nocaml_import(\n");
    fprintf(ostream, "    name = \"%s\",\n", _pkg_name); /* default target provides archive */
    emit_bazel_metadatum(ostream, 1,
                         _repo,              /* @opam */
                         _imports_path, _entries, "version", "version");
    emit_bazel_metadatum(ostream, 1,
                         _repo, // "@opam",
                         _imports_path, _entries, "description", "doc");
    emit_bazel_attribute(ostream, 1,
                         _repo, // "@opam",
                         _pkg_path,
                         _pkg_prefix,
                         _pkg_name,
                         /* for constructing import label: */
                         _imports_path,
                         /* _subpkg_dir, */
                         _entries,
                         "archive");
    emit_bazel_deps(ostream, 1, "@opam", "lib", _entries);
    emit_bazel_deps_adjunct(ostream, 1, "@opam", "lib", _entries);
    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
}

void emit_bazel_plugin_rule(FILE* ostream, int level,
                            char *_repo,
                            char *_imports_path,
                            char *_pkg_path,
                            char *_pkg_prefix,
                            char *_pkg_name,
                            obzl_meta_entries *_entries,
                            char *_subpkg_dir)
{
    log_debug("EMIT_BAZEL_PLUGIN_RULE: _imports_path: %s", _imports_path);
    fprintf(ostream, "\nocaml_import(\n");
    fprintf(ostream, "    name = \"plugin\",\n");
    emit_bazel_attribute(ostream, 1,
                         _repo,
                         _pkg_path,
                         _pkg_prefix,
                         _pkg_name,
                         _imports_path,
                         /* _subpkg_dir, */
                         _entries,
                         "plugin");
    emit_bazel_deps(ostream, 1, "@opam", "lib", _entries);
    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
}

void emit_bazel_deps(FILE* ostream, int level, char *repo, char *pkg,
                               obzl_meta_entries *_entries)
                     /* obzl_meta_package *_pkg) */
{

    //FIXME: skip if no 'requires'
    /* obzl_meta_entries *entries = obzl_meta_package_entries(_pkg); */

    char *pname = "requires";
    /* struct obzl_meta_property *deps_prop = obzl_meta_package_property(_pkg, pname); */
    struct obzl_meta_property *deps_prop = obzl_meta_entries_property(_entries, pname);
    if ( deps_prop == NULL ) {
        /* char *pkg_name = obzl_meta_package_name(_pkg); */
        /* log_warn("Prop '%s' not found: %s.", pname, pkg_name); */
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    int settings_no_ppx_driver_ct = obzl_meta_settings_flag_count(settings, "ppx_driver", false);

    settings_ct -= settings_no_ppx_driver_ct;

    /* log_info("settings count: %d", settings_ct); */

    if (settings_ct == 0) {
        log_info("No deps for %s", obzl_meta_property_name(deps_prop));
        return;
    }

    obzl_meta_values *vals;
    obzl_meta_value *v = NULL;

    if (settings_ct > 1) {
        fprintf(ostream, "%*sdeps = select({\n", level*spfactor, sp);
    } else {
        fprintf(ostream, "%*sdeps = [\n", level*spfactor, sp);
    }

    UT_string *condition_name;
    utstring_new(condition_name);

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        /* log_debug("setting %d", i+1); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);
        int flags_ct = 0;
        if (flags != NULL) {
            register_flags(flags);
            flags_ct = obzl_meta_flags_count(flags);
        }

        if (obzl_meta_flags_has_flag(flags, "ppx_driver", false)) {
            continue;
        }

        bool has_conditions;
        if (flags == NULL)
            utstring_printf(condition_name, "//conditions:default");
        else
            has_conditions = obzl_meta_flags_to_select_condition(flags, condition_name);

        char *condition_comment = obzl_meta_flags_to_comment(flags);
        /* log_debug("condition_comment: %s", condition_comment); */

        /* 'requires' usually has no flags; when it does, empirically we find only */
        /*   ppx pkgs: ppx_driver, -ppx_driver */
        /*   pkg 'batteries': requires(mt) */
        /*   pkg 'num': requires(toploop) */
        /*   pkg 'findlib': requires(toploop), requires(create_toploop) */

        /* Multiple settings on 'requires' means multiple flags; */
        /* empirically, this only happens for ppx packages, typically as */
        /* requires(-ppx_driver,-custom_ppx) */
        /* the (sole?) exception is */
        /*   pkg 'threads': requires(mt,mt_vm), requires(mt,mt_posix) */

        if (settings_ct > 1) {
            fprintf(ostream, "%*s\"%s%s\": [ ## predicates: %s\n",
                    (1+level)*spfactor, sp,
                    utstring_body(condition_name),
                    (has_conditions)? "" : "",
                    condition_comment);
        }

        vals = resolve_setting_values(setting, flags, settings);
        /* vals = obzl_meta_setting_values(setting); */
        /* log_debug("vals ct: %d", obzl_meta_values_count(vals)); */
        /* dump_values(0, vals); */
        /* now we handle UPDATE settings */

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            v = obzl_meta_values_nth(vals, j);
            /* log_info("property val: '%s'", *v); */

            char *s = (char*)*v;
            /* special case: uchar */
            if ((strncmp(s, "uchar", 5) == 0)
                && (strlen(s) == 5)){
                /* log_debug("OMITTING UCHAR dep"); */
                continue;
            }
            /* special case: threads */
            /* if ((strncmp(s, "threads", 7) == 0) */
            /*     && (strlen(s) == 7)){ */
            /*     /\* log_debug("OMITTING THREADS dep"); *\/ */
            /*     continue; */
            /* } */
            while (*s) {
                /* printf("s: %s\n", s); */
                if(s[0] == '.') {
                    /* log_info("Hit"); */
                    s[0] = '/';
                }
                s++;
            }
            if (settings_ct > 1) {
                fprintf(ostream, "%*s\"%s//%s/%s\",\n",
                        (2+level)*spfactor, sp,
                        repo, pkg, *v);
            } else {
                fprintf(ostream, "%*s\"%s//%s/%s\",\n", (1+level)*spfactor, sp, repo, pkg, *v);
            }
        }
        if (settings_ct > 1) {
            fprintf(ostream, "%*s],\n", (1+level)*spfactor, sp);
        }
        free(condition_comment);
    }
    utstring_free(condition_name);
    if (settings_ct > 1)
        fprintf(ostream, "%*s}),\n", level*spfactor, sp);
    else
        fprintf(ostream, "%*s],\n", level*spfactor, sp);
}

void emit_bazel_deps_adjunct(FILE* ostream, int level, char *repo,
                             char *_pkg_prefix, /* e.g. 'lib' */
                             obzl_meta_entries *_entries)
{

    //FIXME: skip if no 'requires'
    /* obzl_meta_entries *entries = obzl_meta_package_entries(_pkg); */

    struct obzl_meta_property *deps_prop = obzl_meta_entries_property(_entries, "ppx_runtime_deps");
    if ( deps_prop == NULL ) {
        /* char *pkg_name = obzl_meta_package_name(_pkg); */
        log_warn("Prop 'ppx_runtime_deps' not found: %s.", _pkg_prefix);
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    int settings_no_ppx_driver_ct = obzl_meta_settings_flag_count(settings, "ppx_driver", false);

    settings_ct -= settings_no_ppx_driver_ct;

    /* log_info("settings count: %d", settings_ct); */

    if (settings_ct == 0) {
        log_info("No deps for %s", obzl_meta_property_name(deps_prop));
        return;
    }

    obzl_meta_values *vals;
    obzl_meta_value *v = NULL;

    if (settings_ct > 1) {
        fprintf(ostream, "%*sdeps_adjunct = select({\n", level*spfactor, sp);
    } else {
        fprintf(ostream, "%*sdeps_adjunct = [\n", level*spfactor, sp);
    }

    UT_string *condition_name;
    utstring_new(condition_name);

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        /* log_debug("setting %d", i+1); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);
        int flags_ct = 0;
        if (flags != NULL) {
            register_flags(flags);
            flags_ct = obzl_meta_flags_count(flags);
        }

        if (obzl_meta_flags_has_flag(flags, "ppx_driver", false)) {
            continue;
        }

        bool has_conditions;
        if (flags == NULL)
            utstring_printf(condition_name, "//conditions:default");
        else
            has_conditions = obzl_meta_flags_to_select_condition(flags, condition_name);

        char *condition_comment = obzl_meta_flags_to_comment(flags);
        /* log_debug("condition_comment: %s", condition_comment); */

        /* 'requires' usually has no flags; when it does, empirically we find only */
        /*   ppx pkgs: ppx_driver, -ppx_driver */
        /*   pkg 'batteries': requires(mt) */
        /*   pkg 'num': requires(toploop) */
        /*   pkg 'findlib': requires(toploop), requires(create_toploop) */

        /* Multiple settings on 'requires' means multiple flags; */
        /* empirically, this only happens for ppx packages, typically as */
        /* requires(-ppx_driver,-custom_ppx) */
        /* the (sole?) exception is */
        /*   pkg 'threads': requires(mt,mt_vm), requires(mt,mt_posix) */

        if (settings_ct > 1) {
            fprintf(ostream, "%*s\"%s%s\": [ ## predicates: %s\n",
                    (1+level)*spfactor, sp,
                    utstring_body(condition_name),
                    (has_conditions)? "_enabled" : "",
                    condition_comment);
        }

        vals = resolve_setting_values(setting, flags, settings);
        /* vals = obzl_meta_setting_values(setting); */
        /* log_debug("vals ct: %d", obzl_meta_values_count(vals)); */
        /* dump_values(0, vals); */
        /* now we handle UPDATE settings */

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            v = obzl_meta_values_nth(vals, j);
            /* log_info("property val: '%s'", *v); */

            char *s = (char*)*v;
            while (*s) {
                /* printf("s: %s\n", s); */
                if(s[0] == '.') {
                    /* log_info("Hit"); */
                    s[0] = '/';
                }
                s++;
            }
            if (settings_ct > 1) {
                fprintf(ostream, "%*s\"%s//%s/%s\",\n",
                        (2+level)*spfactor, sp,
                        repo, _pkg_prefix, *v);
            } else {
                fprintf(ostream, "%*s\"%s//%s/%s\",\n", (1+level)*spfactor, sp, repo, _pkg_prefix, *v);
            }
        }
        if (settings_ct > 1) {
            fprintf(ostream, "%*s],\n", (1+level)*spfactor, sp);
        }
        free(condition_comment);
    }
    utstring_free(condition_name);
    if (settings_ct > 1)
        fprintf(ostream, "%*s}),\n", level*spfactor, sp);
    else
        fprintf(ostream, "%*s],\n", level*spfactor, sp);
}

void emit_bazel_path_attrib(FILE* ostream, int level,
                            char *repo,
                            char *_pkg_src,
                            char *pkg,
                            obzl_meta_entries *_entries)
{
    log_debug("emit_bazel_path_attrib");
    /*
The variable "directory" redefines the location of the package directory. Normally, the META file is the first file read in the package directory, and before any other file is read, the "directory" variable is evaluated in order to see if the package directory must be changed. The value of the "directory" variable is determined with an empty set of actual predicates. The value must be either: an absolute path name of the alternate directory, or a path name relative to the stdlib directory of OCaml (written "+path"), or a normal relative path name (without special syntax). In the latter case, the interpretation depends on whether it is contained in a main or sub package, and whether the standard repository layout or the alternate layout is in effect (see site-lib for these terms). For a main package in standard layout the base directory is the directory physically containing the META file, and the relative path is interpreted for this base directory. For a main package in alternate layout the base directory is the directory physically containing the META.pkg files. The base directory for subpackages is the package directory of the containing package. (In the case that a subpackage definition does not have a "directory" setting, the subpackage simply inherits the package directory of the containing package. By writing a "directory" directive one can change this location again.)
    */

    struct obzl_meta_property *deps_prop = obzl_meta_entries_property(_entries, "directory");
    if ( deps_prop == NULL ) {
        /* char *pkg_name = obzl_meta_package_name(_pkg); */
        /* log_warn("Prop 'directory' not found: %s.", pkg_name); */
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    /* log_info("settings count: %d", settings_ct); */

    if (settings_ct == 0) {
        log_info("No deps for %s", obzl_meta_property_name(deps_prop));
        return;
    }
    if (settings_ct > 1) {
        log_warn("More than one setting for property 'description'; using the first");
        return;
    }

    UT_string *condition_name;
    setting = obzl_meta_settings_nth(settings, 0);

    obzl_meta_flags *flags = obzl_meta_setting_flags(setting);
    int flags_ct = obzl_meta_flags_count(flags);
    if (flags_ct > 0) {
        log_error("Property 'directory' has unexpected flags.");
        return;
    }

    obzl_meta_values *vals = resolve_setting_values(setting, flags, settings);
    log_debug("vals ct: %d", obzl_meta_values_count(vals));
    /* dump_values(0, vals); */

    if (obzl_meta_values_count(vals) == 0) {
        return;
    }

    if (obzl_meta_values_count(vals) > 1) {
        log_error("Property 'directory' has more than one value.");
        return;
    }

    obzl_meta_value *dir = NULL;
    dir = obzl_meta_values_nth(vals, 0);
    log_info("property val: '%s'", *dir);

    bool stdlib = false;
    if ( strncmp(*dir, "+", 1) == 0 ) {
        (*dir)++;
        stdlib = true;
    }
    log_info("property val 2: '%s'", *dir);
    fprintf(ostream, "%s",
            "    modules = [\"//:compiler-libs.common\"],\n");

    /* fprintf(ostream, "%s", */
    /*         "    modules = glob([\n"); */
    /* fprintf(ostream, */
    /*         "        \"../../%s/%s%s/\*.cmi\",\n", _pkg_src, stdlib? "ocaml/" : "", *dir); */
    /* fprintf(ostream, */
    /*         "        \"../../%s/%s%s/\*.mli\",\n", _pkg_src, stdlib? "ocaml/" : "", *dir); */
    /* fprintf(ostream, "%s", "    ]) + select({\n"); */
    /* fprintf(ostream, "%s", "        \"@ocaml//mode:native\": glob([\n"); */
    /* fprintf(ostream, */
    /*         "            \"//:%s/%s%s/\*.cmx\",\n", */
    /*         _pkg_src, stdlib? "ocaml/" : "", *dir); */
    /* fprintf(ostream, "%s", "            ]),\n"); */
    /* fprintf(ostream, "%s", "        \"@ocaml//mode:bytecode\": glob([\n"); */
    /* fprintf(ostream, */
    /*         "            \"//:%s/%s%s/\*.cmo\",\n", */
    /*         _pkg_src, stdlib? "ocaml/" : "", *dir); */
    /* fprintf(ostream, */
    /*         "            \"//:%s/%s%s/\*.o\",\n", */
    /*         _pkg_src, stdlib? "ocaml/" : "", *dir); */
    /* fprintf(ostream, "%s", "            ]),\n"); */
    /* fprintf(ostream, */
    /*         "    }),\n"); */

    /* fprintf(ostream, "%s", "    \\n"); */
}

void emit_bazel_dummy_target(FILE* ostream, int level,
                               char *_repo,
                               char *_pkg_src,
                               char *_pkg_path,
                               char *_pkg_name,
                               obzl_meta_entries *_entries)
{
    /* for packages with 'requires' but no 'archive' or 'plugin' */
    /* e.g. package 'threads' */
    fprintf(ostream, "\nocaml_import(\n");
    /* if deps count > 0 */
    /* fprintf(ostream, "    name = \"%s\",\n", _pkg_name); */
    /* else */
    fprintf(ostream, "    name = \"%s\",\n", _pkg_name);
    emit_bazel_deps(ostream, 1, "@opam", "lib", _entries);
    emit_bazel_path_attrib(ostream, 1, "@opam", _pkg_src, "lib", _entries);
    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
}

// FIXME: most of this duplicates emit_bazel_deps
void emit_bazel_ppx_dummy_deps(FILE* ostream, int level, char *repo, char *pkg,
                               obzl_meta_entries *_entries)
                     /* obzl_meta_package *_pkg) */
{
    log_debug("emit_bazel_ppx_dummy_deps");
    //FIXME: skip if no 'requires'
    /* obzl_meta_entries *entries = obzl_meta_package_entries(_pkg); */

    char *pname = "requires";
    /* struct obzl_meta_property *deps_prop = obzl_meta_package_property(_pkg, pname); */
    struct obzl_meta_property *deps_prop = obzl_meta_entries_property(_entries, pname);
    if ( deps_prop == NULL ) {
        /* char *pkg_name = obzl_meta_package_name(_pkg); */
        /* log_warn("Prop '%s' not found: %s.", pname, pkg_name); */
        return;
    }

    obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop);
    obzl_meta_setting *setting = NULL;

    int settings_ct = obzl_meta_settings_count(settings);
    int settings_no_ppx_driver_ct = obzl_meta_settings_flag_count(settings, "ppx_driver", false);

    /* settings_ct -= settings_no_ppx_driver_ct; */

    /* log_info("settings count: %d", settings_ct); */

    if (settings_no_ppx_driver_ct == 0) {
        log_info("No 'requires(-ppx_driver)' for %s", obzl_meta_property_name(deps_prop));
        return;
    }

    obzl_meta_values *vals;
    obzl_meta_value *v = NULL;

    if (settings_no_ppx_driver_ct > 1) {
        fprintf(ostream, "%*sdeps = select({\n", level*spfactor, sp);
    } else {
        fprintf(ostream, "%*sdeps = [\n", level*spfactor, sp);
    }

    UT_string *condition_name;
    utstring_new(condition_name);

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        log_debug("setting %d", i+1);
        /* dump_setting(8, setting); */

        if (obzl_meta_setting_has_flag(setting, "ppx_driver", true)) {
            log_debug("skipping setting with flag +ppx_driver");
            continue;
        }

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);
        /* int flags_ct = 0; */
        /* if (flags != NULL) { */
        /*     register_flags(flags); */
        /*     flags_ct = obzl_meta_flags_count(flags); */
        /* } */

        /* select only settings with flag -ppx_driver */
        if ( !obzl_meta_flags_has_flag(flags, "ppx_driver", false) ) {
            continue;
        }

        bool has_conditions;
        if (flags == NULL)
            utstring_printf(condition_name, "//conditions:default");
        else
            has_conditions = obzl_meta_flags_to_select_condition(flags, condition_name);

        char *condition_comment = obzl_meta_flags_to_comment(flags);
        /* log_debug("condition_comment: %s", condition_comment); */

        /* 'requires' usually has no flags; when it does, empirically we find only */
        /*   ppx pkgs: ppx_driver, -ppx_driver */
        /*   pkg 'batteries': requires(mt) */
        /*   pkg 'num': requires(toploop) */
        /*   pkg 'findlib': requires(toploop), requires(create_toploop) */

        /* Multiple settings on 'requires' means multiple flags; */
        /* empirically, this only happens for ppx packages, typically as */
        /* requires(-ppx_driver,-custom_ppx) */
        /* the (sole?) exception is */
        /*   pkg 'threads': requires(mt,mt_vm), requires(mt,mt_posix) */

        // FIXME: we're assuming we always have:
        // 'requires(-ppx_driver)' and 'requires(-ppx_driver,-custom_ppx)'

        if (settings_ct > 1) {
            fprintf(ostream, "%*s\"%s\": [ ## predicates: %s\n",
                    (1+level)*spfactor, sp,
                    utstring_body(condition_name), // : "//conditions:default",
                    /* (has_conditions)? "_enabled" : "", */
                    condition_comment);
        }

        vals = resolve_setting_values(setting, flags, settings);
        /* vals = obzl_meta_setting_values(setting); */
        /* log_debug("vals ct: %d", obzl_meta_values_count(vals)); */
        /* dump_values(0, vals); */
        /* now we handle UPDATE settings */

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            v = obzl_meta_values_nth(vals, j);
            /* log_info("property val: '%s'", *v); */

            char *s = (char*)*v;
            while (*s) {
                /* printf("s: %s\n", s); */
                if(s[0] == '.') {
                    /* log_info("Hit"); */
                    s[0] = '/';
                }
                s++;
            }
            if (settings_ct > 1) {
                fprintf(ostream, "%*s\"%s//%s/%s\",\n",
                        (2+level)*spfactor, sp,
                        repo, pkg, *v);
            } else {
                fprintf(ostream, "%*s\"%s//%s/%s\"\n", (1+level)*spfactor, sp, repo, pkg, *v);
            }
        }
        if (settings_ct > 1) {
            fprintf(ostream, "%*s],\n", (1+level)*spfactor, sp);
        }
        free(condition_comment);
    }
    utstring_free(condition_name);
    if (settings_no_ppx_driver_ct > 1)
        fprintf(ostream, "%*s}),\n", level*spfactor, sp);
    else
        fprintf(ostream, "%*s],\n", level*spfactor, sp);
}

void emit_bazel_subpackages(char *_repo,
                            char *_repo_root,
                            char *_pkg_prefix,
                            char *_imports_path,
                            /* char *_pkg_path, */
                            struct obzl_meta_package *_pkg)
                            /* char *_subpkg_dir) */
{
    log_debug("emit_bazel_subpackages, pfx: %s", _pkg_prefix);
    log_debug("\t_repo: %s; _repo_root: %s", _repo, _repo_root);
    log_debug("\t_pkg_prefix: %s", _pkg_prefix); //, _pkg_path);
    log_debug("\t_imports_path: %s", _imports_path);

    obzl_meta_entries *entries = _pkg->entries;
    obzl_meta_entry *entry = NULL;

    char *pkg_name = obzl_meta_package_name(_pkg);
    UT_string *_new_pkg_prefix;
    utstring_new(_new_pkg_prefix);
    utstring_printf(_new_pkg_prefix, "%s/%s", _pkg_prefix, pkg_name);

    for (int i = 0; i < obzl_meta_entries_count(entries); i++) {
        entry = obzl_meta_entries_nth(entries, i);
        if (entry->type == OMP_PACKAGE) {
            obzl_meta_package *subpkg = entry->package;
            obzl_meta_entries *sub_entries = subpkg->entries;
            char *subdir = obzl_meta_directory_property(sub_entries);

            emit_build_bazel(_repo, _repo_root,
                             utstring_body(_new_pkg_prefix),
                             _imports_path,
                             //_pkg_path,
                             subpkg);
            /* select only subpackages with non-empty 'directory' prop */
            /* obzl_meta_property *dprop */
            /*     = obzl_meta_entries_property(e->package->entries, */
            /*                                  "directory"); */
            /* if (dprop) { */
            /*     log_debug("DPROP %s := %s", */
            /*               obzl_meta_property_name(dprop), */
            /*               obzl_meta_property_value(dprop)); */
            /*     emit_build_bazel(_repo, _repo_root, _pkg_prefix, _pkg_path, e->package); */
            /* } else { */
            /*     log_debug("skipping subpackage with empty 'directory' prop"); */
            /* } */
        }
    }
    utstring_free(_new_pkg_prefix);
}

/* void emit_bazel_flags_mt(char *pfx) */
/* { */
/*     UT_string *fname; */
/*     utstring_new(fname); */

/*     utstring_printf(fname, "%s", bzlroot); */
/*     utstring_printf(fname, "%s",  "/cfg/mt"); */
/*     mkdir_r(utstring_body(fname), ""); */
/*     utstring_printf(fname, "%s",  "/BUILD.bazel"); */

/*     log_debug("writing cfg file: %s", utstring_body(fname)); */
/*     fprintf(stdout, "writing cfg file: %s\n", utstring_body(fname)); */

/*     FILE *ostream = fopen(utstring_body(fname), "w"); */
/*     if (ostream == NULL) { */
/*         perror(utstring_body(fname)); */
/*         log_error("fopen fail on %s", utstring_body(fname)); */
/*         fflush(stderr); fflush(stdout); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     fprintf(ostream, "%s", */
/*             "load(\"@bazel_skylib//rules:common_settings.bzl\", \"bool_flag\", \"string_flag\")\n" */
/*             "\n" */
/*             "package(default_visibility = [\"//visibility:public\"])\n" */
/*             "\n" */
/*             "bool_flag     ( name = \"mt\", build_setting_default = False)\n" /\* --@opam//cfg/mt:mt *\/ */
/*             "config_setting("   /\* @opam//cfg/mt:enabled *\/ */
/*             " name = \"enabled\"," */
/*             " flag_values = { \":mt\": \"True\"}" */
/*             ")\n" */
/*             "\n" */
/*             "bool_flag( name = \"posix\", build_setting_default = False)\n" /\* --@opam//cfg/mt:posix *\/ */
/*             "bool_flag( name = \"vm\", build_setting_default = False)\n" /\* --@opam//cfg/mt:vm *\/ */
/*             "\n"); */
/*     fclose(ostream); */

/*     /\* **************** *\/ */
/*     utstring_clear(fname); */
/*     utstring_printf(fname, "%s", bzlroot); */
/*     utstring_printf(fname, "%s", "/cfg/mt/posix"); */
/*     mkdir_r(utstring_body(fname), ""); */
/*     utstring_printf(fname, "%s", "/BUILD.bazel"); */

/*     log_debug("writing cfg file: %s", utstring_body(fname)); */
/*     ostream = fopen(utstring_body(fname), "w"); */
/*     if (ostream == NULL) { */
/*         perror(utstring_body(fname)); */
/*         log_error("fopen fail on %s", utstring_body(fname)); */
/*         fflush(stderr); fflush(stdout); */
/*         exit(EXIT_FAILURE); */
/*     } */
/*     fprintf(ostream, "%s", */
/*             "load(\"@bazel_skylib//rules:common_settings.bzl\", \"bool_flag\")\n" */
/*             "\n" */
/*             "package(default_visibility = [\"//visibility:public\"])\n" */
/*             "\n" */
/*             "config_setting("   /\* @opam//cfg/vm:enabled *\/ */
/*             " name = \"enabled\"," */
/*             " flag_values = { \"//cfg/mt:vm\": \"True\"} " */
/*             ")\n" */
/*             "\n"); */
/*     fclose(ostream); */
/* } */

/* void emit_bazel_config_setting_rules(char *pfx) */
/* { */
/*     log_debug("emit_bazel_flags"); */
/*     const char **flag = NULL; */
/*     struct config_flag *config_flag; */

/*     emit_bazel_flags_mt(pfx); */

/*     char fname[PATH_MAX]; */
/*     FILE *ostream; */

/*     char config_setting[PATH_MAX]; */

/*     fname[0] = '\0'; */
/*     mkdir_r(bzlroot, "cfg"); */
/*     mystrcat(fname, bzlroot); */
/*     mystrcat(fname, "/"); */
/*     mystrcat(fname, "cfg/BUILD.bazel"); */
/*     log_debug("writing cfg file: %s", fname); */
/*     ostream = fopen(fname, "w"); */
/*     if (ostream == NULL) { */
/*         perror(fname); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     fprintf(ostream, "load(\"@bazel_skylib//rules:common_settings.bzl\", \"bool_flag\")\n\n"); */

/*     fprintf(ostream, "package(default_visibility = [\"//visibility:public\"])\n\n"); */

/*     while ( (flag = (const char**)utarray_next(pos_flags, flag)) != NULL ) { */

/*         /\* mt flags are handled above by emit_bazel_flags_mt() *\/ */
/*         if (strncmp(*flag, "mt", 2) == 0 ) continue; */
/*         if (strncmp(*flag, "mt_posix", 8) == 0 ) continue; */
/*         if (strncmp(*flag, "mt_vm", 5) == 0 ) continue; */

/*         /\* ppx_driver not a flag for obazl *\/ */
/*         if (strncmp(*flag, "ppx_driver", 10) == 0 ) continue; */

/*         /\* HASH_FIND_STR(the_flag_table, *flag, config_flag); *\/ */
/*         /\* if (config_flag == NULL) { *\/ */
/*         /\*     log_debug("flag %s not found in flags table.", flag); *\/ */
/*         /\* } else { *\/ */
/*         fprintf(ostream, */
/*                 "bool_flag( name = \"%s\", build_setting_default = False )\n", */
/*                 *flag); */
/*         fprintf(ostream, */
/*                 "config_setting( name = \"%s_enabled\", flag_values = {\":%s\": \"True\"} )\n", */
/*                 *flag, *flag); */
/*         fprintf(ostream, */
/*                 "config_setting( name = \"%s_disabled\", flag_values = {\":%s\": \"False\"} )\n\n", */
/*                 *flag, *flag); */
/*         /\* } *\/ */
/*     } */

/*     while ( (flag = (const char**)utarray_next(neg_flags, flag)) != NULL ) { */

/*         /\* FIXME: write no_* rules for mt? *\/ */
/*         if (strncmp(*flag, "mt", 2) == 0 ) continue; */
/*         if (strncmp(*flag, "mt_posix", 8) == 0 ) continue; */
/*         if (strncmp(*flag, "mt_vm", 5) == 0 ) continue; */

/*         fprintf(ostream, */
/*                 "bool_flag( name = \"no_%s\", build_setting_default = False )\n", */
/*                 *flag); */
/*         fprintf(ostream, */
/*                 "config_setting( name = \"no_%s_enabled\", flag_values = {\":%s\": \"True\"} )\n", */
/*                 *flag, *flag); */
/*         fprintf(ostream, */
/*                 "config_setting( name = \"no_%s_disabled\", flag_values = {\":%s\": \"False\"} )\n\n", */
/*                 *flag, *flag); */
/*         /\* } *\/ */
/*     } */

/*     /\* now config_setting rules for the compound conditions *\/ */

/*     struct config_setting *cs, *cstmp; */
/*     HASH_ITER(hh, the_config_settings, cs, cstmp) { */
/* #ifdef DEBUG_TRACE */
/*         log_debug("emitting config setting: %s", cs->label); */
/* #endif */
/*         fprintf(ostream, */
/*                 "config_setting(\n" */
/*                 "    name = \"%s\",\n" */
/*                 "    flag_values = {\n", */
/*                 cs->name); */
/*         int ct = obzl_meta_flags_count(cs->flags); */
/* #ifdef DEBUG_TRACE */
/*         log_trace("%*sflags ct: %d", indent, sp, ct); */
/* #endif */
/*         char config_name[128]; */
/*         config_name[0] = '\0'; */
/*         struct obzl_meta_flag *a_flag = NULL; */
/*         for (int i=0; i < ct; i++) { */
/*             a_flag = obzl_meta_flags_nth(cs->flags, i); */
/* #ifdef DEBUG_TRACE */
/*             log_debug("%*s%s (polarity: %d)", delta+indent, sp, a_flag->s, a_flag->polarity); */
/* #endif */
/*             if (strncmp(a_flag->s, "byte", 4) == 0) { */
/*                 fprintf(ostream, "%s", "        \"@ocaml//mode\": \"bytecode\",\n"); */
/*                 continue; */
/*             } */
/*             if (strncmp(a_flag->s, "native", 6) == 0) { */
/*                 fprintf(ostream, "%s", "        \"@ocaml//mode\": \"native\",\n"); */
/*                 continue; */
/*             } */
/*             if (strncmp(a_flag->s, "mt_posix", 8) == 0) { */
/*                 fprintf(ostream, "%s", "        \"//cfg/mt:posix\": \"True\",\n"); */
/*                 continue; */
/*             } */
/*             if (strncmp(a_flag->s, "mt_vm", 5) == 0) { */
/*                 fprintf(ostream, "%s", "        \"//cfg/mt:vm\": \"True\",\n"); */
/*                 continue; */
/*             } */
/*             if (strncmp(a_flag->s, "mt", 2) == 0) { */
/*                 fprintf(ostream, "%s", "        \"//cfg/mt:mt\": \"True\",\n"); */
/*                 continue; */
/*             } */
/*             fprintf(ostream, "        \":%s%s\": \"True\",\n", */
/*                     a_flag->polarity? "" : "no_", */
/*                     a_flag->s); */
/*         } */
/*         fprintf(ostream, "%s", "    },\n"); */
/*         fprintf(ostream, "%s", "    visibility = [\"//visibility:public\"]\n"); */
/*         fprintf(ostream, "%s", ")\n\n"); */
/*     } */

/*     fclose(ostream); */
/* } */

void handle_directory_property(FILE* ostream, int level,
                               char *_repo,
                               char *_pkg_src,
                               char *_pkg_name,
                               obzl_meta_entries *_entries)

{
    /*
The variable "directory" redefines the location of the package directory. Normally, the META file is the first file read in the package directory, and before any other file is read, the "directory" variable is evaluated in order to see if the package directory must be changed. The value of the "directory" variable is determined with an empty set of actual predicates. The value must be either: an absolute path name of the alternate directory, or a path name relative to the stdlib directory of OCaml (written "+path"), or a normal relative path name (without special syntax). In the latter case, the interpretation depends on whether it is contained in a main or sub package, and whether the standard repository layout or the alternate layout is in effect (see site-lib for these terms). For a main package in standard layout the base directory is the directory physically containing the META file, and the relative path is interpreted for this base directory. For a main package in alternate layout the base directory is the directory physically containing the META.pkg files. The base directory for subpackages is the package directory of the containing package. (In the case that a subpackage definition does not have a "directory" setting, the subpackage simply inherits the package directory of the containing package. By writing a "directory" directive one can change this location again.)
    */

    /*
      IOW, the 'directory' property tells us where 'archive' and
      'plugin' files are to be found. Does not affect interpretation
      of 'requires' property (I think).
     */
    /*
      Directory '^' means '@opam//lib/ocaml'. problem
      is lib/ocaml has no META file; it's the stdlib, always included.
      what about subdirs like 'threads' or 'integers'?

      If directory starts with '+', pkg is relative to '@opam//lib/ocaml'. problem
      is lib/ocaml has no META file; it's the stdlib, always included.
      what about subdirs like 'threads' or 'integers'?
    */

    /* should have one setting */
    /* char *dir = (char*)*obzl_meta_property_value(e->property); */

    /*
      Special cases: ^, +
    */

    /* char *pkg_name = obzl_meta_package_name(_pkg); */
    /* log_debug("Directory prop val: %s", dir); */
    /* log_debug("Package name: %s", pkg_name); */
    /* if ( strncmp(dir, pkg_name, PATH_MAX) ) { */
    /*     log_error("Mismatch: pkg '%s', directory prop '%s', in %s", */
    /*               pkg_name, dir, THE_METAFILE); */
        /* fclose(ostream); */
        /* exit(EXIT_FAILURE); */
    /* } */
}

/*
  params:
  _tgtroot: output dir
  _repo: @opam

  _pkg_prefix - result of concatenating ancestors up to parent; starts
                with bzl_lib (default: "lib")

  pkg path will be made by concatenating _pkg_prefix to pkg name
 */
EXPORT void emit_build_bazel(char *_repo,
                             char *_repo_root,
                             char *_pkg_prefix,
                             char *_imports_path,
                             /* char *_pkg_path, /\* FIXME: remove *\/ */
                             struct obzl_meta_package *_pkg)
                             /* char *_subpkg_dir) */
{
    log_info("EMIT_BUILD_BAZEL");
    char *pkg_name = obzl_meta_package_name(_pkg);
#ifdef DEBUG
    log_info("\n\t_repo: @%s; _pkg_prefix: '%s'; pkg name: '%s'",
             _repo, _pkg_prefix, pkg_name);
    /* log_info("\n\t_subpkg_dir: %s", _subpkg_dir); */
    log_info("\tlabel: @%s//%s/%s", _repo, _pkg_prefix, pkg_name);
    log_info("\t_imports_path: '%s'", _imports_path);
    log_info("\t_repo_root: '%s'", _repo_root);
    /* log_set_quiet(false); */
    /* log_debug("%*sparsed name: %s", indent, sp, pkg_name); */
    log_debug("%*spkg dir:  %s", indent, sp, obzl_meta_package_dir(_pkg));
    log_debug("%*sparsed src:  %s", indent, sp, obzl_meta_package_src(_pkg));
#endif

    utstring_renew(build_bazel_file);
    /* utstring_clear(build_bazel_file); */
    utstring_printf(build_bazel_file, "%s/%s", _repo_root, _pkg_prefix);

    obzl_meta_entries *entries = _pkg->entries;
    char *subpkg_dir = obzl_meta_directory_property(entries);
    if (subpkg_dir != NULL) {
        log_debug("directory property: %s", subpkg_dir);
        /* utstring_printf(build_bazel_file, "/%s", subpkg_dir); */
    }

    /* if (obzl_meta_package_dir(_pkg) != NULL) */
    utstring_printf(build_bazel_file, "/%s", pkg_name);

    mkdir_r(utstring_body(build_bazel_file), "");
    utstring_printf(build_bazel_file, "/%s", "BUILD.bazel");

    log_debug("emitting: %s", utstring_body(build_bazel_file));
    /* fprintf(stdout, "Writing: %s\n", utstring_body(build_bazel_file)); */

    FILE *ostream;
    ostream = fopen(utstring_body(build_bazel_file), "w");
    if (ostream == NULL) {
        perror(utstring_body(build_bazel_file));
        /* log_error("fopen failure for %s", utstring_body(build_bazel_file)); */
        /* log_error("Value of errno: %d", errnum); */
        /* log_error("fopen error %s", strerror( errnum )); */
        exit(EXIT_FAILURE);
    }

    fprintf(ostream, "## original: %s\n\n", obzl_meta_package_src(_pkg));

    emit_bazel_hdr(ostream, 1, "@opam", "lib", _pkg);

    struct obzl_meta_property *directory_prop = obzl_meta_entries_property(entries, "directory");
    char *directory;
    if ( directory_prop == NULL ) {
        ;
        /* log_warn("Prop 'directory' not found."); */
        /* dump_package(0, _pkg); */
    } else {
        directory = (char*)obzl_meta_property_value(directory_prop);
        /* log_info("Prop 'directory': %s", directory); */
        if (directory == NULL) {
            ; /* from directory = "" */
        } else {
            if ( strncmp(directory, "+", 1) == 0 ) {
                /* stdlib = true; */
                /* log_debug("Found STDLIB directory '%s' for %s", directory, pkg_name); */
                directory++;
            } else {
                if ( (strlen(directory) == 1)
                     && (strncmp(directory, "^", 1) == 0) ) {
                    /* stdlib = true; */
                    stdlib_root = true;
                    /* log_debug("Found STDLIB root directory '%s' for %s", directory, pkg_name); */
                }
            }
        }
    }

    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PROPERTY) {
        /* log_debug("emitting entry %d, type %d", i, e->type); */
        /* if ((e->type == OMP_PROPERTY) || (e->type == OMP_PACKAGE)) { */
            /* log_debug("\tOMP_PROPERTY"); */

            /* 'library_kind` :=  'ppx_deriver' or 'ppx_rewriter' */
            /* if (strncmp(e->property->name, "library_kind", 12) == 0) { */
            /*     emit_bazel_ppx_dummy_rule(ostream, 1, "@opam", "_lib", _pkg_path, pkg_name, entries); */
            /*     continue; */
            /* } */
            if (strncmp(e->property->name, "archive", 7) == 0) {
                log_debug("calling emit_bazel_archive_rule with %s",
                          _imports_path);
                emit_bazel_archive_rule(ostream, 1,
                                        _repo, // "@opam",
                                        _imports_path,
                                        _pkg_prefix,
                                        _pkg_prefix,
                                        pkg_name,
                                        entries);
                                        /* subpkg_dir); */
                continue;
            }
            if (strncmp(e->property->name, "plugin", 6) == 0) {
                emit_bazel_plugin_rule(ostream, 1, "@opam", "_lib",
                                       _pkg_prefix,
                                       _pkg_prefix,
                                       pkg_name, entries, subpkg_dir);
                continue;
            }
            /* if (strncmp(e->property->name, "ppx", 3) == 0) { */
            /*     log_warn("IGNORING PPX PROPERTY for %s", pkg_name); */
            /*     continue; */
            /* } */
            if (strncmp(e->property->name, "requires", 6) == 0) {
                /* some pkgs have 'requires' but no 'archive' nor 'plugin' */
                /* compiler-libs has 'requires = ""' and 'director = "+compiler-libs"' */
                /* ppx packages have 'requires(ppx_driver)' and 'requires(-ppx_driver)' */
                if (obzl_meta_entries_property(entries, "archive"))
                    continue;
                if (obzl_meta_entries_property(entries, "plugin"))
                    continue;
                emit_bazel_dummy_target(ostream, 1, "@opam", "_lib",
                                        _pkg_prefix,
                                        pkg_name, entries);
                continue;
            }
            /* some (dummy) packages have 'requires' but no 'archive' or 'plugin' entries */
            if (strncmp(e->property->name, "directory", 9) == 0) {
                handle_directory_property(ostream, 1, "@opam", "_lib", pkg_name, entries);
                continue;
            }

            log_warn("processing other property: %s", e->property->name);
            // push all flags to global pos_flags
        }
    }

    /* emit_bazel_flags(ostream, _repo_root, _repo, _pkg_prefix, _pkg); */

    fclose(ostream);

    /* char new_pkg_path[PATH_MAX]; */
    /* new_pkg_path[0] = '\0'; */
    /* mystrcat(new_pkg_path, _pkg_path); */
    /* if (strlen(_pkg_path) > 0) */
    /*     mystrcat(new_pkg_path, "/"); */

    /* subpackage may not have a 'directory' property. example: ptime */
    char  *subpkg_name = obzl_meta_package_name(_pkg);
    /* char *subpkg_dir = obzl_meta_directory_property(entries); */
    /* log_debug("SUBPKG NAME: %s; DIR: %s", subpkg_name, subpkg_dir); */

    /* mystrcat(new_pkg_path, subpkg_name); */
    /* log_debug("NEW_PKG_PATH: %s; PFX: %s", new_pkg_path, _pkg_prefix); */
    /* log_debug("_PKG_PREFIX: %s, _PKG_PATH: %s, _PKG: %s", */
    /*           _pkg_prefix, _pkg_path, _pkg->name); */

    /* SPECIAL CASE: compiler-libs */
    /* if (strncmp(_pkg_path, "compiler-libs", 13) == 0) { */
    /*     /\* emit_bazel_compiler_lib_subpackages(_repo_root, _repo, _pkg_prefix, new_pkg_path, _pkg); *\/ */
    /*     emit_bazel_subpackages(_repo, _repo_root, */
    /*                            _pkg_prefix, _imports_path, */
    /*                            // _pkg_path, */
    /*                            _pkg); //, _subpkg_dir); */
    /* } else { */
        emit_bazel_subpackages(_repo, _repo_root,
                               _pkg_prefix, _imports_path,
                               // new_pkg_path,
                               _pkg); //, _subpkg_dir);
    /* } */
}
