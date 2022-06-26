#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#include <libgen.h>

#if EXPORT_INTERFACE
#include <stdio.h>
#endif

#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "utarray.h"
#include "utstring.h"

#include "emit_build_bazel.h"

/* **************************************************************** */
static int level = 0;
static int spfactor = 4;
static char *sp = " ";

#if defined(DEBUG_TRACE)
static int indent = 2;
static int delta = 2;
#endif

bool stdlib_root = false;

/* char *buildfile_prefix = "@//" HERE_OBAZL_ROOT "/buildfiles"; */
/* was: "@//.opam.d/buildfiles"; */

long *KPM_TABLE;

FILE *opam_resolver;

UT_string *repo_name = NULL;

UT_string *bazel_pkg_root;
UT_string *build_bazel_file;
UT_string *workspace_file;

/* void write_opam_resolver(char *pkg_prefix, char *pkg_name, */
/*                          obzl_meta_entries *entries) */
/* { */
/*     // should only be called for here switch? */
/*     log_error("write_opam_resolver"); */
/*     // CAVEAT: output will contain duplicate entries */
/*     if (pkg_prefix == NULL) { */
/*         fprintf(opam_resolver, "(%s . @%s//:%s)\n", */
/*                 pkg_name, pkg_name, pkg_name); */
/*     } else { */
/*         char *split = strchr(pkg_prefix, '/'); */
/*         if ( split == NULL ) { */
/*             fprintf(opam_resolver, "(%s.%s . @%s//%s)\n", */
/*                     pkg_prefix, pkg_name, */
/*                     pkg_prefix, pkg_name); */
/*         } else { */
/*             /\* this will work for one '/', e.g. mtime/clock, */
/*                so we're good for up to three segs (e.g. mtime.clock.os)*\/ */
/*             int seg1_len = split - pkg_prefix; */
/*             int seg2_len = strlen(pkg_prefix) - seg1_len; */
/*             char *split2 = strchr(pkg_prefix + seg1_len + 1, '/'); */
/*             if (split2 == NULL) { */
/*                 // only one '/' */
/*                 fprintf(opam_resolver, "(%.*s.%.*s.%s . @%.*s//%s/%s) ;; %s %s\n", */
/*                         seg1_len, */
/*                         pkg_prefix, */
/*                         seg2_len, */
/*                         pkg_prefix + seg1_len + 1, */
/*                         pkg_name, */

/*                         seg1_len, */
/*                         pkg_prefix, */
/*                         pkg_prefix + seg1_len + 1, */
/*                         pkg_name, */
/*                         pkg_prefix, pkg_name); */
/*             } else { */
/*                 seg2_len = split2 - split - 1; */
/*                 /\* int seg3_len = strlen(pkg_prefix) - seg2_len; *\/ */
/*                 fprintf(opam_resolver, "(%.*s.%.*s.%s.%s . @%.*s//%s/%s) ;; %s %s\n", */
/*                         seg1_len, */
/*                         pkg_prefix, */
/*                         seg2_len, */
/*                         pkg_prefix + seg1_len + 1, */
/*                         pkg_prefix + seg1_len + seg2_len + 2, */
/*                         pkg_name, */

/*                         seg1_len, */
/*                         pkg_prefix, */
/*                         pkg_prefix + seg1_len + 1, */
/*                         pkg_name, */
/*                         pkg_prefix, pkg_name); */
/*             } */
/*         } */
/*     } */
/* } */

void emit_new_local_subpkg_entries(FILE *bootstrap_FILE,
                                   struct obzl_meta_package *_pkg,
                                   char *_subdir,
                                   char *_pkg_prefix,
                                   char *_filedeps_path)
{
    log_debug("EMIT_NEW_local_subpkg_entries: %s", _pkg->name);
    /* dump_package(0, _pkg); */

    char *pkg_name = _pkg->name;
    int subpkg_ct = obzl_meta_package_subpkg_count(_pkg);
    log_debug("subpkg_ct: %d", subpkg_ct);

    if (subpkg_ct > 0) {

        obzl_meta_entries *entries = _pkg->entries;
        obzl_meta_entry *entry = NULL;
        obzl_meta_package *subpkg = NULL;

        UT_string *_new_pkg_prefix;
        utstring_new(_new_pkg_prefix);
        if (_pkg_prefix == NULL)
            utstring_printf(_new_pkg_prefix, "%s", pkg_name);
        else
            utstring_printf(_new_pkg_prefix, "%s/%s", _pkg_prefix, pkg_name);

        for (int i = 0; i < obzl_meta_entries_count(entries); i++) {
            entry = obzl_meta_entries_nth(entries, i);
            subpkg = entry->package;
            if (entry->type == OMP_PACKAGE) {
                log_debug("next subpkg: %s", subpkg->name);

                /* char *directory = NULL; */
                UT_string *filedeps_dir;
                utstring_new(filedeps_dir);
                if (_filedeps_path)
                    utstring_printf(filedeps_dir, "%s", _filedeps_path);

                struct obzl_meta_property *directory_prop = obzl_meta_entries_property(subpkg->entries, "directory");
                if (directory_prop) {
                    utstring_printf(filedeps_dir, "/%s",
                                    (char*)obzl_meta_property_value(directory_prop));
                }

                UT_string *subdir;
                utstring_new(subdir);

                /* dump_package(0, subpkg); */

                /* fprintf(bootstrap_FILE, "## subpkg: %s\n", subpkg->name); */

                if (obzl_meta_entries_property(subpkg->entries, "error")) {
                    // FIXME: emit 'fail()'
                    /* fprintf(bootstrap_FILE, */
                    /*         "            \"%s %s/lib\",\n", */
                    /*         subpkg->name, */
                    /*         opam_switch_prefix); */
                    ;           /* skip */
                } else {
                    fprintf(bootstrap_FILE,
                            "            \"@//%s/%s/%s:BUILD.bazel\":\n",
                            /* buildfile_prefix, */
                            utstring_body(bzl_switch_pfx),
                            utstring_body(_new_pkg_prefix),
                            /* pkg_name, */
                            subpkg->name);

                    utstring_renew(subdir);
                    if (_subdir == NULL) {
                        utstring_printf(subdir, "%s", subpkg->name);
                    } else {
                        utstring_printf(subdir, "%s/%s",
                                        _subdir, // _pkg_prefix,
                                        subpkg->name);
                    }
                    fprintf(bootstrap_FILE,
                            "            \"%s %s\",\n",
                            utstring_body(subdir),
                            /* opam_switch_prefix, */
                            /* depfiles_path, */
                            utstring_body(filedeps_dir));
                            /* filedeps_dir); */
                            /* pkg_name, */
                            /* subpkg->name); */
                }
                fprintf(bootstrap_FILE, "\n");

                /* recur to get subsubpkgs */
                emit_new_local_subpkg_entries(bootstrap_FILE,
                                              subpkg,
                                              utstring_body(subdir),
                                              utstring_body(_new_pkg_prefix),
                                              utstring_body(filedeps_dir));
                utstring_free(filedeps_dir);
                utstring_free(subdir);
            }
        }
    }
}

void emit_new_local_pkg_repo(FILE *bootstrap_FILE,
                             struct obzl_meta_package *_pkg)
{
    log_debug("emit_new_local_pkg_repo %s", _pkg->name);

    /* first emit for root pkg */
    /* then iterate over subpkgs */

    char *pkg_name = _pkg->name; // obzl_meta_package_name(_pkg);

    //FIXME rename:  new_local_opam_pkg_repository(\n");
    fprintf(bootstrap_FILE, "    new_local_pkg_repository(\n");
    /* fprintf(bootstrap_FILE, "    native.new_local_repository(\n"); */

    fprintf(bootstrap_FILE, "        name       = ");

    char *_pkg_prefix = NULL;
    // ????
    if (_pkg_prefix == NULL)
        fprintf(bootstrap_FILE, "\"%s\",\n", pkg_name);
    else {
        char *s = _pkg_prefix;
        char *tmp;
        while(s) {
            tmp = strchr(s, '/');
            if (tmp == NULL) break;
            *tmp = '.';
            s = tmp;
        }
        fprintf(bootstrap_FILE, "\"%s.%s\",\n",
                _pkg_prefix, pkg_name);
    }

    /* fprintf(bootstrap_FILE, "        environ = [\"OPAM_SWITCH_PREFIX\"],\n"); */

    fprintf(bootstrap_FILE, "        build_file = ");
    if (_pkg_prefix == NULL)
        fprintf(bootstrap_FILE,
                "\"@//%s/%s:BUILD.bazel\",\n",
                /* buildfile_prefix, */
                utstring_body(bzl_switch_pfx),
                pkg_name);
    else
        fprintf(bootstrap_FILE,
                "\"@//%s/%s/%s:BUILD.bazel\",\n",
                /* buildfile_prefix, */
                utstring_body(bzl_switch_pfx),
                _pkg_prefix, pkg_name);

    fprintf(bootstrap_FILE, "        path       = ");
    if (_pkg_prefix == NULL)
        fprintf(bootstrap_FILE, "\"%s\",\n",
                /* opam_switch_prefix, */
                pkg_name);
    else
        fprintf(bootstrap_FILE, "\"%s/%s\",\n",
                /* opam_switch_prefix, */
                _pkg_prefix, pkg_name);

    /* **************************************************************** */
    /*  now subpackages */
    /* **************************************************************** */

    if ((strncmp(_pkg->name, "threads", 7) == 0)
        && strlen(_pkg->name) == 7) {
        ; /* special case: skip threads subpkgs */
    } else {
        int subpkg_ct = obzl_meta_package_subpkg_count(_pkg);
        if (subpkg_ct > 0) {
            fprintf(bootstrap_FILE, "        subpackages = {\n");
            emit_new_local_subpkg_entries(bootstrap_FILE,
                                          _pkg,
                                          NULL,
                                          _pkg_prefix,
                                          _pkg->name); /* filedeps_path */
            fprintf(bootstrap_FILE, "        }\n");
        }
    }
    /* **************************************************************** */
    fprintf(bootstrap_FILE, "    )\n\n");
}

void emit_local_repo_decl(FILE *bootstrap_FILE,
                          struct obzl_meta_package *_pkg)
{
    log_debug("emit_local_repo_decl %s", _pkg->name);

    /* first emit for root pkg */
    /* then iterate over subpkgs */

    char *pkg_name = _pkg->name; // obzl_meta_package_name(_pkg);

    fprintf(bootstrap_FILE, "    native.local_repository(\n");
    fprintf(bootstrap_FILE, "        name       = ");

    char *_pkg_prefix = NULL;
    // ????
    if (_pkg_prefix == NULL)
        fprintf(bootstrap_FILE, "\"%s\",\n", pkg_name);
    else {
        char *s = _pkg_prefix;
        char *tmp;
        while(s) {
            tmp = strchr(s, '/');
            if (tmp == NULL) break;
            *tmp = '.';
            s = tmp;
        }
        fprintf(bootstrap_FILE, "\"%s.%s\",\n",
                _pkg_prefix, pkg_name);
    }

    fprintf(bootstrap_FILE, "        path       = ");
    /* if (_pkg_prefix == NULL) */
    fprintf(bootstrap_FILE, "\"%s/%s\",\n",
            utstring_body(bzl_switch_pfx),
            pkg_name);
    /* else */
    /*     fprintf(bootstrap_FILE, "\"%s/%s\",\n", */
    /*             /\* opam_switch_prefix, *\/ */
    /*             _pkg_prefix, pkg_name); */

    /* **************************************************************** */
    /*  now subpackages ???????????????? */
    /* **************************************************************** */

    /* if ((strncmp(_pkg->name, "threads", 7) == 0) */
    /*     && strlen(_pkg->name) == 7) { */
    /*     ; /\* special case: skip threads subpkgs *\/ */
    /* } else { */
    /*     int subpkg_ct = obzl_meta_package_subpkg_count(_pkg); */
    /*     if (subpkg_ct > 0) { */
    /*         fprintf(bootstrap_FILE, "        subpackages = {\n"); */
    /*         emit_new_local_subpkg_entries(bootstrap_FILE, */
    /*                                       _pkg, */
    /*                                       NULL, */
    /*                                       _pkg_prefix, */
    /*                                       _pkg->name); /\* filedeps_path *\/ */
    /*         fprintf(bootstrap_FILE, "        }\n"); */
    /*     } */
    /* } */
    /* **************************************************************** */
    fprintf(bootstrap_FILE, "    )\n\n");
}

/* **************************************************************** */
void emit_bazel_hdr(FILE* ostream)
//, int level, char *repo, char *pkg_prefix, obzl_meta_package *_pkg)
{
    fprintf(ostream,
            "load(\"@rules_ocaml//build:rules.bzl\", \"ocaml_import\")\n");
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
                          /* char *_repo, */
                          /* char *_pkg_path, */
                          char *_pkg_prefix,
                          char *_pkg_name,
                          /* for constructing import label: */
                          char *_filedeps_path, /* _lib */
                          /* char *_subpkg_dir, */
                          obzl_meta_entries *_entries,
                          char *property, /* = 'archive' or 'plugin' */
                          obzl_meta_package *_pkg)
{
    log_debug("EMIT_BAZEL_ATTRIBUTE _pkg_name: '%s'; prop: '%s'; filedeps path: '%s'",
              _pkg_name, property, _filedeps_path);
    log_debug("  pkg_prefix: %s", _pkg_prefix);

    static UT_string *archive_srcfile; // FIXME: dealloc?

    /* obzl_meta_property *_prop = obzl_meta_entries_property(_entries,property); */
    /* if (strncmp(_pkg_name, "ppx_sexp_conv", 13) == 0) { */
    /*     char *_prop_name = obzl_meta_property_name(_prop); */
    /*     char *_prop_val = obzl_meta_property_value(_prop); */

    /*     log_debug("prop: '%s' == '%s'", */
    /*               _prop_name, _prop_val); */
    /*     log_debug("DUMP_PROP"); */
    /*     dump_property(0, _prop); */
    /* } */

    /* log_debug("    _pkg_path: %s", _pkg_path); */
    /*
      NOTE: archive/plugin location is relative to the 'directory' property!
      which may be empty, even for subpackages (e.g. opam-lib subpackages)
     */

    /* char *directory = NULL; */
    /* bool stdlib = false; */
    /* struct obzl_meta_property *directory_prop = obzl_meta_entries_property(_entries, "directory"); */
    /* log_debug("DIRECTORY: %s", directory); */
    /* log_debug(" PROP: %s", property); */

    if (debug)
        log_debug("## _filedeps_path: '%s'\n", _filedeps_path);

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

    /* NB: 'property' will be 'archive' or 'plugin' (?) */
    if (settings_ct > 1) {
        fprintf(ostream, "%*s%s = select({\n", level*spfactor, sp, property);
    } else {
        fprintf(ostream, "%*s%s = [\n", level*spfactor, sp, property);
    }

    UT_string *condition_name;  /* i.e. flag */
    utstring_new(condition_name);

    obzl_meta_setting *setting = NULL;

    obzl_meta_values *vals;
    obzl_meta_value *archive_name = NULL;

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        /* log_debug("setting[%d]", i+1); */
        /* dump_setting(0, setting); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);

        /* skip any setting with deprecated flag, e.g. 'vm' */
        if (obzl_meta_flags_deprecated(flags)) {
            /* log_debug("OMIT attribute with DEPRECATED flag"); */
            continue;
        }

        if (flags != NULL) register_flags(flags);

        /* if (g_ppx_pkg) {        /\* set by opam_bootstrap.handle_lib_meta *\/ */
        /* } */

        bool has_conditions = false;
        if (flags == NULL)
            utstring_printf(condition_name, "//conditions:default");
        else
            has_conditions = obzl_meta_flags_to_selection_label(flags, condition_name);

        if (!has_conditions) {
            goto next;          /* continue does not seem to exit loop */
        }

        /* char *condition_comment = obzl_meta_flags_to_comment(flags); */
        /* log_debug("condition_comment: %s", condition_comment); */

        /* FIXME: multiple settings means multiple flags; decide how
           to handle for deps */
        // construct a select expression on the flags
        if (settings_ct > 1) {
            fprintf(ostream, "        \"%s\"%-4s",
                    utstring_body(condition_name), ": [\n");
            /* condition_comment, /\* FIXME: name the condition *\/ */
        }

        /* now we handle UPDATE settings */
        vals = resolve_setting_values(setting, flags, settings);
        log_debug("setting values:");
        /* dump_values(0, vals); */

        /* now we construct a bazel label for each value */
        UT_string *label;
        utstring_new(label);

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            archive_name = obzl_meta_values_nth(vals, j);
            log_info("prop[%d] '%s' == '%s'",
                     j, property, (char*)*archive_name);

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
            /* utstring_printf(label, "//:%s", _filedeps_path); /\* e.g. _lib *\/ */
            /* log_debug("21 _filedeps_path: %s", _filedeps_path); */
            /* log_debug("22 _pkg_prefix: %d: %s", _pkg_prefix == NULL, _pkg_prefix); */
            int rc;
            if (_filedeps_path == NULL)
                rc = 1;
            else
                rc = strncmp("ocaml", _filedeps_path, 5);

            /* emit 'archive' attr targets */
            if (rc == 0)    /* i.e. 'ocaml', 'ocaml/ocamldoc' */
                 /* special cases: unix, dynlink, etc. */
                if (strlen(_filedeps_path) == 5) {
                    utstring_printf(label, "@%s//lib:%s",
                                    _filedeps_path, *archive_name);
                } else {
                    rc = strncmp("ocaml-compiler-libs", _filedeps_path, 19);
                    if (rc == 0)
                        /* utstring_printf(label, "ZZZZ@%s:%s", */
                        /*                 _filedeps_path, *archive_name); */
                        utstring_printf(label, ":%s",
                                        *archive_name);
                    else {
                        utstring_printf(label, ":%s",
                                        *archive_name);
                        utstring_printf(label, "ERROR@%s:%s",
                                        _filedeps_path, *archive_name);

                        /* ocaml-syntax-shims, ocamlbuild, etc. */
                    }
                }
            else {
                /* we're constructing archive attribute labels
                   _pkg_name is the name of the import target, not the arch
                */
                if (_pkg_prefix == NULL) {
                    utstring_printf(label,
                                    "@%s//:%s", // filedeps path: %s",
                                    /* _pkg_name, */
                                    _filedeps_path,
                                    *archive_name);
                } else {
                    char *start = strchr(_pkg_prefix, '/');
                    int repo_len = start - (char*)_pkg_prefix;
                    if (start == NULL) {
                        utstring_printf(label,
                                        "@%.*s//%s:%s", // || PKG_pfx: %s",
                                        repo_len,
                                        _pkg_prefix,
                                        _pkg_name,
                                        *archive_name);
                                        /* _pkg_prefix); */
                    } else {
                        start++;
                        utstring_printf(label,
                                        "@%.*s//%s/%s:%s", // || PKG_pfx: %s",
                                        repo_len,
                                        _pkg_prefix,
                                        (char*)start,
                                        _pkg_name,
                                        *archive_name);
                                        /* _pkg_prefix); */
                    }

                        /* fprintf(ostream, "%*s\"@%.*s//%s\",\n", */
                        /*         (1+level)*spfactor, sp, */
                        /*         repo_len, */
                        /*         *v, */
                        /*         start+1); */

                }
                /* char *s = (char*)_filedeps_path; */
                /* char *tmp; */
                /* while(s) { */
                /*     tmp = strchr(s, '/'); */
                /*     if (tmp == NULL) break; */
                /*     *tmp = '.'; */
                /*     s = tmp; */
                /* } */
                /* char *start = strrchr(_filedeps_path, '.'); */

                /* if (start == NULL) */
                /*     utstring_printf(label, */
                /*                     "X@%s//:%s", */
                /*                     _filedeps_path, *archive_name); */
                /* else { */
                /*     *start++ = '\0'; // split on '.' */
                /*     utstring_printf(label, */
                /*                     "Y@%s//%s:%s", */
                /*                     _filedeps_path, */
                /*                     start, */
                /*                     *archive_name); */
                /* } */
            }

            /* utstring_printf(label, "@rules_ocaml//cfg/:%s/%s", */
            /*                 _filedeps_path, *archive_name); */
            /* log_debug("label 2: %s", utstring_body(label)); */

            // FIXME: verify existence using access()?
            if (settings_ct > 1) {
                    fprintf(ostream, "            \"%s\",\n", utstring_body(label));
            } else {
                    fprintf(ostream, "\"%s\",\n", utstring_body(label));
            }

            /* if v is .cmxa, then add .a too? */
            /* int cmxapos = utstring_findR(label, -1, "cmxa", 4); */
            /* if (cmxapos > 0) { */
            /*     char *cmxa_a = strdup(utstring_body(label)); */
            /*     cmxa_a[utstring_len(label) - 4] = 'a'; */
            /*     cmxa_a[utstring_len(label) - 3] = '\0'; */

            /*     // stdlib-shims is a special case, with empty */
            /*     // stdlib_shims.cmxa and missing stdlib_shims.a; see */
            /*     // https://github.com/ocaml/ocaml/pull/9011 */
            /*     // so we need to test existence */
            /*     utstring_renew(archive_srcfile); */
            /*     // drop leading ':' from basename */
            /*     char * bn = basename(cmxa_a); */

            /*     /\* utstring_printf(archive_srcfile, "%s/%s", *\/ */
            /*     /\*                 _pkg->path, bn); *\/ */

            /*     /\* if (access(utstring_body(archive_srcfile), F_OK) == 0) *\/ */
            /*     /\*     fprintf(ostream, "            \"%s\",\n", cmxa_a); *\/ */
            /*     /\* else *\/ */
            /*     /\*     if (debug) { *\/ */
            /*     /\*         log_debug("## missing %s\n", *\/ */
            /*     /\*                   utstring_body(archive_srcfile)); *\/ */
            /*     /\*     } *\/ */
            /* } */
            /* if (flags != NULL) /\* skip mt, mt_vm, mt_posix *\/ */
            /*     fprintf(ostream, "%s", "        ],\n"); */
        }
        utstring_free(label);
        if (settings_ct > 1) {
            /* fprintf(ostream, "\n"); // , (1+level)*spfactor, sp); */
            fprintf(ostream, "%*s],\n", (1+level)*spfactor, sp);
        }
        /* free(condition_comment); */
    next:
        ;
    }
    utstring_free(condition_name);
    if (settings_ct > 1)
        fprintf(ostream, "%*s}),\n", level*spfactor, sp);
    else
        fprintf(ostream, "%*s],\n", level*spfactor, sp);
}

void emit_bazel_cc_imports(FILE* ostream,
                           int level, /* indentation control */
                           char *_pkg_prefix,
                           char *_pkg_name,
                           char *_filedeps_path, /* _lib */
                           obzl_meta_entries *_entries,
                           obzl_meta_package *_pkg)
{
    printf("emit_bazel_cc_imports\n");
    char *dname = dirname(utstring_body(build_bazel_file));
    printf("dir: %s\n", dname);

    errno = 0;
    DIR *d = opendir(dname);
    if (d == NULL) {
        fprintf(stderr,
                "ERROR: bad opendir: %s\n", strerror(errno));
        return;
    }

    bool wrote_loader = false;

    /* RASH ASSUMPTION: only one stublib per directory */
    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if ((direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK)) {
            if (fnmatch("*stubs.a", direntry->d_name, FNM_CASEFOLD) == 0) {
                /* printf("FOUND STUBLIB: %s\n", direntry->d_name); */

                fprintf(ostream, "cc_import(\n");
                fprintf(ostream, "    name            = \"_%s\",\n",
                        direntry->d_name);
                fprintf(ostream, "    static_library = \"%s\",\n",
                        direntry->d_name);
                fprintf(ostream, ")\n");
            } else {
                if (fnmatch("*stubs.so", direntry->d_name, FNM_CASEFOLD) == 0) {
                    printf("FOUND SO LIB: %s\n", direntry->d_name);
            }
                /* printf("skipping %s\n", direntry->d_name); */
            }
        }
    }
    closedir(d);
}

void emit_bazel_stublibs_attr(FILE* ostream,
                              int level, /* indentation control */
                              char *_pkg_prefix,
                              char *_pkg_name,
                              char *_filedeps_path, /* _lib */
                              obzl_meta_entries *_entries,
                              obzl_meta_package *_pkg)
{
    printf("emit_bazel_stublibs_attr\n");
    char *dname = dirname(utstring_body(build_bazel_file));
    printf("dir: %s\n", dname);

    errno = 0;
    DIR *d = opendir(dname);
    if (d == NULL) {
        fprintf(stderr,
                "ERROR: bad opendir: %s\n", strerror(errno));
        return;
    }

    bool wrote_loader = false;

    /* RASH ASSUMPTION: only one stublib per directory */
    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if ((direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK)) {
            if (fnmatch("*stubs.a", direntry->d_name, FNM_CASEFOLD) == 0) {

                fprintf(ostream, "%*sstublibs   = [\":_%s\"],\n",
                        level*spfactor, sp, direntry->d_name);

                /* fprintf(ostream, "cc_import(\n"); */
                /* fprintf(ostream, "    name    = \"%s_stubs\",\n", */
                /*         _pkg_name); */
                /* fprintf(ostream, "    archive = \"%s\",\n", */
                /*         direntry->d_name); */
                /* fprintf(ostream, ")\n"); */
            } else {
                if (fnmatch("*stubs.so", direntry->d_name, FNM_CASEFOLD) == 0) {
                    printf("FOUND SO LIB: %s\n", direntry->d_name);
            }
                /* printf("skipping %s\n", direntry->d_name); */
            }
        }
    }
    closedir(d);
}

void emit_bazel_archive_attr(FILE* ostream,
                             int level, /* indentation control */
                             /* char *_repo, */
                             /* char *_pkg_path, */
                             char *_pkg_prefix,
                             char *_pkg_name,
                             /* for constructing import label: */
                             char *_filedeps_path, /* _lib */
                             /* char *_subpkg_dir, */
                             obzl_meta_entries *_entries,
                             char *property, /* = 'archive' or 'plugin' */
                             obzl_meta_package *_pkg)
{
    log_debug("EMIT_BAZEL_ARCHIVE_ATTR _pkg_name: '%s'; prop: '%s'; filedeps path: '%s'",
              _pkg_name, property, _filedeps_path);
    log_debug("  pkg_prefix: %s", _pkg_prefix);

    static UT_string *archive_srcfile; // FIXME: dealloc?

    if (debug)
        log_debug("## _filedeps_path: '%s'\n", _filedeps_path);

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
    UT_string *cmtag;  /* cma or cmxa */
    utstring_new(cmtag);

    obzl_meta_setting *setting = NULL;

    obzl_meta_values *vals;
    obzl_meta_value *archive_name = NULL;

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        /* log_debug("setting[%d]", i+1); */
        /* dump_setting(0, setting); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);

        /* skip any setting with deprecated flag, e.g. 'vm' */
        if (obzl_meta_flags_deprecated(flags)) {
            /* log_debug("OMIT attribute with DEPRECATED flag"); */
            continue;
        }

        if (flags != NULL) register_flags(flags);

        /* if (g_ppx_pkg) {        /\* set by opam_bootstrap.handle_lib_meta *\/ */
        /* } */

        bool has_conditions = false;
        if (flags == NULL)
            utstring_printf(cmtag, "//conditions:default");
        else
            /* updates cmtag */
            has_conditions = obzl_meta_flags_to_cmtag(flags, cmtag);

        if (!has_conditions) {
            goto next;          /* continue does not seem to exit loop */
        }
        if (settings_ct > 1) {
            fprintf(ostream, "%*s%s", level*spfactor, sp,
                    utstring_body(cmtag));
        }

        /* now we handle UPDATE settings */
        vals = resolve_setting_values(setting, flags, settings);
        log_debug("setting values:");
        /* dump_values(0, vals); */

        /* now we construct a bazel label for each value */
        UT_string *label;
        utstring_new(label);

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            archive_name = obzl_meta_values_nth(vals, j);
            log_info("prop[%d] '%s' == '%s'",
                     j, property, (char*)*archive_name);
            utstring_clear(label);
            if (_pkg_prefix == NULL) {
                utstring_printf(label,
                                "%s",
                                /* "@%s//:%s", // filedeps path: %s", */
                                /* /\* _pkg_name, *\/ */
                                /* _filedeps_path, */
                                *archive_name);
            } else {
                char *start = strchr(_pkg_prefix, '/');
                int repo_len = start - (char*)_pkg_prefix;
                if (start == NULL) {
                    utstring_printf(label,
                                    "%s",
                                    /* "@%.*s//%s:%s", // || PKG_pfx: %s", */
                                    /* repo_len, */
                                    /* _pkg_prefix, */
                                    /* _pkg_name, */
                                    *archive_name);
                    /* _pkg_prefix); */
                } else {
                    start++;
                    utstring_printf(label,
                                    "%s",
                                    /* "@%.*s//%s/%s:%s", */
                                    /* repo_len, */
                                    /* _pkg_prefix, */
                                    /* (char*)start, */
                                    /* _pkg_name, */
                                    *archive_name);
                    /* _pkg_prefix); */
                }

                /* fprintf(ostream, "%*s\"@%.*s//%s\",\n", */
                /*         (1+level)*spfactor, sp, */
                /*         repo_len, */
                /*         *v, */
                /*         start+1); */

            }
            // FIXME: verify existence using access()?
            int indent = 3;
            if (strncmp(utstring_body(cmtag), "cmxa", 4) == 0)
                indent--;
            if (settings_ct > 1) {
                    fprintf(ostream, "%*s = [\"%s\"],\n",
                                        /* "@%.*s//%s/%s:%s", */
                            indent, sp, utstring_body(label));
            } else {
                    fprintf(ostream, "%*s = [\"%s\"],\n",
                            indent, sp, utstring_body(label));
            }
        }
        utstring_free(label);
    next:
        ;
    }
}

void emit_bazel_cmxs_attr(FILE* ostream,
                          int level, /* indentation control */
                          /* char *_repo, */
                          /* char *_pkg_path, */
                          char *_pkg_prefix,
                          char *_pkg_name,
                          /* for constructing import label: */
                          char *_filedeps_path, /* _lib */
                          /* char *_subpkg_dir, */
                          obzl_meta_entries *_entries,
                          char *property, /* = 'archive' or 'plugin' */
                          obzl_meta_package *_pkg)
{
    log_debug("EMIT_BAZEL_CMXS_ATTR _pkg_name: '%s'; prop: '%s'; filedeps path: '%s'",
              _pkg_name, property, _filedeps_path);
    log_debug("  pkg_prefix: %s", _pkg_prefix);

    static UT_string *archive_srcfile; // FIXME: dealloc?

    /* obzl_meta_property *_prop = obzl_meta_entries_property(_entries,property); */
    /* if (strncmp(_pkg_name, "ppx_sexp_conv", 13) == 0) { */
    /*     char *_prop_name = obzl_meta_property_name(_prop); */
    /*     char *_prop_val = obzl_meta_property_value(_prop); */

    /*     log_debug("prop: '%s' == '%s'", */
    /*               _prop_name, _prop_val); */
    /*     log_debug("DUMP_PROP"); */
    /*     dump_property(0, _prop); */
    /* } */

    /* log_debug("    _pkg_path: %s", _pkg_path); */
    /*
      NOTE: archive/plugin location is relative to the 'directory' property!
      which may be empty, even for subpackages (e.g. opam-lib subpackages)
     */

    /* char *directory = NULL; */
    /* bool stdlib = false; */
    /* struct obzl_meta_property *directory_prop = obzl_meta_entries_property(_entries, "directory"); */
    /* log_debug("DIRECTORY: %s", directory); */
    /* log_debug(" PROP: %s", property); */

    if (debug)
        log_debug("## _filedeps_path: '%s'\n", _filedeps_path);

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

    /* NB: 'property' will be 'archive' or 'plugin' (?) */
    /* if (settings_ct > 1) { */
    /*     fprintf(ostream, "%*s%s = select({\n", level*spfactor, sp, property); */
    /* } else { */
    /*     fprintf(ostream, "%*s%s = [\n", level*spfactor, sp, property); */
    /* } */

    fprintf(ostream, "%*scmxs", level*spfactor, sp);

    UT_string *cmtag;  /* i.e. flag */
    utstring_new(cmtag);

    obzl_meta_setting *setting = NULL;

    obzl_meta_values *vals;
    obzl_meta_value *archive_name = NULL;

    for (int i = 0; i < settings_ct; i++) {
        setting = obzl_meta_settings_nth(settings, i);
        /* log_debug("setting[%d]", i+1); */
        /* dump_setting(0, setting); */

        obzl_meta_flags *flags = obzl_meta_setting_flags(setting);

        /* skip any setting with deprecated flag, e.g. 'vm' */
        if (obzl_meta_flags_deprecated(flags)) {
            /* log_debug("OMIT attribute with DEPRECATED flag"); */
            continue;
        }

        if (flags != NULL) register_flags(flags);

        /* if (g_ppx_pkg) {        /\* set by opam_bootstrap.handle_lib_meta *\/ */
        /* } */

        bool has_conditions = false;
        if (flags == NULL)
            utstring_printf(cmtag, "//conditions:default");
        else
            has_conditions = obzl_meta_flags_to_cmtag(flags, cmtag);
            /* has_conditions = obzl_meta_flags_to_selection_label(flags, cmtag); */

        if (!has_conditions) {
            goto next;          /* continue does not seem to exit loop */
        }

        /* char *condition_comment = obzl_meta_flags_to_comment(flags); */
        /* log_debug("condition_comment: %s", condition_comment); */

        /* FIXME: multiple settings means multiple flags; decide how
           to handle for deps */
        // construct a select expression on the flags
        /* if (settings_ct > 1) { */
        /*     fprintf(ostream, "        \"%s\"%-4s", */
        /*             utstring_body(cmtag), ": [\n"); */
        /*     /\* condition_comment, /\\* FIXME: name the condition *\\/ *\/ */
        /* } */

        /* now we handle UPDATE settings */
        vals = resolve_setting_values(setting, flags, settings);
        log_debug("setting values:");
        /* dump_values(0, vals); */

        /* now we construct a bazel label for each value */
        UT_string *label;
        utstring_new(label);

        for (int j = 0; j < obzl_meta_values_count(vals); j++) {
            archive_name = obzl_meta_values_nth(vals, j);
            log_info("prop[%d] '%s' == '%s'",
                     j, property, (char*)*archive_name);

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
            /* utstring_printf(label, "//:%s", _filedeps_path); /\* e.g. _lib *\/ */
            /* log_debug("21 _filedeps_path: %s", _filedeps_path); */
            /* log_debug("22 _pkg_prefix: %d: %s", _pkg_prefix == NULL, _pkg_prefix); */
            /* int rc; */
            /* if (_filedeps_path == NULL) */
            /*     rc = 1; */
            /* else */
            /*     rc = strncmp("ocaml", _filedeps_path, 5); */

            /* /\* emit 'archive' attr targets *\/ */
            /* if (rc == 0)    /\* i.e. 'ocaml', 'ocaml/ocamldoc' *\/ */
            /*      /\* special cases: unix, dynlink, etc. *\/ */
            /*     if (strlen(_filedeps_path) == 5) { */
            /*         utstring_printf(label, "@%s//lib:%s", */
            /*                         _filedeps_path, *archive_name); */
            /*     } else { */
            /*         rc = strncmp("ocaml-compiler-libs", _filedeps_path, 19); */
            /*         if (rc == 0) */
            /*             /\* utstring_printf(label, "ZZZZ@%s:%s", *\/ */
            /*             /\*                 _filedeps_path, *archive_name); *\/ */
            /*             utstring_printf(label, ":%s", */
            /*                             *archive_name); */
            /*         else { */
            /*             utstring_printf(label, ":%s", */
            /*                             *archive_name); */
            /*             utstring_printf(label, "ERROR@%s:%s", */
            /*                             _filedeps_path, *archive_name); */

            /*             /\* ocaml-syntax-shims, ocamlbuild, etc. *\/ */
            /*         } */
            /*     } */
            /* else { */
                /* we're constructing archive attribute labels
                   _pkg_name is the name of the import target, not the arch
                */
                if (_pkg_prefix == NULL) {
                    utstring_printf(label,
                                    "%s",
                                    /* "@%s//:%s", // filedeps path: %s", */
                                    /* /\* _pkg_name, *\/ */
                                    /* _filedeps_path, */
                                    *archive_name);
                } else {
                    char *start = strchr(_pkg_prefix, '/');
                    int repo_len = start - (char*)_pkg_prefix;
                    if (start == NULL) {
                        utstring_printf(label,
                                        "%s",
                                        /* "@%.*s//%s:%s", // || PKG_pfx: %s", */
                                        /* repo_len, */
                                        /* _pkg_prefix, */
                                        /* _pkg_name, */
                                        *archive_name);
                                        /* _pkg_prefix); */
                    } else {
                        start++;
                        utstring_printf(label,
                                        "%s",
                                        /* "@%.*s//%s/%s:%s", // || PKG_pfx: %s", */
                                        /* repo_len, */
                                        /* _pkg_prefix, */
                                        /* (char*)start, */
                                        /* _pkg_name, */
                                        *archive_name);
                                        /* _pkg_prefix); */
                    }

                        /* fprintf(ostream, "%*s\"@%.*s//%s\",\n", */
                        /*         (1+level)*spfactor, sp, */
                        /*         repo_len, */
                        /*         *v, */
                        /*         start+1); */

                }
                /* char *s = (char*)_filedeps_path; */
                /* char *tmp; */
                /* while(s) { */
                /*     tmp = strchr(s, '/'); */
                /*     if (tmp == NULL) break; */
                /*     *tmp = '.'; */
                /*     s = tmp; */
                /* } */
                /* char *start = strrchr(_filedeps_path, '.'); */

                /* if (start == NULL) */
                /*     utstring_printf(label, */
                /*                     "X@%s//:%s", */
                /*                     _filedeps_path, *archive_name); */
                /* else { */
                /*     *start++ = '\0'; // split on '.' */
                /*     utstring_printf(label, */
                /*                     "Y@%s//%s:%s", */
                /*                     _filedeps_path, */
                /*                     start, */
                /*                     *archive_name); */
                /* } */
            /* } */

            /* utstring_printf(label, "@rules_ocaml//cfg/:%s/%s", */
            /*                 _filedeps_path, *archive_name); */
            /* log_debug("label 2: %s", utstring_body(label)); */

            // FIXME: verify existence using access()?
                if (strncmp(utstring_body(cmtag), "cmxa", 4) == 0) {
                    fprintf(ostream, " = [\"%s\"],\n", utstring_body(label));
                }
        }
        utstring_free(label);
    next:
        ;
    }
    utstring_free(cmtag);
}

/*
  FIXME: version, description really apply to whole pkg; put them in comments, not rule attributes
 */
void emit_bazel_metadatum(FILE* ostream, int level,
                          char *repo,
                          /* char *pkg, */
                          obzl_meta_entries *_entries,
                          char *_property, // META property
                          char *_attrib    // Bazel attr name
                          )
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
    /* log_debug("metadatum %s setting vals ct: %d", _property, obzl_meta_values_count(vals)); */
    /* if vals ct > 1 warn, should be only 1 */
    v = obzl_meta_values_nth(vals, 0);
    /* log_debug("vals[0]: %s", *v); */
    if (v != NULL)
        /* fprintf(stderr, "    %s = \"%s\",\n", _attrib, *v); */
        fprintf(ostream, "    %s = \"\"\"%s\"\"\",\n",
                _attrib,
                /* _property, */
                *v);
}

void emit_bazel_archive_rule(FILE* ostream,
                             int level,
                             char *_repo,
                             char *_filedeps_path, /* _lib/... */
                             /* char *_pkg_path, */
                             char *_pkg_prefix,
                             char *_pkg_name,
                             obzl_meta_entries *_entries, //)
                             /* char *_subpkg_dir) */
                             obzl_meta_package *_pkg)
{
    log_debug("EMIT_BAZEL_ARCHIVE_RULE: _filedeps_path: %s", _filedeps_path);
    /* http://projects.camlcity.org/projects/dl/findlib-1.9.1/doc/ref-html/r759.html

The variable "archive" specifies the list of archive files. These files should be given either as (1) plain names without any directory information; they are only searched in the package directory. (2) Or they have the form "+path" in which case the files are looked up relative to the standard library. (3) Or they have the form "@name/file" in which case the files are looked up in the package directory of another package. (4) Or they are given as absolute paths.

The names of the files must be separated by white space and/or commas. In the preprocessor stage, the archive files are passed as extensions to the preprocessor (camlp4) call. In the linker stage (-linkpkg), the archive files are linked. In the compiler stage, the archive files are ignored.

Note that "archive" should only be used for archive files that are intended to be included in executables or loaded into toploops. For modules loaded at runtime there is the separate variable "plugin".
     */

    /* archive flags: byte, native */
    /* pkg threads: mt, mt_vm, mt_posix */
    /* ppx: ppx_driver, -ppx_driver always go together. not a syntactic rule, but seems to be the convention */

    /* _filedeps_path: relative to opam_switch_lib
       we need to convert it to a bazel label
       e.g. ounit2/advanced => @ounit2//advanced
       we do this here because it applies to all props/subpkgs
       BUT: just use pkg_prefix and pkg name?
     */

    /* write scheme opam-resolver table */
    //FIXME: for here-switch only
    /* write_opam_resolver(_pkg_prefix, _pkg_name, _entries); */
    fprintf(ostream, "\nocaml_import(\n");
    fprintf(ostream, "    name = \"%s\",\n", _pkg_name); /* default target provides archive */

    emit_bazel_metadatum(ostream, 1,
                         host_repo,              /* @ocaml */
                         /* _filedeps_path, */
                         _entries, "version", "version");
    emit_bazel_metadatum(ostream, 1,
                         host_repo, // "@ocaml",
                         /* _filedeps_path, */
                         _entries, "description", "doc");

    emit_bazel_archive_attr(ostream, 1,
                            /* _repo, // host_repo, */
                            /* _pkg_path, */
                            _pkg_prefix,
                            _pkg_name,
                            /* for constructing import label: */
                            _filedeps_path,
                            /* _subpkg_dir, */
                            _entries,
                            "archive",
                            _pkg);

    fprintf(ostream, "    cmi      = glob([\"*.cmi\"]),\n");
    fprintf(ostream, "    cmo      = glob([\"*.cmo\"]),\n");
    fprintf(ostream, "    cmx      = glob([\"*.cmx\"]),\n");
    fprintf(ostream, "    ofiles   = glob([\"*.o\"]),\n");
    fprintf(ostream, "    afiles   = glob([\"*.a\"], exclude=[\"*_stubs.a\"]),\n");
    fprintf(ostream, "    cmt      = glob([\"*.cmt\"]),\n");
    fprintf(ostream, "    cmti     = glob([\"*.cmti\"]),\n");
    /* fprintf(ostream, "    cclibs   = glob([\"*_stubs.a\"]),\n"); */
    fprintf(ostream, "    vmlibs   = glob([\"dll*.so\"]),\n");
    fprintf(ostream, "    srcs     = glob([\"*.ml\", \"*.mli\"]),\n");

    emit_bazel_stublibs_attr(ostream, 1,
                             _pkg_prefix,
                             _pkg_name,
                             _filedeps_path,
                             _entries,
                             _pkg);

    /* fprintf(ostream, "    all      = glob([\"*.\"]),\n"); */
    emit_bazel_deps_attribute(ostream, 1, host_repo, "lib", _pkg_name, _entries);

    emit_bazel_ppx_codeps(ostream, 1, host_repo, _pkg_name, "lib", _entries);
    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
}

void emit_bazel_plugin_rule(FILE* ostream, int level,
                            char *_repo,
                            char *_filedeps_path,
                            /* char *_pkg_path, */
                            char *_pkg_prefix,
                            char *_pkg_name,
                            obzl_meta_entries *_entries, //)
                            obzl_meta_package *_pkg)
                            /* char *_subpkg_dir) */
{
    log_debug("EMIT_BAZEL_PLUGIN_RULE: _filedeps_path: %s", _filedeps_path);
    log_debug("pkg_pfx: %s", _pkg_prefix);
    log_debug("_pkg_name: %s", _pkg_name);
    log_debug("pkg_name: %s", _pkg->name);
    //FIXME: for here-switch only
    /* write_opam_resolver(_pkg_prefix, _pkg_name, _entries); */

    fprintf(ostream, "\nocaml_import(\n");
    fprintf(ostream, "    name = \"plugin\",\n");
    /* log_debug("PDUMPP %s", _pkg_name); */
    /* dump_entries(0, _entries); */

    /* emit_bazel_attribute(ostream, 1, */
    /*                      /\* _repo, *\/ */
    /*                      /\* _pkg_path, *\/ */
    /*                      _pkg_prefix, */
    /*                      _pkg_name, */
    /*                      _filedeps_path, */
    /*                      /\* _subpkg_dir, *\/ */
    /*                      _entries, */
    /*                      "plugin", //); */
    /*                      _pkg); */

    /* emit_bazel_archive_attr(ostream, 1, */
    /*                         /\* _repo, // host_repo, *\/ */
    /*                         /\* _pkg_path, *\/ */
    /*                         _pkg_prefix, */
    /*                         _pkg_name, */
    /*                         /\* for constructing import label: *\/ */
    /*                         _filedeps_path, */
    /*                         /\* _subpkg_dir, *\/ */
    /*                         _entries, */
    /*                         "plugin", */
    /*                         _pkg); */
    /* fprintf(ostream, "    cmxa = glob([\"*.cmxa\", \"*.a\"]),\n"); */
    /* fprintf(ostream, "    cma  = glob([\"*.cma\"]),\n"); */

    emit_bazel_cmxs_attr(ostream, 1,
                            /* _repo, // host_repo, */
                            /* _pkg_path, */
                            _pkg_prefix,
                            _pkg_name,
                            /* for constructing import label: */
                            _filedeps_path,
                            /* _subpkg_dir, */
                            _entries,
                            "plugin",
                            _pkg);

    /* fprintf(ostream, "    cmxs = glob([\"*.cmxs\"]),\n"); */

    /* fprintf(ostream, "    cmi    = glob([\"*.cmi\"]),\n"); */
    /* fprintf(ostream, "    cmo    = glob([\"*.cmo\"]),\n"); */
    /* fprintf(ostream, "    cmx    = glob([\"*.cmx\"]),\n"); */
    /* fprintf(ostream, "    ofiles = glob([\"*.o\"]),\n"); */
    /* fprintf(ostream, "    afiles = glob([\"*.a\"]),\n"); */
    /* fprintf(ostream, "    cmt    = glob([\"*.cmt\"]),\n"); */
    /* fprintf(ostream, "    cmti   = glob([\"*.cmti\"]),\n"); */
    /* fprintf(ostream, "    srcs   = glob([\"*.ml\", \"*.mli\"]),\n"); */

    emit_bazel_deps_attribute(ostream, 1, host_repo, "lib", _pkg_name, _entries);
    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
}

/*
  special casing: META with directory starting with '^'
  bigarray, dynlink, str, threads, unix
  also: num, raw-spacetime, stdlib, ocamldoc?
  num is a special case. was part of core distrib, now it's a pkg

  we emit an alias to redirect from standalone to @ocaml
  e.g. @bigarray//:bigarry => @ocaml//bigarray

  this is an OPAM legacy thing in case people depend on these pkgs. we
  don't really need to do this since legacy-to-obazl conversion should
  map legacy pkg names correctly. But we do it anyway just in case.
 */
bool emit_special_case_rule(FILE* ostream,
                            obzl_meta_package *_pkg)
{
    log_trace("emit_special_case_rule pkg: %s", _pkg->name);

    if ((strncmp(_pkg->name, "bigarray", 8) == 0)
        && strlen(_pkg->name) == 8) {
        log_trace("emit_special_case_rule: bigarrray");

        fprintf(ostream, "alias(\n"
                "    name = \"bigarray\",\n"
                "    actual = \"@ocaml//bigarray\",\n"
                "    visibility = [\"//visibility:public\"]\n"
                ")\n");
        return true;
    }

    if ((strncmp(_pkg->name, "dynlink", 7) == 0)
        && strlen(_pkg->name) == 7) {
        log_trace("emit_special_case_rule: dynlink");

        fprintf(ostream, "alias(\n"
                "    name = \"dynlink\",\n"
                "    actual = \"@ocaml//dynlink\",\n"
                "    visibility = [\"//visibility:public\"]\n"
                ")\n");
        return true;
    }

    /* WARNING: some META files list 'compiler-libs' as a requirement,
       but the META file for compiler-libs itself lists no content for
       that package, only subpackages compiler-libs.common etc. are
       populated. The official docs indirectly suggest that the base
       pkg is compiler-libs.common, so that's what we do here.
     */

    if ((strncmp(_pkg->name, "compiler-libs", 13) == 0)
        && strlen(_pkg->name) == 13) {
        log_trace("emit_special_case_rule: compiler-libs");

        fprintf(ostream, "alias(\n"
                "    name = \"compiler-libs\",\n"
                "    actual = \"@ocaml//compiler-libs/common\",\n"
                "    visibility = [\"//visibility:public\"]\n"
                ")\n");
        return true;
    }

    /* if ((strncmp(_pkg->name, "bytecomp", 8) == 0) */
    /*     && strlen(_pkg->name) == 8) { */
    /*     log_trace("emit_special_case_rule: bytecomp"); */
    /*     log_trace("emit_special_case_rule: pkg->dir: %s", _pkg->directory); */
    /*     log_trace("emit_special_case_rule: pkg->path: %s", _pkg->path); */
    /* /\* char *name; *\/ */
    /* /\* char *path; *\/ */
    /* /\* char *directory;            /\\* subdir *\\/ *\/ */
    /* /\* char *metafile; *\/ */
    /* /\* obzl_meta_entries *entries;          /\\* list of struct obzl_meta_entry *\\/ *\/ */


    /*     fprintf(ostream, "alias(\n" */
    /*             "    name = \"bytecomp\",\n" */
    /*             "    actual = \"@ocaml//compiler-libs/bytecomp\",\n" */
    /*             "    visibility = [\"//visibility:public\"]\n" */
    /*             ")\n"); */
    /*     return true; */
    /* } */

    //TODO: compiler-libs/common, bytecomp, optcomp,
    // toplevel, native-toplevel


    if ((strncmp(_pkg->name, "num", 3) == 0)
        && strlen(_pkg->name) == 3) {
        log_trace("emit_special_case_rule: num");

        fprintf(ostream, "alias(\n"
                "    name = \"num\",\n"
                "    actual = \"@ocaml//num/core\",\n"
                "    visibility = [\"//visibility:public\"]\n"
                ")\n");
        return true;
    }

    if ((strncmp(_pkg->name, "ocamldoc", 8) == 0)
        && strlen(_pkg->name) == 8) {
        log_trace("emit_special_case_rule: ocamldoc");

        fprintf(ostream, "alias(\n"
                "    name = \"ocamldoc\",\n"
                "    actual = \"@ocaml//ocamldoc\",\n"
                "    visibility = [\"//visibility:public\"]\n"
                ")\n");
        return true;
    }

    if ((strncmp(_pkg->name, "str", 3) == 0)
        && strlen(_pkg->name) == 3) {
        log_trace("emit_special_case_rule: str");
        /* fprintf(ostream, "xxxxxxxxxxxxxxxx"); */
        fprintf(ostream, "alias(\n"
                "    name = \"str\",\n"
                "    actual = \"@ocaml//str\",\n"
                "    visibility = [\"//visibility:public\"]\n"
                ")\n");
        return true;
    }

    if ((strncmp(_pkg->name, "threads", 7) == 0)
        && strlen(_pkg->name) == 7) {
        log_trace("emit_special_case_rule: threads");

        fprintf(ostream, "alias(\n"
                "    name = \"threads\",\n"
                "    actual = \"@ocaml//threads\",\n"
                "    visibility = [\"//visibility:public\"]\n"
                ")\n");
        return true;
    }

    if ((strncmp(_pkg->name, "unix", 4) == 0)
        && strlen(_pkg->name) == 4) {
        log_trace("emit_special_case_rule: unix");

        fprintf(ostream, "alias(\n"
                "    name = \"unix\",\n"
                "    actual = \"@ocaml//unix\",\n"
                "    visibility = [\"//visibility:public\"]\n"
                ")\n");
        return true;
    }

    return false;
}

/* **************************************************************** */
bool special_case_multiseg_dep(FILE* ostream,
                               obzl_meta_value *dep_name,
                               char *delim1)
{
    if (delim1 == NULL) {
        if (strncmp(*dep_name, "compiler-libs", 13) == 0) {
            fprintf(ostream, "%*s    \"@ocaml//compiler-libs/common\",\n",
                    (1+level)*spfactor, sp);
            return true;
        } else {
            if (strncmp(*dep_name, "threads", 13) == 0) {
                fprintf(ostream, "%*s    \"@ocaml//threads\",\n",
                        (1+level)*spfactor, sp);
                return true;
            }
        }
    } else {

        if (strncmp(*dep_name, "compiler-libs/", 14) == 0) {
            fprintf(ostream,
                    "%*s    \"@ocaml//compiler-libs/%s\",\n",
                    (1+level)*spfactor, sp, delim1+1);
            return true;
        }

        if (strncmp(*dep_name, "threads/", 8) == 0) {
            /* threads.posix, threads.vm => threads */
            fprintf(ostream, "        \"@ocaml//threads\",\n");
            /* (1+level)*spfactor, sp, delim1+1); */
            return true;
        }

        /* if (strncmp(*dep_name, "posix/", 8) == 0) { */
        /*     fprintf(ostream, "%*s\"@rules_ocaml//cfg/threads/%s\",\n", */
        /*             (1+level)*spfactor, sp, delim1+1); */
        /*     return true; */
        /* } */

    }
    return false;
}

void emit_bazel_deps_attribute(FILE* ostream, int level,
                               char *repo, char *pkg, char *pkg_name,
                               obzl_meta_entries *_entries)
/* obzl_meta_package *_pkg) */
{

    //FIXME: skip if no 'requires' or requires == ''
    /* obzl_meta_entries *entries = obzl_meta_package_entries(_pkg); */

    struct obzl_meta_property *deps_prop = NULL;

    /* char *kind = "library_kind"; */
    /* deps_prop = obzl_meta_entries_property(_entries, kind); */
    /* if ( deps_prop == NULL ) { */
    /*     printf("NO LIBRARY_KIND! %s\n", pkg_name); */
    /* } else { */
    /*     obzl_meta_value k = obzl_meta_property_value(deps_prop); */
    /*     printf("LIBRARY_KIND! %s: %s\n", pkg_name, (char*)k); */
    /* } */

    char *pname = "requires";
    deps_prop = NULL;
    deps_prop = obzl_meta_entries_property(_entries, pname);
    if ( deps_prop == NULL ) return;
    obzl_meta_value ds = obzl_meta_property_value(deps_prop);
    if (ds == NULL) return;

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
    obzl_meta_value *dep_name = NULL;

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
            has_conditions = obzl_meta_flags_to_selection_label(flags, condition_name);

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
            fprintf(ostream, "%*s\"X%s%s\": [ ## predicates: %s\n",
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
            dep_name = obzl_meta_values_nth(vals, j);
            /* log_info("property val[%d]: '%s'", j, *dep_name); */

            char *s = (char*)*dep_name;

            /* printf("DEP: %s\n", s); */

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

            /* emit 'deps' attr labels */

            if (settings_ct > 1) {
                fprintf(ostream, "%*s\"@FIXME: %s//%s\",\n",
                        (2+level)*spfactor, sp, *dep_name, pkg);
            } else {
                /* first convert pkg string */
                /* char *s = (char*)*dep_name; */
                /* char *tmp; */
                /* while(s) { */
                /*     tmp = strchr(s, '/'); */
                /*     if (tmp == NULL) break; */
                /*     *tmp = '.'; */
                /*     s = tmp; */
                /* } */
                /* then extract target segment */
                char *delim1 = strchr(*dep_name, '/');
                /* log_debug("WW *dep_name: %s; delim1: %s, null? %d\n", */
                /*         *dep_name, delim1, (delim1 == NULL)); */

                if (delim1 == NULL) {
                    if (special_case_multiseg_dep(ostream, dep_name, delim1))
                        continue;
                    else {
                        /* single-seg pkg, e.g. ptime  */

                        // handle core libs: dynlink, str, unix, etc.
                        if ((strncmp(*dep_name, "bigarray", 8) == 0)
                            && strlen(*dep_name) == 8) {
                            fprintf(ostream, "%*s\"@ocaml//bigarray\",\n",
                                    (1+level)*spfactor, sp);
                        } else {
                            if ((strncmp(*dep_name, "unix", 4) == 0)
                                && strlen(*dep_name) == 4) {
                                fprintf(ostream, "%*s\"@ocaml//unix\",\n",
                                        (1+level)*spfactor, sp);
                            } else {
                                //NOTE: we use @foo instead of @foo//:foo
                                // seems to work
                                /* fprintf(ostream, */
                                /*         "%*s\"@%s\",\n", */
                                /*         (1+level)*spfactor, sp, */
                                /*         *dep_name); */
                                fprintf(ostream,
                                        "%*s\"@%s//:%s\",\n",
                                        (1+level)*spfactor, sp,
                                        *dep_name, *dep_name);
                            }
                        }
                    }
                } else {
                    /* multi-seg pkg, e.g. lwt.unix, ptime.clock.os */
                    if (special_case_multiseg_dep(ostream, dep_name, delim1))
                        continue;
                    else {
                        int repo_len = delim1 - (char*)*dep_name;
                        fprintf(ostream, "%*s\"@%.*s//%s\",\n",
                                (1+level)*spfactor, sp,
                                repo_len,
                                *dep_name,
                                delim1+1);
                        /* (1+level)*spfactor, sp, repo, pkg, *dep_name); */
                    }
                }
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

void emit_bazel_ppx_codeps(FILE* ostream, int level,
                           char *repo,
                           char *_pkg_name,
                           char *_pkg_prefix,
                           obzl_meta_entries *_entries)
{
    /* handle both 'ppx_runtime_deps' and 'requires(-ppx_driver)'

       e.g. ppx_sexp_conv has no ppx_runtime_deps property but:
       requires(-ppx_driver) = "ppx_sexp_conv.runtime-lib"

       ppx_bin_prot has both:
       requires(ppx_driver) = "base
                        compiler-libs.common
                        ppx_bin_prot.shape-expander
                        ppxlib
                        ppxlib.ast"
       ppx_runtime_deps = "bin_prot"
       but:
       requires(-ppx_driver) = "bin_prot ppx_here.runtime-lib"
       requires(-ppx_driver,-custom_ppx) += "ppx_deriving"

       and ppx_expect is even worse; in addition to
       requires(ppx_driver) and requires(-ppx_driver), it has
       ppx(-ppx_driver,-custom_ppx) = "./ppx.exe --as-ppx"

       what does it all mean?  who knows.

       we interpret this to mean: if you build (with ocamlfind) with
       predicate -ppx_driver (= NOT ppx_driver), it means you want to
       use the pkg as a regular dependency, not as part of a ppx
       executable. so in that case, you have to add the ppx_codep as a
       normal dep of the file you're compiling. If you were to use
       predicate ppx_driver, it would mean you are using ppx_sexp_conv
       as part of a ppx executable. The assumption then is that the
       build tool (dune) will compile the ppx executable and use it to
       compile the source file, as a kind of atomic operation, so to
       speak, so it must be smart enough to know that the ppx_codep
       (runtime dep) will be needed to compile the source file after
       ppx processing. which implies that the build tool must treat
       the 'requires(-ppx_driver)' value as a (co)dependency even when
       the predicate is ppx_driver. IOW, '-ppx_driver' does not mean
       "only use this if -ppx_driver' is passed"

       fuck. that means we really need two deps, one for ppx and one
       for non-ppx dep. Example: ppx_bin_prot. the META files have
       this comment:
       # This line makes things transparent for people mixing preprocessors
       # and normal dependencies
       requires(-ppx_driver) = "bin_prot ppx_here.runtime-lib"

       meaning we need a target @ppx_bin_prot//:foo for using it as a
       "normal" dep, not a ppx dep.

     */

    //FIXME: skip if no 'requires'
    /* obzl_meta_entries *entries = obzl_meta_package_entries(_pkg); */

    log_debug("emit_bazel_ppx_codeps, repo: %s, pfx: %s, pkg: %s\n",
              repo, _pkg_prefix, _pkg_name);
/* (FILE* ostream, int level, */
/*                            char *repo, */
/*                            char *_pkg_prefix, */
/*                            obzl_meta_entries *_entries) */

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
        fprintf(ostream, "%*sppx_codeps = select({\n", level*spfactor, sp);
    } else {
        fprintf(ostream, "%*sppx_codeps = [\n", level*spfactor, sp);
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
            has_conditions = obzl_meta_flags_to_selection_label(flags, condition_name);

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
            /* log_info("XXXX property val: '%s'", *v); */
            /* fprintf(ostream, "property val: '%s'\n", *v); */

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
                fprintf(ostream, "%*s\"FIXME%s//%s/%s\",\n",
                        (2+level)*spfactor, sp,
                        repo, _pkg_prefix, *v);
            } else {
                char *s = (char*)*v;
                char *tmp;
                while(s) {
                    tmp = strchr(s, '/');
                    if (tmp == NULL) break;
                    *tmp = '.';
                    s = tmp;
                }

                /* log_debug("%TESTING %s\n", *v); */

                /* then extract target segment */
                char * delim1 = strchr(*v, '.');

                if (delim1 == NULL) {
                    if (special_case_multiseg_dep(ostream, v, delim1))
                        continue;
                    else {
                /* if (delim1 == NULL) { */
                    int repo_len = strlen((char*)*v);
                    /* fprintf(ostream, "%*s\"@%.*s//:%s\",\n", */
                    fprintf(ostream, "%*s\"@%.*s\",\n",
                            (1+level)*spfactor, sp,
                            repo_len, *v, *v);
                    }
                } else {
                    if (special_case_multiseg_dep(ostream, v, delim1))
                        continue;
                    else {
                        //first the repo string
                        /* *delim1 = '\0'; // split string on '.' */
                        int repo_len = delim1 - (char*)*v;
                        /* fprintf(ostream, "%*s\"@%.*s//%s\",\n", */
                        fprintf(ostream, "%*s\"@%.*s/",
                                (1+level)*spfactor, sp,
                                repo_len, *v);
                        /* delim1+1); */
                        // then the pkg:target
                        char *s = (char*)delim1;
                        char *tmp;
                        while(s) {
                            tmp = strchr(s, '.');
                            if (tmp == NULL) break;
                            *tmp = '/';
                            s = tmp;
                        }
                        fprintf(ostream, "%s\",\n", delim1);

                        /* fprintf(ostream, "%*s\"G@%s//%s\",\n", */
                        /*         (1+level)*spfactor, sp, *v, _pkg_prefix); */
                        /* (1+level)*spfactor, sp, repo, _pkg_prefix, *v); */
                    }
                }
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

/* void emit_bazel_path_attrib(FILE* ostream, int level, */
/*                             char *repo, */
/*                             char *_pkg_src, */
/*                             char *pkg, */
/*                             obzl_meta_entries *_entries) */
/* { */
/*     log_debug("emit_bazel_path_attrib"); */
/*     /\* */
/* The variable "directory" redefines the location of the package directory. Normally, the META file is the first file read in the package directory, and before any other file is read, the "directory" variable is evaluated in order to see if the package directory must be changed. The value of the "directory" variable is determined with an empty set of actual predicates. The value must be either: an absolute path name of the alternate directory, or a path name relative to the stdlib directory of OCaml (written "+path"), or a normal relative path name (without special syntax). In the latter case, the interpretation depends on whether it is contained in a main or sub package, and whether the standard repository layout or the alternate layout is in effect (see site-lib for these terms). For a main package in standard layout the base directory is the directory physically containing the META file, and the relative path is interpreted for this base directory. For a main package in alternate layout the base directory is the directory physically containing the META.pkg files. The base directory for subpackages is the package directory of the containing package. (In the case that a subpackage definition does not have a "directory" setting, the subpackage simply inherits the package directory of the containing package. By writing a "directory" directive one can change this location again.) */
/*     *\/ */

/*     struct obzl_meta_property *deps_prop = obzl_meta_entries_property(_entries, "directory"); */
/*     if ( deps_prop == NULL ) { */
/*         /\* char *pkg_name = obzl_meta_package_name(_pkg); *\/ */
/*         /\* log_warn("Prop 'directory' not found: %s.", pkg_name); *\/ */
/*         return; */
/*     } */

/*     obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop); */
/*     obzl_meta_setting *setting = NULL; */

/*     int settings_ct = obzl_meta_settings_count(settings); */
/*     /\* log_info("settings count: %d", settings_ct); *\/ */

/*     if (settings_ct == 0) { */
/*         log_info("No deps for %s", obzl_meta_property_name(deps_prop)); */
/*         return; */
/*     } */
/*     if (settings_ct > 1) { */
/*         log_warn("More than one setting for property 'description'; using the first"); */
/*         return; */
/*     } */

/*     UT_string *condition_name; */
/*     setting = obzl_meta_settings_nth(settings, 0); */

/*     obzl_meta_flags *flags = obzl_meta_setting_flags(setting); */
/*     int flags_ct = obzl_meta_flags_count(flags); */
/*     if (flags_ct > 0) { */
/*         log_error("Property 'directory' has unexpected flags."); */
/*         return; */
/*     } */

/*     obzl_meta_values *vals = resolve_setting_values(setting, flags, settings); */
/*     log_debug("vals ct: %d", obzl_meta_values_count(vals)); */
/*     /\* dump_values(0, vals); *\/ */

/*     if (obzl_meta_values_count(vals) == 0) { */
/*         return; */
/*     } */

/*     if (obzl_meta_values_count(vals) > 1) { */
/*         log_error("Property 'directory' has more than one value."); */
/*         return; */
/*     } */

/*     obzl_meta_value *dir = NULL; */
/*     dir = obzl_meta_values_nth(vals, 0); */
/*     log_info("property val: '%s'", *dir); */

/*     bool stdlib = false; */
/*     if ( strncmp(*dir, "+", 1) == 0 ) { */
/*         (*dir)++; */
/*         stdlib = true; */
/*     } */
/*     log_info("property val 2: '%s'", *dir); */
/*     /\* fprintf(ostream, "%s", *\/ */
/*     /\*         "    modules = [\"//:compiler-libs.common\"],\n"); *\/ */
/* } */

/*
  A "deps target" is one that has a 'deps' attribute but no other that
  provides output (i.e. no 'archive' or 'plugin'). Some packages are
  there for compatibility; their resources have been migrated to the
  OCaml distrib. They contain something like 'version = "[distributed
  with Ocaml]"'. Some of these can be treated like normal packages:
  they have 'requires' and/or 'plugin', and a 'directory' property
  redirecting to stdlib, i.e. '^' or leading '+'. So they require no
  special handling (e.g. bigarray). Others may omit 'directory' and
  have no 'archive' or 'plugin' properties; but they do have
  'requires', so we need to emit a target for pkgs depending on them.

 */
void emit_bazel_deps_target(FILE* ostream, int level,
                               char *_repo,
                               char *_pkg_prefix,
                               /* char *_pkg_path, */
                               char *_pkg_name,
                               obzl_meta_entries *_entries)
{
    log_debug("DUMMY %s, %s", _pkg_prefix, _pkg_name);

    /* FIXME: only for here-switch
    write_opam_resolver(_pkg_prefix, _pkg_name, _entries);
    */

    fprintf(ostream, "\nocaml_import(\n");
    fprintf(ostream, "    name = \"%s\",\n", _pkg_name);
    emit_bazel_metadatum(ostream, 1,
                         _repo,
                         /* _pkg_path, */
                         _entries, "version", "version");
    emit_bazel_metadatum(ostream, 1,
                         _repo, // "@ocaml",
                         /* _pkg_path, */
                         _entries, "description", "doc");
    emit_bazel_deps_attribute(ostream, 1, host_repo, "lib", _pkg_name, _entries);
    /* emit_bazel_path_attrib(ostream, 1, host_repo, _pkg_prefix, "lib", _entries); */
    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
}

/*
  A small number of META packages are there just to throw an error;
  for example, core.syntax.
 */
void emit_bazel_error_target(FILE* ostream, int level,
                               char *_repo,
                               char *_pkg_src,
                               /* char *_pkg_path, */
                               char *_pkg_name,
                               obzl_meta_entries *_entries)
{
    log_debug("ERROR TARGET: %s", _pkg_name);
    fprintf(ostream, "\nocaml_import( # error\n");
    fprintf(ostream, "    name = \"%s\",\n", _pkg_name);
    emit_bazel_metadatum(ostream, 1,
                         _repo,
                         /* _pkg_path, */
                         _entries, "version", "version");
    emit_bazel_metadatum(ostream, 1,
                         _repo, // "@ocaml",
                         /* _pkg_path, */
                         _entries, "description", "doc");
    emit_bazel_metadatum(ostream, 1,
                         _repo,
                         /* _pkg_path, */
                         _entries, "error", "error");

    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
}

// FIXME: most of this duplicates emit_bazel_deps_attribute
/* void emit_bazel_ppx_dummy_deps(FILE* ostream, int level, char *repo, char *pkg, */
/*                                obzl_meta_entries *_entries) */
/*                      /\* obzl_meta_package *_pkg) *\/ */
/* { */
/*     log_debug("emit_bazel_ppx_dummy_deps"); */
/*     //FIXME: skip if no 'requires' */
/*     /\* obzl_meta_entries *entries = obzl_meta_package_entries(_pkg); *\/ */

/*     char *pname = "requires"; */
/*     /\* struct obzl_meta_property *deps_prop = obzl_meta_package_property(_pkg, pname); *\/ */
/*     struct obzl_meta_property *deps_prop = obzl_meta_entries_property(_entries, pname); */
/*     if ( deps_prop == NULL ) { */
/*         /\* char *pkg_name = obzl_meta_package_name(_pkg); *\/ */
/*         /\* log_warn("Prop '%s' not found: %s.", pname, pkg_name); *\/ */
/*         return; */
/*     } */

/*     obzl_meta_settings *settings = obzl_meta_property_settings(deps_prop); */
/*     obzl_meta_setting *setting = NULL; */

/*     int settings_ct = obzl_meta_settings_count(settings); */
/*     int settings_no_ppx_driver_ct = obzl_meta_settings_flag_count(settings, "ppx_driver", false); */

/*     /\* settings_ct -= settings_no_ppx_driver_ct; *\/ */

/*     /\* log_info("settings count: %d", settings_ct); *\/ */

/*     if (settings_no_ppx_driver_ct == 0) { */
/*         log_info("No 'requires(-ppx_driver)' for %s", obzl_meta_property_name(deps_prop)); */
/*         return; */
/*     } */

/*     obzl_meta_values *vals; */
/*     obzl_meta_value *v = NULL; */

/*     if (settings_no_ppx_driver_ct > 1) { */
/*         fprintf(ostream, "%*sDEPS = select({\n", level*spfactor, sp); */
/*     } else { */
/*         fprintf(ostream, "%*sDEPS = [\n", level*spfactor, sp); */
/*     } */

/*     UT_string *condition_name; */
/*     utstring_new(condition_name); */

/*     for (int i = 0; i < settings_ct; i++) { */
/*         setting = obzl_meta_settings_nth(settings, i); */
/*         log_debug("setting %d", i+1); */
/*         /\* dump_setting(8, setting); *\/ */

/*         if (obzl_meta_setting_has_flag(setting, "ppx_driver", true)) { */
/*             log_debug("skipping setting with flag +ppx_driver"); */
/*             continue; */
/*         } */

/*         obzl_meta_flags *flags = obzl_meta_setting_flags(setting); */
/*         /\* int flags_ct = 0; *\/ */
/*         /\* if (flags != NULL) { *\/ */
/*         /\*     register_flags(flags); *\/ */
/*         /\*     flags_ct = obzl_meta_flags_count(flags); *\/ */
/*         /\* } *\/ */

/*         /\* select only settings with flag -ppx_driver *\/ */
/*         if ( !obzl_meta_flags_has_flag(flags, "ppx_driver", false) ) { */
/*             continue; */
/*         } */

/*         bool has_conditions; */
/*         if (flags == NULL) */
/*             utstring_printf(condition_name, "//conditions:default"); */
/*         else */
/*             has_conditions = obzl_meta_flags_to_selection_label(flags, condition_name); */

/*         char *condition_comment = obzl_meta_flags_to_comment(flags); */
/*         /\* log_debug("condition_comment: %s", condition_comment); *\/ */

/*         /\* 'requires' usually has no flags; when it does, empirically we find only *\/ */
/*         /\*   ppx pkgs: ppx_driver, -ppx_driver *\/ */
/*         /\*   pkg 'batteries': requires(mt) *\/ */
/*         /\*   pkg 'num': requires(toploop) *\/ */
/*         /\*   pkg 'findlib': requires(toploop), requires(create_toploop) *\/ */

/*         /\* Multiple settings on 'requires' means multiple flags; *\/ */
/*         /\* empirically, this only happens for ppx packages, typically as *\/ */
/*         /\* requires(-ppx_driver,-custom_ppx) *\/ */
/*         /\* the (sole?) exception is *\/ */
/*         /\*   pkg 'threads': requires(mt,mt_vm), requires(mt,mt_posix) *\/ */

/*         // FIXME: we're assuming we always have: */
/*         // 'requires(-ppx_driver)' and 'requires(-ppx_driver,-custom_ppx)' */

/*         if (settings_ct > 1) { */
/*             fprintf(ostream, "%*s\"%s\": [ ## predicates: %s\n", */
/*                     (1+level)*spfactor, sp, */
/*                     utstring_body(condition_name), // : "//conditions:default", */
/*                     /\* (has_conditions)? "_enabled" : "", *\/ */
/*                     condition_comment); */
/*         } */

/*         vals = resolve_setting_values(setting, flags, settings); */
/*         /\* vals = obzl_meta_setting_values(setting); *\/ */
/*         /\* log_debug("vals ct: %d", obzl_meta_values_count(vals)); *\/ */
/*         /\* dump_values(0, vals); *\/ */
/*         /\* now we handle UPDATE settings *\/ */

/*         for (int j = 0; j < obzl_meta_values_count(vals); j++) { */
/*             v = obzl_meta_values_nth(vals, j); */
/*             /\* log_info("property val: '%s'", *v); *\/ */

/*             char *s = (char*)*v; */
/*             while (*s) { */
/*                 /\* printf("s: %s\n", s); *\/ */
/*                 if(s[0] == '.') { */
/*                     /\* log_info("Hit"); *\/ */
/*                     s[0] = '/'; */
/*                 } */
/*                 s++; */
/*             } */
/*             if (settings_ct > 1) { */
/*                 fprintf(ostream, "%*s\"%s//%s/%s\",\n", */
/*                         (2+level)*spfactor, sp, */
/*                         repo, pkg, *v); */
/*             } else { */
/*                 fprintf(ostream, "%*s\"%s//%s/%s\"\n", (1+level)*spfactor, sp, repo, pkg, *v); */
/*             } */
/*         } */
/*         if (settings_ct > 1) { */
/*             fprintf(ostream, "%*s],\n", (1+level)*spfactor, sp); */
/*         } */
/*         free(condition_comment); */
/*     } */
/*     utstring_free(condition_name); */
/*     if (settings_no_ppx_driver_ct > 1) */
/*         fprintf(ostream, "%*s}),\n", level*spfactor, sp); */
/*     else */
/*         fprintf(ostream, "%*s],\n", level*spfactor, sp); */
/* } */

void emit_bazel_subpackages(char *_repo,
                            char *_repo_root,
                            char *_pkg_prefix,
                            char *_filedeps_path,
                            /* char *_pkg_path, */
                            struct obzl_meta_package *_pkg)
                            /* char *_subpkg_dir) */
{
    log_debug("emit_bazel_subpackages, pkg: %s", _pkg->name);
    log_debug("\t_repo: %s; _repo_root: %s", _repo, _repo_root);
    log_debug("\t_pkg_prefix: %s", _pkg_prefix); //, _pkg_path);
    log_debug("\t_filedeps_path: %s", _filedeps_path);

    obzl_meta_entries *entries = _pkg->entries;
    obzl_meta_entry *entry = NULL;

    char *pkg_name = obzl_meta_package_name(_pkg);
    UT_string *_new_pkg_prefix;
    utstring_new(_new_pkg_prefix);
    if (_pkg_prefix == NULL)
        utstring_printf(_new_pkg_prefix, "%s", pkg_name);
    else
        utstring_printf(_new_pkg_prefix, "%s/%s", _pkg_prefix, pkg_name);

    /* UT_string *filedeps_path; */
    for (int i = 0; i < obzl_meta_entries_count(entries); i++) {
        entry = obzl_meta_entries_nth(entries, i);
        if (entry->type == OMP_PACKAGE) {
            obzl_meta_package *subpkg = entry->package;
            /* obzl_meta_entries *sub_entries = subpkg->entries; */

            /* filedeps_path will be handled by emit_build_bazel  */

            /* utstring_renew(filedeps_path); */
            /* char *subdir = obzl_meta_directory_property(sub_entries); */
            /* log_debug("subdir: %s", subdir); */
            /* if (subdir == NULL) { */
            /*     utstring_printf(filedeps_path, "%s", _filedeps_path); */
            /* } else { */
            /*     utstring_printf(filedeps_path, "%s/%s", */
            /*                     _filedeps_path, subdir); */
            /* } */
            /* log_debug("SUBPKG: %s::%s, filedeps: %s", */
            /*           utstring_body(_new_pkg_prefix), */
            /*           subpkg->name, */
            /*           utstring_body(filedeps_path)); */
            emit_build_bazel(_repo,
                             _repo_root,
                             /* _pkg_prefix, */
                             utstring_body(_new_pkg_prefix),
                             _filedeps_path,
                             /* utstring_body(filedeps_path), */
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
      Directory '^' means '@rules_ocaml//cfg/lib/ocaml'. problem
      is lib/ocaml has no META file; it's the stdlib, always included.
      what about subdirs like 'threads' or 'integers'?

      If directory starts with '+', pkg is relative to '@rules_ocaml//cfg/lib/ocaml'. problem
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

void emit_workspace_file(char *repo_name)
{
    FILE *ostream;
    ostream = fopen(utstring_body(workspace_file), "w");
    if (ostream == NULL) {
        log_error("%s", strerror(errno));
        perror(utstring_body(workspace_file));
        exit(EXIT_FAILURE);
    }

    fprintf(ostream, "workspace( name = \"%s\" )"
            "    # generated file - DO NOT EDIT\n",
            repo_name);

    fclose(ostream);
}

void emit_pkg_symlinks(UT_string *dst_dir,
                       UT_string *src_dir,
                       char *pkg_name)
{
    if (debug)
        log_debug("emit_symlinks for pkg: %s", pkg_name);
    if ((strncmp(pkg_name, "dune", 4) == 0)
        || (strncmp(utstring_body(src_dir), "dune/configurator", 17) == 0)) {
        if (debug)
            log_debug("Skipping 'dune' stuff\n");
        return;
    }
    if (debug) {
        log_debug("\tpkg_symlinks src_dir: %s", utstring_body(src_dir));
        log_debug("\tpkg_symlinks dst_dir: %s", utstring_body(dst_dir));
    }

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/%s",
                    utstring_body(opam_switch_lib),
                    //pkg_name
                    utstring_body(src_dir)
                    );

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    log_debug("opening src_dir for read: %s\n", utstring_body(opamdir));
    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        if (strncmp(pkg_name, "care", 4) == 0) {
            if (strncmp(utstring_body(src_dir),
                        "topkg/../topkg-care",
                        19) == 0) {
                if (debug)
                    log_debug("Skipping missing 'topkg-care'\n");
                if (debug && verbose)
                    fprintf(stdout, "Skipping missing pkg 'topkg-care'\n");
                return;
            }
        } else
            fprintf(stderr, "pkg_symlinks: Unable to opendir %s\n",
                    utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        //Condition to check regular file.
        if(direntry->d_type==DT_REG){
            /* printf("dirent: %s/%s\n", */
            /*        utstring_body(opamdir), direntry->d_name); */
            /* printf("\t=> %s/%s\n", */
            /*        utstring_body(dst_dir), */
            /*        //pkg_name, */
            /*        direntry->d_name); */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(dst_dir), direntry->d_name);
            /* if (debug) */
            /*     log_debug("pkg symlinking %s to %s", */
            /*               utstring_body(src), */
            /*               utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

/*
  params:
  _tgtroot: output dir
  _repo: @ocaml

  _pkg_prefix - result of concatenating ancestors up to parent; starts
                with bzl_lib (default: "lib")

  pkg path will be made by concatenating _pkg_prefix to pkg name
 */
EXPORT void emit_build_bazel(char *_repo,
                             char *_repo_root,
                             char *_pkg_prefix,
                             char *_filedeps_path,
                             /* char *_pkg_path, /\* FIXME: remove *\/ */
                             struct obzl_meta_package *_pkg)
                             /* FILE *bootstrap_FILE) */
                             /* char *_subpkg_dir) */
{
    log_info("EMIT_BUILD_BAZEL pkg: %s: pfx: %s",
             obzl_meta_package_name(_pkg),
             _pkg_prefix);
    log_info("_repo_root: %s", _repo_root);
    log_info("_filedeps_path: %s", _filedeps_path);
    /* dump_package(0, _pkg); */

    /* if (strncmp(_pkg->name, "ppx_fixed_literal", 17) == 0) { */
    /*     log_debug("PPX_FIXED_LITERAL dump:"); */
    /*     dump_package(0, _pkg); */
    /* } */

    char *pkg_name = obzl_meta_package_name(_pkg);
#ifdef DEBUG_TRACE
    log_info("\n\t_repo: @%s; _pkg_prefix: '%s'; pkg name: '%s'",
             _repo, _pkg_prefix, pkg_name);
    /* log_info("\n\t_subpkg_dir: %s", _subpkg_dir); */
    log_info("\tlabel: @%s//%s/%s", _repo, _pkg_prefix, pkg_name);
    log_info("\t_filedeps_path: '%s'", _filedeps_path);
    log_info("\t_repo_root: '%s'", _repo_root);
    /* log_set_quiet(false); */
    /* log_debug("%*sparsed name: %s", indent, sp, pkg_name); */
    log_debug("%*spkg dir:  %s", indent, sp, obzl_meta_package_dir(_pkg));
    log_debug("%*sparsed src:  %s", indent, sp, obzl_meta_package_src(_pkg));
#endif

    utstring_renew(bazel_pkg_root);
    utstring_renew(build_bazel_file);
    /* utstring_clear(build_bazel_file); */

    log_info("_REPO_ROOT: %s\n", _repo_root);
    /* printf("PFXDIR: %s\n", pfxdir); */

    if (_pkg_prefix == NULL)
        if (pfxdir == NULL)
            utstring_printf(build_bazel_file, "%s", _repo_root);
        else
            utstring_printf(build_bazel_file, "%s/%s", _repo_root, pfxdir);
    else {
        if (pfxdir == NULL)
            utstring_printf(build_bazel_file, "%s/%s",
                            _repo_root, _pkg_prefix);
        else
            utstring_printf(build_bazel_file, "%s/%s/%s",
                            _repo_root, pfxdir, _pkg_prefix);

    }
    /* printf("_PKG_PREFIX: %s\n", _pkg_prefix); */

    /* WARNING: pkg may be empty (no entries) */
    /* e.g. js_of_ocaml contains: package "weak" ( ) */
    obzl_meta_entries *entries = _pkg->entries;
    UT_string *new_filedeps_path = NULL;
    utstring_renew(new_filedeps_path);
    char *directory = obzl_meta_directory_property(entries);
    log_debug("DIRECTORY property: '%s'", directory);
    if ( directory == NULL ) {
        /* directory omitted or directory = "" */
        /* if (_filedeps_path != NULL) */
            utstring_printf(new_filedeps_path, "%s", _filedeps_path);
    } else {
        /* utstring_printf(build_bazel_file, "/%s", subpkg_dir); */
        if ( strncmp(directory, "+", 1) == 0 ) {
            /* Initial '+' means 'relative to stdlib dir', i.e. these
               are libs found in lib/ocaml. used by: compiler-libs, ?
               subdirs of lib/ocaml: stublibs, caml, compiler-libs,
               threads, threads/posix, ocamldoc.
            */
            /* stdlib = true; */
            log_debug("Found STDLIB directory '%s' for %s", directory, pkg_name);
            directory++;
            utstring_printf(new_filedeps_path, "ocaml/%s", directory);
            /* utstring_printf(new_filedeps_path, "_lib/ocaml/%s", directory); */
        } else {
            if (strncmp(directory, "^", 1) == 0) {
                /* Initial '^' means pkg within stdlib; "Only included
                   because of findlib-browser" or "distributed with
                   Ocaml". only applies to: raw-spacetime, num, threads,
                   bigarray, unix, stdlib, str, dynlink, ocamldoc.
                   in the opam repo these only have a META file,
                   which redirects to files or subdirs in lib/ocaml
                */
                /* stdlib = true; */
                directory++;
                stdlib_root = true;
                if (strlen(directory) == 1)
                    utstring_printf(new_filedeps_path, "%s", "ocaml");
                else
                    utstring_printf(new_filedeps_path,
                                    "ocaml/%s",
                                    directory);
                /* utstring_printf(new_filedeps_path, "_lib/%s", "ocaml"); */
                log_debug("Found STDLIB root directory for %s: '%s' => '%s'",
                          pkg_name, directory, utstring_body(new_filedeps_path));
            } else {
                utstring_printf(new_filedeps_path,
                                "%s/%s", _filedeps_path, directory);
            }
        }
    }
    log_debug("new_filedeps_path: %s", utstring_body(new_filedeps_path));

    /* if (obzl_meta_package_dir(_pkg) != NULL) */

    /* printf("PKG_NAME: %s\n", pkg_name); */

    utstring_printf(build_bazel_file, "/%s", pkg_name);
    utstring_renew(bazel_pkg_root);
    utstring_concat(bazel_pkg_root, build_bazel_file);

    mkdir_r(utstring_body(bazel_pkg_root));

    utstring_renew(workspace_file);
    utstring_concat(workspace_file, build_bazel_file);
    utstring_printf(workspace_file, "/%s", "WORKSPACE.bazel");

    utstring_printf(build_bazel_file, "/%s", "BUILD.bazel");

    log_debug("bazel_pkg_root: %s", utstring_body(bazel_pkg_root));
    log_debug("build_bazel_file: %s", utstring_body(build_bazel_file));
    log_debug("workspace_file: %s", utstring_body(workspace_file));

    rc = access(utstring_body(build_bazel_file), F_OK);

    /* **************************************************************** */
    // now links
    if (_pkg->entries != NULL) {
        emit_pkg_symlinks(bazel_pkg_root,
                          new_filedeps_path,
                          pkg_name);
    }

    /* if (rc < 0) { */
    /*     log_info("EMITTING: %s", utstring_body(build_bazel_file)); */
    /* } else { */
    /*     log_info("ALREADY EMITTED: %s", utstring_body(build_bazel_file)); */
    /*     return; */
    /* } */

    /* ################################################################ */
    /* emit_new_local_pkg_repo(bootstrap_FILE, */
    /*                         _pkg_prefix, */
    /*                         _pkg); */

    /* fprintf(stdout, "Writing: %s\n", utstring_body(build_bazel_file)); */

    FILE *ostream;
    ostream = fopen(utstring_body(build_bazel_file), "w");
    if (ostream == NULL) {
        perror(utstring_body(build_bazel_file));
        log_error("fopen failure for %s", utstring_body(build_bazel_file));
        log_error("Value of errno: %d", errnum);
        log_error("fopen error %s", strerror( errnum ));
        exit(EXIT_FAILURE);
    }

    if ((_pkg_prefix == NULL)
        || ((strncmp(_pkg_prefix, "ocaml", 5) == 0) ))
            /* && (strlen(_pkg_prefix) == 5))) */
        if (emit_special_case_rule(ostream, _pkg)) {
            log_trace("emit_special_case_rule:TRUE %s", _pkg->name);
            return;
        }

    fprintf(ostream, "## generated file - DO NOT EDIT\n");

    fprintf(ostream, "## original: %s\n\n", obzl_meta_package_src(_pkg));

    /*FIXME: META file does not contain info about executables
      generated by dune-package. E.g. ppx_tools. So we can't emit
      target code for them. For now we just export everything.

      options: iterate over files and pick out executables; or parse
      the dune-package file.
    */
    fprintf(ostream, "exports_files(glob([\"**\"]))\n\n");

    emit_bazel_hdr(ostream); //, 1, host_repo, "lib", _pkg);


    /* special case */
    if ((strncmp(pkg_name, "ctypes", 6) == 0)
        && strlen(pkg_name) == 6) {
        fprintf(ostream,
                "load(\"@rules_cc//cc:defs.bzl\", \"cc_library\")\n"
                "cc_library(\n"
                "    name = \"libctypes\",\n"
                "    srcs = glob([\"*.a\"]),\n"
                "    hdrs = glob([\"*.h\"]),\n"
                "    visibility = [\"//visibility:public\"],\n"
                ")\n");
    }

    /* if (strncmp(_pkg->name, "ppx_fixed_literal", 17) == 0) { */
    /*     log_debug("PPX_FIXED_LITERAL dump:"); */
    /*     /\* dump_package(0, _pkg); *\/ */

    /*     log_debug("PPX_FIXED_LITERAL ENTRIES count: %d", */
    /*               obzl_meta_entries_count(_pkg->entries)); */
    /*     /\* dump_entries(0, _pkg->entries); *\/ */
    /* } */

    /* if (strncmp(_pkg->name, "base", 17) == 0) { */
    /*     log_debug("BASE dump:"); */
    /*     dump_package(0, _pkg); */
    /* } */

    emit_bazel_cc_imports(ostream, 1,
                          _pkg_prefix,
                          pkg_name,
                          _filedeps_path,
                          entries,
                          _pkg);

    obzl_meta_entry *e = NULL;
    if (_pkg->entries == NULL) {
        log_debug("EMPTY ENTRIES\n");
    } else {
        for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
            e = obzl_meta_entries_nth(_pkg->entries, i);

            /* if (strncmp(_pkg->name, "ppx_fixed_literal", 17) == 0) */
            /*     log_debug("PPX_FIXED_LITERAL, entry %d, name: %s, type %d", */
            /*               i, e->property->name, e->type); */

            /*
              FIXME: some packages use plugin or toploop flags in an
              'archive' property. We need to check the archive property,
              and if this is the case, generate separate import targets
              for them. Unlike findlib we do not use flags to select the
              files we want; instead we expose everything using
              ocaml_import targets.
            */

            if (e->type == OMP_PROPERTY) {
                /* if ((e->type == OMP_PROPERTY) || (e->type == OMP_PACKAGE)) { */
                /* log_debug("\tOMP_PROPERTY"); */

                /* 'library_kind` :=  'ppx_deriver' or 'ppx_rewriter' */
                /* if (strncmp(e->property->name, "library_kind", 12) == 0) { */
                /*     emit_bazel_ppx_dummy_rule(ostream, 1, host_repo, "_lib", _pkg_path, pkg_name, entries); */
                /*     continue; */
                /* } */

                if (strncmp(e->property->name, "archive", 7) == 0) {
                    log_debug("calling emit_bazel_archive_rule with %s",
                              _filedeps_path);
                    emit_bazel_archive_rule(ostream, 1,
                                            _repo, // host_repo,
                                            //_filedeps_path,
                                            utstring_body(new_filedeps_path),
                                            _pkg_prefix,
                                            pkg_name,
                                            entries, //);
                                            _pkg);
                    /* subpkg_dir); */
                    continue;
                }

                /* log_debug("PDUMP0 %s", _pkg->name); */
                /* dump_entries(0, entries); */

                if (strncmp(e->property->name, "plugin", 6) == 0) {
                    /* log_debug("PDUMPPPP plugin"); */
                    emit_bazel_plugin_rule(ostream, 1,
                                           host_repo,
                                           utstring_body(new_filedeps_path),
                                           _pkg_prefix,
                                           pkg_name,
                                           entries, //);
                                           _pkg);
                    continue;
                }
                /* if (strncmp(e->property->name, "ppx", 3) == 0) { */
                /*     log_warn("IGNORING PPX PROPERTY for %s", pkg_name); */
                /*     continue; */
                /* } */

                /* at this point we've processed archive and plugin props.
                   now we handle pkgs that have deps ('requires') but do
                   not deliver any files ('archive' or 'plugin').
                */

                if (strncmp(e->property->name, "requires", 6) == 0) {
                    /* some pkgs have 'requires' but no 'archive' nor 'plugin' */
                    /* compiler-libs has 'requires = ""' and 'director = "+compiler-libs"' */
                    /* ppx packages have 'requires(ppx_driver)' and 'requires(-ppx_driver)' */

                    /* we should've already handled these 2: */
                    if (obzl_meta_entries_property(entries, "archive"))
                        continue;
                    if (obzl_meta_entries_property(entries, "plugin"))
                        continue;

                    /* compatibility pkgs whose resources are now
                       'distributed with OCaml' may not have 'archive' or
                       'plugin', but we still need to generate a target,
                       since they have deps ('requires') that legacy code
                       may depend on. */

                    /* UPDATE: we now generate the standard dummy pkgs
                       like bigarray and threads using a repo rule */
                    emit_bazel_deps_target(ostream, 1, host_repo,
                                           _pkg_prefix,
                                           pkg_name, entries);
                    continue;
                }
                /* if (strncmp(_pkg->name, "ppx_fixed_literal", 17) == 0) */
                /*     log_debug("PPX_fixed_literal, entry %d, name: %s, type %d", */
                /*               i, e->property->name, e->type); */
                /* no 'archive', 'plugin', or 'requires' */
                /* if (strncmp(e->property->name, "directory", 9) == 0) { */
                /*     handle_directory_property(ostream, 1, host_repo, */
                /*                               "_lib", pkg_name, entries); */
                /*     continue; */
                /* } */

                if (obzl_meta_entries_property(entries, "error")) {
                    emit_bazel_error_target(ostream, 1, host_repo,
                                            "FIXME_ERROR",
                                            /* _pkg_prefix, */
                                            pkg_name, entries);
                    break;
                }

                log_warn("processing other property: %s", e->property->name);
                /* dump_entry(0, e); */
                // push all flags to global pos_flags
            }
        }
    }
    /* emit_bazel_flags(ostream, _repo_root, _repo, _pkg_prefix, _pkg); */

    fclose(ostream);

    /* printf("pkg pfx: %s, pkg name: %s\n", _pkg_prefix, pkg_name); */
    if (_pkg_prefix == NULL)
        emit_workspace_file(pkg_name);

    // now links
    if (_pkg->entries != NULL) {
        /* emit_pkg_symlinks(bazel_pkg_root, */
        /*                   new_filedeps_path, */
        /*                   pkg_name); */

        /* char new_pkg_path[PATH_MAX]; */
        /* new_pkg_path[0] = '\0'; */
        /* mystrcat(new_pkg_path, _pkg_path); */
        /* if (strlen(_pkg_path) > 0) */
        /*     mystrcat(new_pkg_path, "/"); */

        /* subpackage may not have a 'directory' property. example: ptime */
        /* char  *subpkg_name = obzl_meta_package_name(_pkg); */
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
        /*                            _pkg_prefix, _filedeps_path, */
        /*                            // _pkg_path, */
        /*                            _pkg); //, _subpkg_dir); */
        /* } else { */

        emit_bazel_subpackages(_repo,
                               _repo_root,
                               _pkg_prefix,
                               utstring_body(new_filedeps_path),
                               //_filedeps_path,
                               // new_pkg_path,
                               _pkg); //, _subpkg_dir);
    }
}
