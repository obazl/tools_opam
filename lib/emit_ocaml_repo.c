#include <errno.h>
#include <dirent.h>
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
/* #if EXPORT_INTERFACE */
#include <stdio.h>
/* #endif */
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "liblogc.h"
#include "findlibc.h"
#include "opamc.h"
#include "utarray.h"
#if EXPORT_INTERFACE
#include "utstring.h"
#endif

/* #include "cjson/cJSON.h" */
/* #include "mustach-cjson.h" */
/* #include "mustach.h" */
/* #include "librunfiles.h" */

#include "emit_ocaml_repo.h"

extern int verbosity;
extern int log_writes;
extern int log_symlinks;

extern char *default_version;
extern int   default_compat;
extern char *bazel_compat;

/* LOCAL const char *ws_name = "mibl"; */

/* extern UT_string *opam_switch_id; */
/* extern UT_string *opam_switch_prefix; */
extern UT_string *opam_ocaml_version;
/* extern UT_string *opam_switch_bin; */
/* extern UT_string *opam_switch_lib; */
/* extern UT_string *coswitch_runfiles_root; */

#define DEBUG_LEVEL coswitch_debug
extern int  DEBUG_LEVEL;
#define TRACE_FLAG coswitch_trace
extern bool TRACE_FLAG;
bool coswitch_debug_symlinks = false;

/* EXPORT void _emit_toplevel(UT_string *templates, */
/*                     char *template, */
/*                     UT_string *src_file, */
/*                     UT_string *dst_dir, */
/*                     UT_string *dst_file, */
/*                     char *coswitch_lib, */
/*                     char *pkg */
/*                     ) */
/* { */
/*     TRACE_ENTRY; */
/*     (void)templates; */
/*     // create <switch>/lib/str for ocaml >= 5.0.0 */
/*     // previous versions already have it */
/*     // step 1: write MODULE.bazel, WORKSPACE.bazel */
/*     // step 2: write lib/str/lib/str/BUILD.bazel */
/*     // step 3: write registry record */
/*     char *content_template = "" */
/*         "## generated file - DO NOT EDIT\n" */
/*         "\n" */
/*         "module(\n" */
/*         "    name = \"%s\", version = \"0.0.0\",\n" */
/*         "    compatibility_level = \"0\"\n" */
/*         "    bazel_compatibility = [\">=%s\"]\n" */
/*         ")\n" */
/*         "\n" */
/*         "bazel_dep(name = \"ocaml\", version = \"%s\")\n"; */
/*     UT_string *content; */
/*     utstring_new(content); */
/*     utstring_printf(content, content_template, */
/*                     pkg, bazel_compat, ocaml_version); */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/%s", */
/*                     coswitch_lib, */
/*                     pkg); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     utstring_renew(dst_file); */
/*     utstring_printf(dst_file, "%s/MODULE.bazel", */
/*                     utstring_body(dst_dir)); */
/*     // write content to dst_file */
/*     FILE *ostream = fopen(utstring_body(dst_file), "w"); */
/*     if (ostream == NULL) { */
/*         perror(utstring_body(dst_file)); */
/*         log_error("fopen: %s: %s", strerror(errno), */
/*                   utstring_body(dst_file)); */
/*         exit(EXIT_FAILURE); */
/*     } */
/*     fprintf(ostream, "%s", utstring_body(content)); */
/*     fclose(ostream); */
/*     if (verbosity > log_writes) { */
/*         LOG_INFO(0, "wrote: %s", utstring_body(dst_file)); */
/*     } */
/*     /\* utstring_renew(dst_file); *\/ */
/*     /\* utstring_printf(dst_file, "%s/WORKSPACE.bazel", *\/ */
/*     /\*                 utstring_body(dst_dir)); *\/ */
/*     /\* // write content to ws file *\/ */
/*     /\* ostream = fopen(utstring_body(dst_file), "w"); *\/ */
/*     /\* if (ostream == NULL) { *\/ */
/*     /\*     perror(utstring_body(dst_file)); *\/ */
/*     /\*     log_error("fopen: %s: %s", strerror(errno), *\/ */
/*     /\*               utstring_body(dst_file)); *\/ */
/*     /\*     exit(EXIT_FAILURE); *\/ */
/*     /\* } *\/ */
/*     /\* fprintf(ostream, "## generated file - DO NOT EDIT\n"); *\/ */
/*     /\* fclose(ostream); *\/ */
/*     /\* if (verbosity > log_writes) *\/ */
/*     /\*     fprintf(INFOFD, GRN "INFO" CRESET *\/ */
/*     /\*             " wrote: %s\n", utstring_body(dst_file)); *\/ */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/%s/lib/%s", */
/*                     coswitch_lib, */
/*                     pkg, pkg); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     utstring_renew(dst_file); */
/*     utstring_printf(dst_file, "%s/BUILD.bazel", */
/*                     utstring_body(dst_dir)); */

/*     utstring_renew(src_file); */
/*     utstring_printf(src_file, */
/*                     /\* "%s/%s", *\/ */
/*                     BAZEL_CURRENT_REPOSITORY "/templates/ocaml/%s", */
/*                     /\* utstring_body(templates), *\/ */
/*                     template); */

/*     /\* char *s = rf_rlocation(utstring_body(src_file)); *\/ */
/*     /\* LOG_DEBUG(0, "XXXX cp src: %s, dst: %s", *\/ */
/*     /\*           utstring_body(src_file), *\/ */
/*     /\*           utstring_body(dst_file)); *\/ */
/*     copy_buildfile(rf_rlocation(utstring_body(src_file)), */
/*                    dst_file); */
/*     TRACE_EXIT; */
/* } */

FILE *_open_buildfile(UT_string *ocaml_file)
{
    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    /* ostream =       fopen(utstring_body(ocaml_file), "w"); */
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    return ostream;
}

EXPORT void emit_ocaml_stdlib_pkg(UT_string *dst_dir,
                                  char *switch_lib)
{
    TRACE_ENTRY;
    bool add_toplevel = false;

    UT_string *stdlib_dir;
    utstring_new(stdlib_dir);
    utstring_printf(stdlib_dir,
                    "%s/stdlib",
                    switch_lib);
    int rc = access(utstring_body(stdlib_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(stdlib_dir));
        utstring_new(stdlib_dir);
        utstring_printf(stdlib_dir,
                        "%s/ocaml/stdlib",
                        switch_lib);
        int rc = access(utstring_body(stdlib_dir), F_OK);
        if (rc != 0) {
            /* if (coswitch_trace) */
            LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                     utstring_body(stdlib_dir));
            utstring_free(stdlib_dir);
            return;
        } else {
            LOG_WARN(0, YEL "FOUND: %s" CRESET,
                     utstring_body(stdlib_dir));
            add_toplevel = true;
        }
    }

    UT_string *dst_file;
    utstring_new(dst_file);

    // 5.0.0+ has <switch>/lib/ocaml/stdlib/META
    // pre-5.0.0: no <switch>/lib/ocaml/stdlib
    // always emit <coswitch>/lib/ocaml/stdlib
    utstring_printf(dst_file, "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_stdlib_BUILD,
              ocamlsdk_stdlib_BUILD_len,
              utstring_body(dst_file));

    if (add_toplevel) {
        // not found: <switch>/lib/stdlib
        // means >= 5.0.0 (<switch>/lib/ocaml/stdlib)

        // add <coswitch>/lib/stdlib for (bazel) compatibility
        /* _emit_toplevel(templates, "ocaml_stdlib_alias.BUILD", */
        /*                src_file, */
        /*                dst_dir, dst_file, */
        /*                coswitch_lib, "stdlib"); */
    } else {
        // if we found <switch>/lib/stdlib (versions < 5.0.0)
        // then we need to populate <coswitch>/lib/ocaml/stdlib
        // (its already there for >= 5.0.0)
        utstring_new(stdlib_dir);
        utstring_printf(stdlib_dir,
                        "%s/ocaml",
                        switch_lib);
        /* _symlink_ocaml_stdlib(stdlib_dir, */
        /*                        utstring_body(dst_dir)); */
    }

    /* stdlib files are always in <switchpfx>/lib/ocaml
       even for v5
    */
    utstring_renew(stdlib_dir);
    utstring_printf(stdlib_dir,
                    "%s/ocaml",
                    switch_lib);
    /* utstring_renew(dst_dir); */
    /* utstring_printf(dst_dir, */
    /*                 "%s/ocamlsdk/lib/stdlib", */
    /*                 coswitch_lib); */
    _symlink_ocaml_stdlib(stdlib_dir,
                          utstring_body(dst_dir));

    utstring_free(dst_file);
    utstring_free(stdlib_dir);
    TRACE_EXIT;
}

void _symlink_ocaml_stdlib(UT_string *stdlib_srcdir, char *tgtdir)
{
    TRACE_ENTRY;
    /* LOG_DEBUG(0, "stdlib_srcdir: %s", utstring_body(stdlib_srcdir)); */
    /* LOG_DEBUG(0, "tgtdir: %s", tgtdir); */

    /* UT_string *opamdir; */
    /* utstring_new(opamdir); */
    /* utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_stdlib_srcdir)); */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(stdlib_srcdir));
    if (d == NULL) {
        log_error("Unable to opendir for symlinking stdlib: %s\n",
                  utstring_body(stdlib_srcdir));
        /* exit(EXIT_FAILURE); */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            if (strncmp("stdlib", direntry->d_name, 6) != 0) {
                if (strncmp("camlinternal",
                            direntry->d_name, 12) != 0) {
                    continue;
                }
            }

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(stdlib_srcdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* LOG_DEBUG(0, "symlinking %s to %s", */
            /*           utstring_body(src), */
            /*           utstring_body(dst)); */
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
    TRACE_EXIT;
}

EXPORT void emit_ocaml_runtime_pkg(UT_string *dst_dir,
                                   char *switch_lib)
{
    TRACE_ENTRY;
    /* UT_string *templates; */
    /* utstring_new(templates); */
    /* utstring_printf(templates, "%s/templates/ocaml", runfiles); */
    /* utstring_printf(templates, "%s/%s", */
    /*                 runfiles, "/new/coswitch/templates"); */

    /* _symlink_ocaml_runtime(utstring_body(dst_file)); */
    _symlink_ocaml_runtime_libs(switch_lib, utstring_body(dst_dir));
    _symlink_ocaml_runtime_compiler_libs(switch_lib, utstring_body(dst_dir));
    /* _symlink_ocaml_runtime(switch_lib, utstring_body(dst_dir)); */

    /* ******************************** */
    /* UT_string *src_file; */
    /* utstring_new(src_file); */
    /* utstring_printf(src_file, "%s/%s", */
    /*                 utstring_body(templates), */
    /*                 "ocaml_runtime.BUILD"); */

    UT_string *dst_file;
    utstring_new(dst_file);
    utstring_printf(dst_file,
                    "%s/BUILD.bazel",
                    utstring_body(dst_dir));

    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_runtime.BUILD"), */
    /*                dst_file); */

    write_buf(ocamlsdk_runtime_BUILD,
              ocamlsdk_runtime_BUILD_len,
              utstring_body(dst_file));

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/sys",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/sys/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_runtime_sys_BUILD,
              ocamlsdk_runtime_sys_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/runtime/runtime_sys.BUILD"), */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/vm",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/vm/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_runtime_vm_BUILD,
              ocamlsdk_runtime_vm_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/runtime/runtime_vm.BUILD"), */
    /*                dst_file); */

    TRACE_EXIT;
}

void _symlink_ocaml_runtime(char *switch_lib, char *tgtdir)
{
    TRACE_ENTRY;
    LOG_DEBUG(0, "rt switchlib: %s", switch_lib);
    LOG_DEBUG(0, "rt tgtdir: %s", tgtdir);
    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml",
                    switch_lib);
    /* utstring_body(opam_switch_lib)); */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        perror(utstring_body(opamdir));
        log_error(RED "Unable to opendir for symlinking stdlib" CRESET);
        exit(EXIT_FAILURE);
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        /* LOG_DEBUG(0, "DIRENT"); */
        if(direntry->d_type==DT_REG){
            if (strncmp("stdlib", direntry->d_name, 6) != 0)
                if (strncmp("std_exit", direntry->d_name, 8) != 0)
                    continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir,
                            direntry->d_name);
            if (verbosity > log_symlinks) {
                fprintf(INFOFD, GRN "INFO" CRESET
                        " symlink: %s -> %s\n",
                        utstring_body(src),
                        utstring_body(dst));
            }

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
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
    TRACE_EXIT;
}

/* ************************************** */
EXPORT void emit_ocaml_stublibs(UT_string *dst_dir,
                                char *switch_pfx)
{
    TRACE_ENTRY;
    UT_string *dst_file;
    utstring_new(dst_file);
    utstring_printf(dst_file,
                    "%s/BUILD.bazel",
                    utstring_body(dst_dir));

    FILE *ostream = fopen(utstring_body(dst_file), "w");
    ostream = fopen(utstring_body(dst_file), "w");
    if (ostream == NULL) {
        perror(utstring_body(dst_file));
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(dst_file));
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n");
    fprintf(ostream, "exports_files(glob([\"**\"]))\n");
    fprintf(ostream,
            "package(default_visibility=[\"//visibility:public\"])\n\n");

    fprintf(ostream, "filegroup(\n");
    fprintf(ostream, "    name = \"stublibs\",\n");
    fprintf(ostream, "    srcs = glob([\"dll*\"], allow_empty=True),\n");
    fprintf(ostream, "    data = glob([\"dll*\"], allow_empty=True),\n");
    fprintf(ostream, ")\n");
    fclose(ostream);
    if (verbosity > log_writes)
        fprintf(INFOFD, GRN "INFO" CRESET
                " wrote: %s\n", utstring_body(dst_file));

    _emit_ocaml_stublibs_symlinks(dst_dir,
                                  switch_pfx,
                                  "ocaml/stublibs");
    utstring_free(dst_file);
    TRACE_EXIT;
}

EXPORT void _emit_ocaml_stublibs_symlinks(UT_string *dst_dir,
                                          char *switch_pfx,
                                          /* char *coswitch_lib, */
                                          char *tgtdir)
{
    TRACE_ENTRY;
    UT_string *src_dir;
    utstring_new(src_dir);
    utstring_printf(src_dir,
                    "%s/lib/%s",
                    switch_pfx,
                    tgtdir);

    /* UT_string *dst_dir; */
    /* utstring_new(dst_dir); */
    /* utstring_printf(dst_dir, */
    /*                 "%s/ocamlsdk/lib/stublibs", */
    /*                 coswitch_lib); */
    /*                 /\* tgtdir); *\/ */
    /* mkdir_r(utstring_body(dst_dir)); */

    LOG_DEBUG(1, "stublibs src_dir: %s", utstring_body(src_dir));
    LOG_DEBUG(1, "stublibs dst_dir: %s", utstring_body(dst_dir));

    UT_string *src_file;
    utstring_new(src_file);
    UT_string *dst_file;
    utstring_new(dst_file);
    int rc;

    LOG_DEBUG(1, "opening src_dir for read: %s",
              utstring_body(src_dir));

    DIR *srcd = opendir(utstring_body(src_dir));
    /* DIR *srcd = opendir(utstring_body(opamdir)); */
    if (srcd == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking stublibs: %s\n",
                utstring_body(src_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(srcd)) != NULL) {
        //Condition to check regular file.
        LOG_DEBUG(1, "stublib: %s, type %d",
                  direntry->d_name, direntry->d_type);
        if( (direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK) ){

            /* do not symlink workspace file */
            if (strncmp(direntry->d_name, "WORKSPACE", 9) == 0) {
                continue;
            }
            utstring_renew(src_file);
            utstring_printf(src_file, "%s/%s",
                            utstring_body(src_dir),
                            direntry->d_name);
            utstring_renew(dst_file);
            utstring_printf(dst_file, "%s/%s",
                            utstring_body(dst_dir),
                            direntry->d_name);

            if (verbosity > log_symlinks) {
                fprintf(INFOFD, GRN "INFO" CRESET
                        " symlink: %s -> %s\n",
                        utstring_body(src_file),
                        utstring_body(dst_file));
            }

            rc = symlink(utstring_body(src_file),
                         utstring_body(dst_file));
            symlink_ct++;
            if (rc != 0) {
                switch (errno) {
                case EEXIST:
                    goto ignore;
                case ENOENT:
                    log_error("symlink ENOENT: %s", strerror(errno));
                    log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir));
                    fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno));
                    fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir));
                    break;
                default:
                    log_error("symlink err: %s", strerror(errno));
                    fprintf(stderr, "symlink err: %s", strerror(errno));
                }
                log_error("Exiting");
                fprintf(stderr, "Error, exiting\n");
                exit(EXIT_FAILURE);
            ignore:
                ;
            }
        }
    }
    closedir(srcd);
    TRACE_EXIT;
}

/**
   <switch>/lib/stublibs - no META file, populated by other pkgs
*/
EXPORT void emit_lib_stublibs_pkg(UT_string *dst_dir,
                                  char *switch_lib)
/* char *switch_stublibs) */
{
    /* (void)registry; */
    TRACE_ENTRY;
    UT_string *switch_stublibs_dir;
    utstring_new(switch_stublibs_dir);
    utstring_printf(switch_stublibs_dir,
                    "%s/stublibs",
                    switch_lib);
    /* switch_stublibs); */
    int rc = access(utstring_body(switch_stublibs_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "no stublibs dir found: %s" CRESET,
                 utstring_body(switch_stublibs_dir));
        return;
    }

    /* UT_string *dst_dir; */
    /* utstring_new(dst_dir); */
    /* utstring_printf(dst_dir, */
    /*                 "%s/stublibs", */
    /*                 coswitch_lib); */
    /* mkdir_r(utstring_body(dst_dir)); */

    UT_string *dst_file;
    utstring_new(dst_file);
    utstring_printf(dst_file,
                    "%s/MODULE.bazel",
                    utstring_body(dst_dir));
    FILE *ostream;
    ostream = fopen(utstring_body(dst_file), "w");
    if (ostream == NULL) {
        log_error("%s", strerror(errno));
        perror(utstring_body(dst_file));
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "## generated file - DO NOT EDIT\n\n");
    fprintf(ostream, "module(\n");
    fprintf(ostream, "    name = \"stublibs\", version = \"%s\",\n",
            default_version);
    fprintf(ostream, "    compatibility_level = %d,\n",
            default_compat);
    fprintf(ostream, ")\n");
    fprintf(ostream, "\n");

    fprintf(ostream, "bazel_dep(name = \"rules_ocaml\",");
    fprintf(ostream, " version = \"%s\")\n", rules_ocaml_version);

    fprintf(ostream,
            "bazel_dep(name = \"ocaml\", version = \"%s\")\n", compiler_version);

    fprintf(ostream,
            "bazel_dep(name = \"bazel_skylib\", version = \"%s\")\n", skylib_version);

    fclose(ostream);
    if (verbosity > log_writes)
        fprintf(INFOFD, GRN "INFO" CRESET
                " wrote: %s\n", utstring_body(dst_file));

    /* utstring_new(dst_file); */
    /* utstring_printf(dst_file, "%s/WORKSPACE.bazel", */
    /*                 utstring_body(dst_dir)); */
    /* emit_workspace_file(dst_file, "stublibs"); */

    /* now BUILD.bazel */
    utstring_renew(dst_file);
    utstring_printf(dst_file, "%s/lib/stublibs",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/lib/stublibs/BUILD.bazel",
                    utstring_body(dst_dir));

    ostream = fopen(utstring_body(dst_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(dst_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(dst_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(dst_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n\n");

    fprintf(ostream,
            "load(\"@bazel_skylib//rules:common_settings.bzl\",\n");
    fprintf(ostream, "     \"string_setting\")\n\n");

    fprintf(ostream,
            "package(default_visibility=[\"//visibility:public\"])\n\n");

    fprintf(ostream, "exports_files(glob([\"**\"]))\n\n");

    fprintf(ostream,
            "string_setting(name = \"path\",\n");
    fprintf(ostream,
            "               build_setting_default = \"%s/stublibs/lib/stublibs\")\n\n",
            utstring_body(dst_dir)
            );

    fprintf(ostream, "filegroup(\n");
    fprintf(ostream, "    name = \"stublibs\",\n");
    fprintf(ostream, "    srcs = glob([\"**\"], allow_empty=True),\n");
    fprintf(ostream, ")\n");
    fclose(ostream);
    if (verbosity > log_writes) {
        fprintf(INFOFD, GRN "INFO" CRESET
                " wrote: %s\n", utstring_body(dst_file));
    }

    /* utstring_renew(dst_file); */
    /* utstring_printf(dst_file, "%s/lib/stublibs", */
    /*                 utstring_body(dst_dir)); */
    _emit_lib_stublibs_symlinks(utstring_body(switch_stublibs_dir),
                                utstring_body(dst_dir));

    utstring_free(dst_file);
    TRACE_EXIT;
}

EXPORT void _emit_lib_stublibs_symlinks(char *switch_stublibs,
                                        char *coswitch_lib)
{
    TRACE_ENTRY;
    UT_string *dst_dir;
    utstring_new(dst_dir);
    /* utstring_printf(dst_dir, coswitch_pfx); */
    utstring_printf(dst_dir, "%s/lib/stublibs",
                    coswitch_lib);
    mkdir_r(utstring_body(dst_dir));

    UT_string *src_dir; // relative to opam_switch_lib
    utstring_new(src_dir);
    utstring_printf(src_dir, "%s", switch_stublibs);
    /* "%s%s", */
    /* utstring_body(opam_switch_lib), */
    /* "/stublibs"); // _dir); */

    /* LOG_DEBUG(0, "libstublibs src_dir: %s\n", utstring_body(src_dir)); */
    /* LOG_DEBUG(0, "libstublibs dst_dir: %s\n", utstring_body(dst_dir)); */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    LOG_DEBUG(0, "opening src_dir for read: %s",
              utstring_body(src_dir));
    DIR *srcd = opendir(utstring_body(src_dir));
    /* DIR *srcd = opendir(utstring_body(opamdir)); */
    if (srcd == NULL) {
        log_error("Unable to opendir for symlinking stublibs: %s",
                  utstring_body(src_dir));
        fprintf(stderr, "Unable to opendir for symlinking stublibs: %s\n",
                utstring_body(src_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
        /* } else { */
        /*     LOG_DEBUG(0, "XXXXXXXXXXXXXXXX"); */
    }

    struct dirent *direntry;
    while ((direntry = readdir(srcd)) != NULL) {
        //Condition to check regular file.
        /* if (coswitch_debug_symlinks) { */
        LOG_DEBUG(1, "stublib: %s, type %d",
                  direntry->d_name, direntry->d_type);
        /* } */
        if (strncmp(direntry->d_name, "WORKSPACE", 9) == 0) {
            continue;
        }

        if( (direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK) ){
            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(src_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(dst_dir), direntry->d_name);
            LOG_DEBUG(1, "stublibs: symlinking %s to %s",
                      utstring_body(src), utstring_body(dst));

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                switch (errno) {
                case EEXIST:
                    goto ignore;
                case ENOENT:
                    log_error("symlink ENOENT: %s", strerror(errno));
                    log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir));
                    fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno));
                    fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir));
                    break;
                default:
                    log_error("symlink err: %s", strerror(errno));
                    fprintf(stderr, "symlink err: %s", strerror(errno));
                }
                log_error("Exiting");
                fprintf(stderr, "Exiting\n");
                exit(EXIT_FAILURE);
            ignore:
                ;
            }
        }
    }
    closedir(srcd);
    TRACE_EXIT;
}

/* *********************************************** */
/* EXPORT void emit_ocaml_platform_buildfiles(/\* UT_string *templates, *\/ */
/*                                     /\* char *runfiles, *\/ */
/*                                     char *coswitch_lib) */
/* { */
/*     TRACE_ENTRY; */
/*     UT_string *dst_file; */
/*     utstring_new(dst_file); */
/*     utstring_printf(dst_file, */
/*                     "%s/ocamlsdk/platform", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_file)); */
/*     utstring_printf(dst_file, "/BUILD.bazel"); */
/*     /\* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/platform/platform.BUILD"), *\/ */
/*     /\*                dst_file); *\/ */

/*     utstring_renew(dst_file); */
/*     utstring_printf(dst_file, "%s/ocamlsdk/platform/arch", coswitch_lib); */
/*     mkdir_r(utstring_body(dst_file)); */
/*     utstring_printf(dst_file, "/BUILD.bazel"); */
/*     copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/platform/arch.BUILD"), */
/*                    dst_file); */

/*     utstring_renew(dst_file); */
/*     utstring_printf(dst_file, "%s/ocamlsdk/platform/executor", coswitch_lib); */
/*     mkdir_r(utstring_body(dst_file)); */
/*     utstring_printf(dst_file, "/BUILD.bazel"); */
/*     copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/platform/executor.BUILD"), */
/*                    dst_file); */

/*     utstring_renew(dst_file); */
/*     utstring_printf(dst_file, "%s/ocamlsdk/platform/emitter", coswitch_lib); */
/*     mkdir_r(utstring_body(dst_file)); */
/*     utstring_printf(dst_file, "/BUILD.bazel"); */
/*     copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/platform/emitter.BUILD"), */
/*                    dst_file); */

/*     utstring_free(dst_file); */
/*     TRACE_EXIT; */
/* } */

EXPORT void emit_ocaml_toolchain_buildfiles(char *obazl_pfx,
                                            UT_string *dst_dir)
{
    TRACE_ENTRY;
    UT_string *dst_file;
    utstring_new(dst_file);

    utstring_printf(dst_file,
                    "%s/selectors/local",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_selector_local_BUILD,
                   ocamlsdk_toolchain_selector_local_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/selectors/local.BUILD"), */
    /*                dst_file); */

#if defined(XCOMPILE_MACOS)
    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/selectors/macos/x86_64",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_selector_macos_x86_64_BUILD,
                   ocamlsdk_toolchain_selector_macos_x86_64_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/selectors/macos/x86_64.BUILD"), */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/selectors/macos/arm",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_selector_macos_arm_BUILD,
                   ocamlsdk_toolchain_selector_macos_arm_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/selectors/macos/arm.BUILD"), */
    /*                dst_file); */
#endif

#if defined(XCOMPILE_LINUX)
    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/selectors/linux/x86_64",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_selector_linux_x86_64_BUILD,
                   ocamlsdk_toolchain_selector_linux_x86_64_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/selectors/linux/x86_64.BUILD"), */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/selectors/linux/arm",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_selector_linux_arm_BUILD,
                   ocamlsdk_toolchain_selector_linux_arm_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/selectors/linux/arm.BUILD"), */
    /*                dst_file); */
#endif

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/profiles",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_profiles_BUILD,
                   ocamlsdk_toolchain_profiles_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/profiles/profiles.BUILD"), */
    /*                dst_file); */

    utstring_new(dst_file);
    utstring_printf(dst_file,
                    "%s/adapters/local",
                    utstring_body(dst_dir));
    /* "%s/ocamlsdk/toolchain/adapters/local", */
    /* coswitch_lib); */
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");

    write_template(obazl_pfx,
                   ocamlsdk_toolchain_adapter_local_BUILD,
                   ocamlsdk_toolchain_adapter_local_BUILD_len,
                   utstring_body(dst_file));

    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/adapters/local.BUILD"), */
    /*                dst_file); */

#if defined(XCOMPILE_LINUX)
    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/adapters/linux/x86_64",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_adapter_linux_x86_64_BUILD,
                   ocamlsdk_toolchain_adapter_linux_x86_64_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/adapters/linux/x86_64.BUILD"), */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/adapters/linux/arm",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_adapter_linux_arm_BUILD,
                   ocamlsdk_toolchain_adapter_linux_arm_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/adapters/linux/arm.BUILD"), */
    /*                dst_file); */
#endif

#if defined(XCOMPILE_MACOS)
    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/adapters/macos/x86_64",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_adapter_macos_x86_64_BUILD,
                   ocamlsdk_toolchain_adapter_macos_x86_64_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/adapters/macos/x86_64.BUILD"), */
    /*                dst_file); */

    utstring_new(dst_file);
    utstring_printf(dst_file,
                    "%s/adapters/macos/arm",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_toolchain_adapter_macos_arm_BUILD,
                   ocamlsdk_toolchain_adapter_macos_arm_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/toolchain/adapters/macos/arm.BUILD"), */
    /*                dst_file); */
#endif
    utstring_free(dst_file);
    TRACE_EXIT;
}

EXPORT void emit_ocaml_bin_dir(char *bld_file)
{
    TRACE_ENTRY;

    FILE *ostream;
    ostream = fopen(bld_file, "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  bld_file);
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                bld_file);
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(dst_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n");
    fprintf(ostream, "exports_files(glob([\"**\"]))\n");
    fclose(ostream);
    if (verbosity > log_writes)
        fprintf(INFOFD, GRN "INFO" CRESET " wrote %s\n", bld_file);

    /* **************************************************************** */
    TRACE_EXIT;
}

EXPORT void _emit_ocaml_bin_symlinks(UT_string *dst_dir,
                                     char *opam_switch_pfx
                                     /* char *coswitch_lib // dest */
                                     )
{
    TRACE_ENTRY;
    /*     LOG_DEBUG(0, "opam pfx: %s", opam_switch_pfx); */
    /*         LOG_DEBUG(0, "dst_dir: %s", utstring_body(dst_dir)); */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    UT_string *opam_switch_bin;
    utstring_new(opam_switch_bin);
    utstring_printf(opam_switch_bin, "%s/bin", opam_switch_pfx);
#if defined(PROFILE_fastbuild)
    if (verbosity > 3)
        LOG_DEBUG(0, "opening src_dir for read: %s",
                  utstring_body(opam_switch_bin));
#endif
    DIR *srcd = opendir(utstring_body(opam_switch_bin));
    /* DIR *srcd = opendir(utstring_body(opamdir)); */
    if (dst == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking toolchain: %s\n",
                utstring_body(opam_switch_bin));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(srcd)) != NULL) {
        //Condition to check regular file.
        if( (direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK) ){

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opam_switch_bin),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(dst_dir), direntry->d_name);

            if (verbosity > log_symlinks)
                fprintf(INFOFD, GRN "INFO" CRESET
                        " symlink: %s -> %s\n",
                        //direntry->d_name);
                        utstring_body(src),
                        utstring_body(dst));

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                switch (errno) {
                case EEXIST:
                    goto ignore;
                case ENOENT:
                    log_error("symlink ENOENT: %s", strerror(errno));
                    log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir));
                    fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno));
                    fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir));
                    break;
                default:
                    log_error("symlink err: %s", strerror(errno));
                    fprintf(stderr, "symlink err: %s", strerror(errno));
                }
                log_error("Exiting");
                fprintf(stderr, "Exiting\n");
                exit(EXIT_FAILURE);
            ignore:
                ;
            }
        }
    }
    closedir(srcd);
    utstring_free(opam_switch_bin);
    TRACE_EXIT;
}

/* **************************************************************** */
/* obsolete but keep it around in case we decide to use it later */
/* void _symlink_buildfile(char *buildfile, UT_string *to_file) */
/* { */
/*     TRACE_ENTRY; */
/*     UT_string *src; */
/*     utstring_new(src); */
/*     utstring_printf(src, */
/*                     "%s/external/%s/coswitch/templates/%s", */
/*                     utstring_body(coswitch_runfiles_root), */
/*                     ws_name, */
/*                     buildfile); */
/*     int rc = access(utstring_body(src), F_OK); */
/*     if (rc != 0) { */
/*         log_error("not found: %s", utstring_body(src)); */
/*         fprintf(stderr, "not found: %s\n", utstring_body(src)); */
/*         return; */
/*     } */

/* #if defined(PROFILE_fastbuild) */
/*     if (coswitch_debug) { */
/*         LOG_DEBUG(0, "c_libs: symlinking %s to %s\n", */
/*                   utstring_body(src), */
/*                   utstring_body(to_file)); */
/*     } */
/* #endif */
/*     errno = 0; */
/*     rc = symlink(utstring_body(src), */
/*                  utstring_body(to_file)); */
/*     symlink_ct++; */
/*     if (rc != 0) { */
/*         switch (errno) { */
/*         case EEXIST: */
/*             goto ignore; */
/*         case ENOENT: */
/*             log_error("symlink ENOENT: %s", strerror(errno)); */
/*             /\* log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir)); *\/ */
/*             fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno)); */
/*             /\* fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir)); *\/ */
/*             break; */
/*         default: */
/*             log_error("symlink err: %s", strerror(errno)); */
/*             fprintf(stderr, "symlink err: %s", strerror(errno)); */
/*         } */
/*         log_error("Exiting"); */
/*         fprintf(stderr, "Exiting\n"); */
/*         exit(EXIT_FAILURE); */
/*     ignore: */
/*         ; */
/*     } */
/*     TRACE_EXIT; */
/* } */

/* ************************************************ */
/*
  pre-v5: <switch-pfx>/lib/bigarray, redirects to lib/ocaml
  v5+: no bigarray subdir anywhere
*/
EXPORT void emit_ocaml_bigarray_pkg(char *runfiles,
                                    char *obazl_pfx,
                                    char *switch_lib,
                                    char *coswitch_lib)
{
    TRACE_ENTRY;

    UT_string *bigarray_dir;
    utstring_new(bigarray_dir);
    utstring_printf(bigarray_dir,
                    "%s/bigarray",
                    switch_lib);
    int rc = access(utstring_body(bigarray_dir), F_OK);
    if (rc != 0) {
#if defined(PROFILE_fastbuild)
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(bigarray_dir));
#endif
        utstring_free(bigarray_dir);
        // v >= 5.0.0 does not include any bigarray archive
        return;
        /* #if defined(PROFILE_fastbuild) */
        /*     } else { */
        /*         LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(bigarray_dir)); */
        /* #endif */
    }

    UT_string *templates;
    utstring_new(templates);
    utstring_printf(templates, "%s/%s",
                    runfiles, "/templates/ocaml");

    UT_string *src_file;
    UT_string *dst_dir;
    UT_string *dst_file;
    utstring_new(src_file);
    utstring_new(dst_dir);
    utstring_new(dst_file);

    utstring_printf(src_file, "%s/%s",
                    utstring_body(templates),
                    "ocaml_bigarray.BUILD");

    utstring_printf(dst_dir,
                    "%s/ocamlsdk/lib/bigarray",
                    coswitch_lib);
    mkdir_r(utstring_body(dst_dir));
    utstring_printf(dst_file, "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_template(obazl_pfx,
                   ocamlsdk_bigarray_BUILD,
                   ocamlsdk_bigarray_BUILD_len,
                   utstring_body(dst_file));

    // if we found <switch>/lib/bigarray,
    // then the files will be in <switch>/lib/ocaml
    // (not <switch>/lib/ocaml/bigarray)
    utstring_renew(bigarray_dir);
    utstring_printf(bigarray_dir,
                    "%s/ocaml",
                    switch_lib);
    _symlink_ocaml_bigarray(bigarray_dir, utstring_body(dst_dir))
        ;

    utstring_free(src_file);
    utstring_free(dst_file);
    utstring_free(templates);
    utstring_free(bigarray_dir);
    TRACE_EXIT;
}

EXPORT void _symlink_ocaml_bigarray(UT_string *bigarray_dir,
                                    char *tgtdir)
{
    TRACE_ENTRY;
    /* #if defined(PROFILE_fastbuild) */
    /*     /\* if (coswitch_debug_symlinks) *\/ */
    /*     if (verbosity > 2) { */
    /*         LOG_DEBUG(0, "src: %s, dst: %s", */
    /*                   utstring_body(bigarray_dir), tgtdir); */
    /*     } */
    /* #endif */

    /* UT_string *opamdir; */
    /* utstring_new(opamdir); */
    /* utstring_printf(opamdir, "%s/ocaml", switch_lib); */
    /*                 /\* utstring_body(opam_switch_lib)); *\/ */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(bigarray_dir));
    if (d == NULL) {
        perror(utstring_body(bigarray_dir));
        log_error(RED "Unable to opendir for symlinking bigarray" CRESET " %s\n",
                  utstring_body(bigarray_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* link files starting with "bigarray" */
            if (strncmp("bigarray", direntry->d_name, 8) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(bigarray_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* LOG_DEBUG(0, "symlinking %s to %s", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    /* perror(utstring_body(src)); */
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

/*
  for toplevel <switch>/lib/compiler-libs.
  all targets aliased to <switch>/lib/ocaml/compiler-libs
*/
EXPORT void emit_compiler_libs_pkg(char *obazl_pfx,
                                   UT_string *dst_dir,
                                   char *coswitch_lib)
{
    TRACE_ENTRY;
    (void)coswitch_lib;
    LOG_DEBUG(0, "clibs dir: %s", utstring_body(dst_dir));
    UT_string *dst_file;
    utstring_new(dst_file);

    /* utstring_printf(dst_file, */
    /*                 "%s/compiler-libs", */
    /*                 utstring_body(dst_dir)); */
    /* mkdir_r(utstring_body(dst_file)); */
    /* utstring_printf(dst_file, "/BUILD.bazel"); */
    /* write_buf(compiler_libs_BUILD, */
    /*           compiler_libs_BUILD_len, */
    /*           utstring_body(dst_file)); */
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/compiler_libs/common.BUILD"), */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/common",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   compiler_libs_common_BUILD,
                   compiler_libs_common_BUILD_len,
                   utstring_body(dst_file));

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/bytecomp",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   compiler_libs_bytecomp_BUILD,
                   compiler_libs_bytecomp_BUILD_len,
                   utstring_body(dst_file));

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/bytecomp",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   compiler_libs_optcomp_BUILD,
                   compiler_libs_optcomp_BUILD_len,
                   utstring_body(dst_file));

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/toplevel",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   compiler_libs_toplevel_BUILD,
                   compiler_libs_toplevel_BUILD_len,
                   utstring_body(dst_file));

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/native-toplevel",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   compiler_libs_native_toplevel_BUILD,
                   compiler_libs_native_toplevel_BUILD_len,
                   utstring_body(dst_file));
    utstring_free(dst_file);

    TRACE_EXIT;
}

/* **************************************************************** */
/* for <switch>/lib/ocaml/compiler-libs */
EXPORT void emit_ocaml_compiler_libs_pkg(char *obazl_pfx,
                                         UT_string *dst_dir,
                                         char *switch_lib)
{
    TRACE_ENTRY;
    UT_string *dst_file;
    utstring_new(dst_file);

    utstring_printf(dst_file,
                    "%s",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_compiler_libs_BUILD,
                   ocamlsdk_compiler_libs_BUILD_len,
                   utstring_body(dst_file));
    /* char *bf = rf_rlocation(BAZEL_CURRENT_REPOSITORY */
    /*       "/templates/ocaml/ocaml_compiler-libs.BUILD"); */
    /* copy_buildfile(bf, */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/common",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_compiler_libs_common_BUILD,
                   ocamlsdk_compiler_libs_common_BUILD_len,
                   utstring_body(dst_file));
    /* char *bf = rf_rlocation(BAZEL_CURRENT_REPOSITORY */
    /* "/templates/ocaml/ocaml_compiler-libs-common.BUILD"); */
    /* copy_buildfile(bf, dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/bytecomp",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_compiler_libs_bytecomp_BUILD,
                   ocamlsdk_compiler_libs_bytecomp_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_compiler-libs-bytecomp.BUILD"), */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/optcomp",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_compiler_libs_optcomp_BUILD,
                   ocamlsdk_compiler_libs_optcomp_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_compiler-libs-optcomp.BUILD"), */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/toplevel",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_compiler_libs_toplevel_BUILD,
                   ocamlsdk_compiler_libs_toplevel_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_compiler-libs-toplevel.BUILD"), */
    /*                dst_file); */

    utstring_renew(dst_file);
    utstring_printf(dst_file,
                    "%s/native-toplevel",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_template(obazl_pfx,
                   ocamlsdk_compiler_libs_native_toplevel_BUILD,
                   ocamlsdk_compiler_libs_native_toplevel_BUILD_len,
                   utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_compiler-libs-native-toplevel.BUILD"), */
    /*                dst_file); */

    _symlink_ocaml_compiler_libs(switch_lib,
                                 /* coswitch_lib); */
                                 utstring_body(dst_dir));

    TRACE_EXIT;
}

void _symlink_ocaml_compiler_libs(char *switch_lib,
                                  char *coswitch_lib)
/* char *tgtdir) */
{
    TRACE_ENTRY;
    LOG_DEBUG(1, "switch_lib: %s, coswitch_lib: %s",
              switch_lib, coswitch_lib); //, tgtdir);
    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/compiler-libs",
                    switch_lib);
    /* utstring_body(opam_switch_lib)); */
    /* LOG_DEBUG(0, "OPAM DIR src: %s", utstring_body(opamdir)); */
    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking compiler-libs: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);

            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            //tgtdir,
                            coswitch_lib,
                            direntry->d_name);
            LOG_DEBUG(2, "symlinking %s to %s",
                      utstring_body(src),
                      utstring_body(dst));
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    perror(utstring_body(src));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

/* ***************************************** */
EXPORT void emit_ocaml_dynlink_pkg(UT_string *dst_dir,
                                   char *switch_lib)
{
    TRACE_ENTRY;
    bool add_toplevel = false;

    UT_string *dynlink_dir;
    utstring_new(dynlink_dir);
    utstring_printf(dynlink_dir,
                    "%s/dynlink",
                    switch_lib);
    int rc = access(utstring_body(dynlink_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(dynlink_dir));
        utstring_new(dynlink_dir);
        utstring_printf(dynlink_dir,
                        "%s/ocaml/dynlink",
                        switch_lib);
        int rc = access(utstring_body(dynlink_dir), F_OK);
        if (rc != 0) {
            /* if (coswitch_trace) */
            LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                     utstring_body(dynlink_dir));
            utstring_free(dynlink_dir);
            return;
        } else {
            LOG_WARN(0, YEL "FOUND: %s" CRESET,
                     utstring_body(dynlink_dir));
            add_toplevel = true;
        }
        /*     } else { */
        /*         LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(dynlink_dir)); */
    }

    UT_string *templates;
    utstring_new(templates);
    /* utstring_printf(templates, "%s/%s", */
    /*                 runfiles, "templates/ocaml"); */

    /* UT_string *src_file; */
    /* utstring_new(src_file); */

    // 5.0.0+ has <switch>/lib/ocaml/dynlink/META
    // pre-5.0.0: no <switch>/lib/ocaml/dynlink
    // always emit <coswitch>/lib/ocaml/dynlink
    /* utstring_printf(src_file, "%s/%s", */
    /*                 utstring_body(templates), */
    /*                 "ocaml_dynlink.BUILD"); */

    UT_string *dst_file;
    utstring_new(dst_file);
    utstring_printf(dst_file, "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_dynlink_BUILD,
              ocamlsdk_dynlink_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_dynlink.BUILD"), */
    /*                dst_file); */

    if (add_toplevel) {
        // not found: <switch>/lib/dynlink
        // means >= 5.0.0
        // add <coswitch>/lib/dynlink for (bazel) compatibility
        _symlink_ocaml_dynlink(dynlink_dir,
                               utstring_body(dst_dir));
        /* _emit_toplevel(templates, "ocaml_dynlink_alias.BUILD", */
        /*                src_file, */
        /*                dst_dir, dst_file, */
        /*                coswitch_lib, "dynlink"); */
    } else {
        // if we found <switch>/lib/dynlink (versions < 5.0.0)
        // then we need to populate <coswitch>/lib/ocamlsdk/dynlink
        // (its already there for >= 5.0.0)
        utstring_new(dynlink_dir);
        utstring_printf(dynlink_dir,
                        "%s/ocaml",
                        switch_lib);
        _symlink_ocaml_dynlink(dynlink_dir,
                               utstring_body(dst_dir));
    }

    utstring_free(dst_file);
    utstring_free(dynlink_dir);
    TRACE_EXIT;
}

void _symlink_ocaml_dynlink(UT_string *dynlink_dir, char *tgtdir)
{
    TRACE_ENTRY;
    LOG_DEBUG(0, "src: %s, dst: %s",
              utstring_body(dynlink_dir), tgtdir);
    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(dynlink_dir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking dynlink: %s\n",
                utstring_body(dynlink_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
        /* } else { */
        /*     if (verbosity > 1) */
        /*         LOG_DEBUG(0, "opened dir %s", utstring_body(dynlink_dir)); */
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* link files starting with "dynlink" */
            if (strncmp("dynlink", direntry->d_name, 7) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(dynlink_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            LOG_DEBUG(0, "symlinking %s to %s",
                      utstring_body(src),
                      utstring_body(dst));
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

/* ***************************************** */
EXPORT void emit_ocaml_num_pkg(UT_string *dst_dir,
                               char *switch_lib)
{ /* only if opam 'nums' pseudo-pkg was installed */
    TRACE_ENTRY;

    UT_string *num_dir;
    utstring_new(num_dir);
    utstring_printf(num_dir,
                    "%s/num",
                    switch_lib);
    int rc = access(utstring_body(num_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(num_dir));
        return;
        /*     } else { */
        /*         LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(num_dir)); */
    }

    /*     utstring_new(dst_file); */
    /*     utstring_printf(dst_file, "%s/num/META", switch_lib); */
    /*     rc = access(utstring_body(dst_file), F_OK); */
    /*     if (rc != 0) { */
    /* #if defined(PROFILE_fastbuild) */
    /*         if (coswitch_trace) log_trace(YEL "num pkg not installed" CRESET); */
    /* #endif */
    /*         return; */
    /*     } */

    UT_string *dst_file;
    utstring_new(dst_file);
    utstring_printf(dst_file,
                    "%s/num/core",
                    utstring_body(dst_dir));
    mkdir_r(utstring_body(dst_file));
    utstring_printf(dst_file, "/BUILD.bazel");
    write_buf(ocamlsdk_num_BUILD,
              ocamlsdk_num_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_num.BUILD"), */
    /*                dst_file); */

    _symlink_ocaml_num(switch_lib, utstring_body(dst_file));

    utstring_free(dst_file);
    TRACE_EXIT;
}

void _symlink_ocaml_num(char *switch_lib, char *tgtdir)
{
    TRACE_ENTRY;

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", switch_lib);
    /* utstring_body(opam_switch_lib)); */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        perror(utstring_body(opamdir));
        log_error("Unable to opendir for symlinking num: %s\n",
                  utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }
    LOG_DEBUG(0, "reading num dir %s", utstring_body(opamdir));

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "num") == NULL) */
            if (strncmp("nums.", direntry->d_name, 5) != 0)
                if (strncmp("libnums.", direntry->d_name, 8) != 0)
                    continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* if (coswitch_debug_symlinks) */
            LOG_DEBUG(0, "symlinking %s to %s\n",
                      utstring_body(src),
                      utstring_body(dst));
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
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
    TRACE_EXIT;
}

/* ***************************************** */
EXPORT void emit_ocaml_profiling_pkg(UT_string *dst_dir,
                                     char *switch_lib)
{
    TRACE_ENTRY;
    /* bool add_toplevel = false; */

    UT_string *profiling_dir;
    utstring_new(profiling_dir);
    utstring_printf(profiling_dir,
                    "%s/profiling",
                    switch_lib);
    int rc = access(utstring_body(profiling_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(profiling_dir));
        utstring_new(profiling_dir);
        utstring_printf(profiling_dir,
                        "%s/ocaml/profiling",
                        switch_lib);
        int rc = access(utstring_body(profiling_dir), F_OK);
        if (rc != 0) {
            /* if (coswitch_trace) */
            LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                     utstring_body(profiling_dir));
            utstring_free(profiling_dir);
            return;
        } else {
            LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(profiling_dir));
            /* add_toplevel = true; */
        }
    }

    UT_string *dst_file;
    utstring_new(dst_file);
    utstring_printf(dst_file, "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_profiling_BUILD,
              ocamlsdk_profiling_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_profiling.BUILD"), */
    /*                dst_file); */

    _symlink_ocaml_profiling(profiling_dir,
                             utstring_body(dst_dir));

    /*     _emit_toplevel(templates, "ocaml_profiling_alias.BUILD", */
    /*                    src_file, */
    /*                    dst_dir, dst_file, */
    /*                    coswitch_lib, "runtime_events"); */
    /* } else { */
    /*     // if we found <switch>/lib/profiling (versions < 5.0.0) */
    /*     // then we need to populate <coswitch>/lib/ocaml/profiling */
    /*     // (its already there for >= 5.0.0) */
    /*     utstring_new(profiling_dir); */
    /*     utstring_printf(profiling_dir, */
    /*                     "%s/ocaml", */
    /*                     switch_lib); */
    /*     _symlink_ocaml_profiling(profiling_dir, */
    /*                            utstring_body(dst_dir)); */
    /* } */

    /* utstring_free(src_file); */
    utstring_free(dst_file);
    utstring_free(profiling_dir);
    TRACE_EXIT;
}

void _symlink_ocaml_profiling(UT_string *profiling_dir, char *tgtdir)
{
    TRACE_ENTRY;
    if (verbosity > 2) {
        LOG_DEBUG(0, "src: %s, dst: %s",
                  utstring_body(profiling_dir), tgtdir);
    }
    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(profiling_dir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking profiling: %s\n",
                utstring_body(profiling_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
        /* } else { */
        /*     if (verbosity > 1) */
        /*         LOG_DEBUG(0, "opened dir %s", utstring_body(profiling_dir)); */
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* link files starting with "profiling" */
            if (strncmp("profiling", direntry->d_name, 9) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(profiling_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* LOG_DEBUG(0, "symlinking %s to %s", */
            /*           utstring_body(src), */
            /*           utstring_body(dst)); */
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

/* ***************************************** */
EXPORT void emit_ocaml_rtevents_pkg(UT_string *dst_dir,
                                    char *switch_lib)
/* char *coswitch_lib) */
{
    TRACE_ENTRY;
    bool add_toplevel = false;

    UT_string *rtevents_dir;
    utstring_new(rtevents_dir);
    utstring_printf(rtevents_dir,
                    "%s/runtime_events",
                    switch_lib);
    int rc = access(utstring_body(rtevents_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(rtevents_dir));
        utstring_new(rtevents_dir);
        utstring_printf(rtevents_dir,
                        "%s/ocaml/runtime_events",
                        switch_lib);
        int rc = access(utstring_body(rtevents_dir), F_OK);
        if (rc != 0) {
            /* if (coswitch_trace) */
            LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                     utstring_body(rtevents_dir));
            utstring_free(rtevents_dir);
            return;
        } else {
            LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(rtevents_dir));
            add_toplevel = true;
        }
        /*     } else { */
        /*         LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(rtevents_dir)); */
    }

    /* UT_string *templates; */
    /* utstring_new(templates); */
    /* utstring_printf(templates, "%s/%s", */
    /*                 runfiles, "templates/ocaml"); */

    UT_string *dst_file;
    utstring_new(dst_file);

    // 5.0.0+ has <switch>/lib/ocaml/rtevents/META
    // pre-5.0.0: no <switch>/lib/ocaml/rtevents
    // always emit <coswitch>/lib/ocaml/rtevents
    utstring_printf(dst_file,
                    "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_rtevents_BUILD,
              ocamlsdk_rtevents_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_rtevents.BUILD"), */
    /*                dst_file); */

    if (add_toplevel) {
        // not found: <switch>/lib/rtevents
        // means >= 5.0.0
        // add <coswitch>/lib/rtevents for (bazel) compatibility
        _symlink_ocaml_rtevents(rtevents_dir,
                                utstring_body(dst_dir));
        /* _emit_toplevel(templates, "ocaml_rtevents_alias.BUILD", */
        /*                src_file, */
        /*                dst_dir, dst_file, */
        /*                coswitch_lib, "runtime_events"); */
    } else {
        // if we found <switch>/lib/rtevents (versions < 5.0.0)
        // then we need to populate <coswitch>/lib/ocaml/rtevents
        // (its already there for >= 5.0.0)
        utstring_new(rtevents_dir);
        utstring_printf(rtevents_dir,
                        "%s/ocaml",
                        switch_lib);
        _symlink_ocaml_rtevents(rtevents_dir,
                                utstring_body(dst_dir));
    }

    utstring_free(dst_file);
    utstring_free(rtevents_dir);
    TRACE_EXIT;
}

void _symlink_ocaml_rtevents(UT_string *rtevents_dir, char *tgtdir)
{
    TRACE_ENTRY;
    if (verbosity > 2) {
        LOG_DEBUG(0, "src: %s, dst: %s",
                  utstring_body(rtevents_dir), tgtdir);
    }
    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(rtevents_dir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking rtevents: %s\n",
                utstring_body(rtevents_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
        /* } else { */
        /*     if (verbosity > 1) */
        /*         LOG_DEBUG(0, "opened dir %s", utstring_body(rtevents_dir)); */
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* link files starting with "rtevents" */
            if (strncmp("runtime_events", direntry->d_name, 7) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(rtevents_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* LOG_DEBUG(0, "symlinking %s to %s", */
            /*           utstring_body(src), */
            /*           utstring_body(dst)); */
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

/* ************************************* */
EXPORT void emit_ocaml_str_pkg(UT_string *dst_dir,
                               char *switch_lib)
{
    TRACE_ENTRY;

    // add_toplevel true means ocaml version >= 5.0.0
    // which means switch lacks lib/str
    // but we add it to coswitch so @str//lib/str will
    // work with >= 5.0.0
    bool add_toplevel = false;

    UT_string *str_dir;
    utstring_new(str_dir);
    utstring_printf(str_dir,
                    "%s/str",
                    switch_lib);
    int rc = access(utstring_body(str_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(str_dir));
        utstring_new(str_dir);
        utstring_printf(str_dir,
                        "%s/ocaml/str",
                        switch_lib);
        int rc = access(utstring_body(str_dir), F_OK);
        if (rc != 0) {
            /* if (coswitch_trace) */
            LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                     utstring_body(str_dir));
            utstring_free(str_dir);
            return;
        } else {
            // found lib/ocaml/str
            LOG_WARN(0, YEL "FOUND: %s" CRESET,
                     utstring_body(str_dir));
            add_toplevel = true;
        }
    } else {
        LOG_WARN(0, YEL "FOUND: %s" CRESET,
                 utstring_body(str_dir));
    }

    UT_string *dst_file;
    utstring_new(dst_file);

    if (add_toplevel) {
        // we found lib/ocaml/str, so we need to emit the
        // toplevel w/alias, lib/str
        // but we only symlink to lib/ocamlsdk/lib/str
        /* LOG_DEBUG(0, "EMITTING STR TOPLEVEL"); */
        /* utstring_printf(dst_dir, */
        /*                 "%s/str", */
        /*                 coswitch_lib); */
        /* mkdir_r(utstring_body(dst_dir)); */
        utstring_printf(dst_file, "%s/BUILD.bazel",
                        utstring_body(dst_dir));
        /* _emit_toplevel(templates, */
        /*                "ocaml_str_alias.BUILD", */
        /*                src_file, */
        /*                dst_dir, dst_file, */
        /*                coswitch_lib, */
        /*                "str"); */

        // and also lib/ocamlsdk/str
        /* utstring_renew(src_file); */
        /* utstring_printf(src_file, "%s/%s", */
        /*                 utstring_body(templates), */
        /*                 "ocaml_str.BUILD"); */
        /* utstring_renew(dst_dir); */
        /* utstring_printf(dst_dir, */
        /*                 "%s/ocamlsdk/lib/str", */
        /*                 coswitch_lib); */
        mkdir_r(utstring_body(dst_dir));
        utstring_renew(dst_file);
        utstring_printf(dst_file,
                        "%s/BUILD.bazel",
                        utstring_body(dst_dir));
        write_buf(ocamlsdk_str_BUILD,
                  ocamlsdk_str_BUILD_len,
                  utstring_body(dst_file));
        /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_str.BUILD"), */
        /*                dst_file); */

        // but symlink to lib/ocaml/str
        _symlink_ocaml_str(str_dir, utstring_body(dst_dir));

    } else {
        // if we found <switch>/lib/str (versions < 5.0.0)
        // then we need to link
        // from <coswitch>/lib/ocaml
        // to <coswitch>/lib/ocaml/lib/str
        /* utstring_printf(dst_dir, */
        /*                 "%s/ocamlsdk/lib/str", */
        /*                 coswitch_lib); */
        mkdir_r(utstring_body(dst_dir));
        utstring_printf(dst_file, "%s/BUILD.bazel",
                        utstring_body(dst_dir));
        write_buf(ocamlsdk_str_BUILD,
                  ocamlsdk_str_BUILD_len,
                  utstring_body(dst_file));
        /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_str.BUILD"), */
        /*                dst_file); */

        utstring_new(str_dir);
        utstring_printf(str_dir,
                        "%s/ocaml",
                        switch_lib);
        _symlink_ocaml_str(str_dir,
                           utstring_body(dst_dir));

    }

    /* utstring_free(src_file); */
    utstring_free(dst_file);
    /* utstring_free(templates); */
    utstring_free(str_dir);
    TRACE_EXIT;
}

void _symlink_ocaml_str(UT_string *str_dir, char *tgtdir)
{
    TRACE_ENTRY;
    if (verbosity > 3) {
        LOG_DEBUG(0, "src: %s,\n\tdst %s",
                  utstring_body(str_dir),
                  tgtdir);
    }

    /* UT_string *opamdir; */
    /* utstring_new(opamdir); */
    /* utstring_printf(opamdir, "%s/ocaml", switch_lib); */
    /*                 /\* utstring_body(opam_switch_lib)); *\/ */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(str_dir));
    if (d == NULL) {
        log_error("Unable to opendir for symlinking str: %s\n",
                  utstring_body(str_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
        /* } else { */
        /*     if (verbosity > 1) */
        /*         LOG_DEBUG(0, "opened dir %s", utstring_body(str_dir)); */
    } else {
        LOG_DEBUG(0, "fopened %s as symlink src",
                  utstring_body(str_dir));
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        /* LOG_DEBUG(0, "direntry: %s", direntry->d_name); */
        if(direntry->d_type==DT_REG){
            /* link files starting with "str." */
            if (strncmp("str.", direntry->d_name, 4) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(str_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* LOG_DEBUG(0, "symlinking %s to %s", */
            /*           utstring_body(src), */
            /*           utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

/* ***************************************** */
/**
   pre-5.0.0: <switch>/threads and <switch>/lib/ocaml/threads
   5.0.0+:    <switch>/lib/ocaml/threads only
*/
EXPORT void emit_ocaml_threads_pkg(UT_string *dst_dir,
                                   char *switch_lib)
{
    TRACE_ENTRY;
    bool add_toplevel = false;

    UT_string *threads_dir;
    utstring_new(threads_dir);
    utstring_printf(threads_dir,
                    "%s/threads",
                    switch_lib);
    int rc = access(utstring_body(threads_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(threads_dir));
        utstring_new(threads_dir);
        utstring_printf(threads_dir,
                        "%s/ocaml/threads",
                        switch_lib);
        int rc = access(utstring_body(threads_dir), F_OK);
        if (rc != 0) {
            /* if (coswitch_trace) */
            LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                     utstring_body(threads_dir));
            utstring_free(threads_dir);
            return;
        } else {
            LOG_WARN(0, YEL "FOUND: %s" CRESET,
                     utstring_body(threads_dir));
            add_toplevel = true;
        }
        /*     } else { */
        /*         LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(threads_dir)); */
    }

    // 5.0.0+ has <switch>/lib/ocaml/threads
    // pre-5.0.0: no <switch>/lib/ocaml/threads
    // always emit <coswitch>/lib/ocaml/threads
    UT_string *dst_file;
    utstring_new(dst_file);

    utstring_printf(dst_file, "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_threads_BUILD,
              ocamlsdk_threads_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_threads.BUILD"), */
    /*                dst_file); */

    if (add_toplevel) {
        // not found: <switch>/lib/threads
        // means >= 5.0.0
        // add <coswitch>/lib/threads for (bazel) compatibility
        /* _emit_toplevel(templates, */
        /*                "ocaml_threads_alias.BUILD", */
        /*                src_file, */
        /*                dst_dir, dst_file, */
        /*                coswitch_lib, */
        /*                "threads"); */
    }
    // all versions have <switch>/lib/ocaml/threads
    // so we always symlink to <coswitch>/lib/ocaml/threads
    utstring_new(threads_dir);
    utstring_printf(threads_dir,
                    "%s/ocaml",
                    switch_lib);
    /* utstring_renew(dst_dir); */
    /* utstring_printf(dst_dir, */
    /*                 "%s/ocamlsdk/lib/threads", */
    /*                 coswitch_lib); */

    // symlink <switch>/lib/ocaml/threads
    _symlink_ocaml_threads(threads_dir, utstring_body(dst_dir));
    // symlink <switch>/lib/ocaml/libthreads.s, libthreadsnat.a

    utstring_free(dst_file);
    utstring_free(threads_dir);
    TRACE_EXIT;
}

void _symlink_ocaml_threads(UT_string *threads_dir, char *tgtdir)
{
    TRACE_ENTRY;
    if (verbosity > 2) {
        LOG_DEBUG(0, "  src: %s", utstring_body(threads_dir));
        LOG_DEBUG(0, "  dst: %s", tgtdir);
    }
    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    utstring_printf(src, "%s/libthreads.a",
                    utstring_body(threads_dir));
    //FIXME: verify access
    utstring_printf(dst, "%s/libthreads.a",
                    tgtdir);
    if (verbosity > log_symlinks)
        fprintf(INFOFD, GRN "INFO" CRESET
                " symlink: %s -> %s\n",
                utstring_body(src),
                utstring_body(dst));
    rc = symlink(utstring_body(src),
                 utstring_body(dst));
    symlink_ct++;
    if (rc != 0) {
        if (errno != EEXIST) {
            log_error("ERROR: %s", strerror(errno));
            log_error("src: %s, dst: %s",
                      utstring_body(src),
                      utstring_body(dst));
            log_error("exiting");
            exit(EXIT_FAILURE);
        }
    }

    utstring_renew(src);
    utstring_printf(src, "%s/libthreadsnat.a",
                    utstring_body(threads_dir));
    //FIXME: verify access
    utstring_renew(dst);
    utstring_printf(dst, "%s/libthreadsnat.a",
                    tgtdir);
    if (verbosity > log_symlinks) {
        fprintf(INFOFD, GRN "INFO" CRESET
                " symlink: %s -> %s\n",
                utstring_body(src),
                utstring_body(dst));
    }
    rc = symlink(utstring_body(src),
                 utstring_body(dst));
    symlink_ct++;
    if (rc != 0) {
        if (errno != EEXIST) {
            log_error("ERROR: %s", strerror(errno));
            log_error("src: %s, dst: %s",
                      utstring_body(src),
                      utstring_body(dst));
            log_error("exiting");
            exit(EXIT_FAILURE);
        }
    }

    utstring_printf(threads_dir, "/%s", "threads");
    DIR *d = opendir(utstring_body(threads_dir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking threads: %s\n",
                utstring_body(threads_dir));
        log_error("Unable to opendir for symlinking threads: %s\n", utstring_body(threads_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(threads_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

/* ***************************************** */
//FIXMEFIXME
EXPORT void emit_ocaml_ocamldoc_pkg(UT_string *dst_dir,
                                    char *switch_lib)
{
    TRACE_ENTRY;

    UT_string *ocamldoc_dir;
    utstring_new(ocamldoc_dir);
    utstring_printf(ocamldoc_dir,
                    "%s/ocamldoc",
                    switch_lib);
    int rc = access(utstring_body(ocamldoc_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(ocamldoc_dir));
        utstring_new(ocamldoc_dir);
        utstring_printf(ocamldoc_dir,
                        "%s/ocaml/ocamldoc",
                        switch_lib);
        int rc = access(utstring_body(ocamldoc_dir), F_OK);
        if (rc != 0) {
            /* if (coswitch_trace) */
            LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                     utstring_body(ocamldoc_dir));
            utstring_free(ocamldoc_dir);
            return;
        } else {
            LOG_WARN(0, YEL "FOUND: %s" CRESET,
                     utstring_body(ocamldoc_dir));
        }
    }

    UT_string *dst_file;
    utstring_new(dst_file);

    utstring_printf(dst_file, "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_ocamldoc_BUILD,
              ocamlsdk_ocamldoc_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_ocamldoc.BUILD"), */
    /*                dst_file); */

    _symlink_ocaml_ocamldoc(ocamldoc_dir, utstring_body(dst_dir));

    utstring_free(dst_file);
    utstring_free(ocamldoc_dir);
    TRACE_EXIT;
}

void _symlink_ocaml_ocamldoc(UT_string *ocamldoc_dir,
                             char *tgtdir)
{
    TRACE_ENTRY;

    /* UT_string *opamdir; */
    /* utstring_new(opamdir); */
    /* utstring_printf(opamdir, "%s/ocaml/ocamldoc", switch_lib); */
    /*                 /\* utstring_body(opam_switch_lib)); *\/ */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(ocamldoc_dir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking ocamldoc: %s\n",
                utstring_body(ocamldoc_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
        /* } else { */
        /*     if (verbosity > 1) */
        /*         LOG_DEBUG(0, "opened dir %s", utstring_body(ocamldoc_dir)); */
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "ocamldoc") == NULL) */
            /* if (strncmp("odoc", direntry->d_name, 4) != 0) */
            /*     continue; */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(ocamldoc_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* LOG_DEBUG(0, "ocamldoc symlinking %s to %s", */
            /*           utstring_body(src), */
            /*           utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

EXPORT void emit_ocaml_unix_pkg(UT_string *dst_dir,
                                char *switch_lib)
{
    TRACE_ENTRY;
    bool add_toplevel = false;

    UT_string *unix_dir;
    utstring_new(unix_dir);
    utstring_printf(unix_dir,
                    "%s/unix",
                    switch_lib);

    int rc = access(utstring_body(unix_dir), F_OK);
    if (rc != 0) {
        /* if (coswitch_trace) */
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                 utstring_body(unix_dir));
        utstring_new(unix_dir);
        utstring_printf(unix_dir,
                        "%s/ocaml/unix",
                        switch_lib);
        int rc = access(utstring_body(unix_dir), F_OK);
        if (rc != 0) {

            /* if (coswitch_trace) */
            LOG_WARN(0, YEL "NOT FOUND: %s" CRESET,
                     utstring_body(unix_dir));
            utstring_free(unix_dir);
            return;
        } else {
            LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(unix_dir));
            /* did not find <switch>/unix (< 5.0) */
            /* found <switch>/ocaml/unix meaning >= 5.0 */
            add_toplevel = true;
        }
    }
    UT_string *dst_file;
    utstring_new(dst_file);

    // 5.0.0+ has <switch>/lib/ocaml/unix
    // pre-5.0.0: no <switch>/lib/ocaml/unix
    // always emit <coswitch>/lib/ocaml/lib/unix,
    // even for < 5.0.0, for obazl compat
    utstring_printf(dst_file,
                    "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_unix_BUILD,
              ocamlsdk_unix_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_unix.BUILD"), */
    /*                dst_file); */

    if (add_toplevel) {
        // found <switch>/lib/ocaml/unix, not found <switch>/lib/unix, means >= 5.0.0
        // link the former,
        // add <coswitch>/lib/unix alias for (bazel) compat
        // link files in lib/ocaml/unix
        _symlink_ocaml_unix(unix_dir,
                            utstring_body(dst_dir));

        // link lib/ocaml/libunixbyt.a, libunixnat.a
        _symlink_libunix(switch_lib, dst_dir, dst_file, symlink_ct);

        /* _emit_toplevel(templates, */
        /*                "ocaml_unix_alias.BUILD", */
        /*                src_file, */
        /*                dst_dir, dst_file, */
        /*                coswitch_lib, */
        /*                "unix"); */
    } else {
        // if we found <switch>/lib/unix (versions < 5.0.0)
        // then we need to link to <coswitch>/lib/ocaml/unix
        // (its already there for >= 5.0.0)
        utstring_new(unix_dir);
        utstring_printf(unix_dir,
                        "%s/ocaml",
                        switch_lib);
        _symlink_ocaml_unix(unix_dir, utstring_body(dst_dir));
    }

    utstring_free(dst_file);
    utstring_free(unix_dir);
    TRACE_EXIT;
}

void _symlink_libunix(char *switch_lib,
                      UT_string *dst_dir, UT_string *dst_file,
                      int symlink_ct)
{
    TRACE_ENTRY;
    (void)symlink_ct;
    /* first try v >= 5.1 */
    UT_string *src_file;
    utstring_new(src_file);
    utstring_printf(src_file, "%s/ocaml/libunixnat.a",
                    switch_lib);
    int rc = access(utstring_body(src_file), F_OK);
    if (rc == 0) {
#if defined(PROFILE_fastbuild)
        LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(src_file));
#endif
        utstring_renew(dst_file);
        utstring_printf(dst_file, "%s/libunixnat.a",
                        utstring_body(dst_dir));
        /* LOG_DEBUG(0, "SYMLINKING %s to %s\n", */
        /*           utstring_body(src_file), */
        /*           utstring_body(dst_file)); */
        rc = symlink(utstring_body(src_file),
                     utstring_body(dst_file));
        if (rc != 0) {
            if (errno != EEXIST) {
                log_error("ERROR: %s", strerror(errno));
                log_error("src: %s, dst: %s",
                          utstring_body(src_file),
                          utstring_body(dst_file));
                log_error("exiting");
                exit(EXIT_FAILURE);
            } else {
                /* ??? */
            }
        }
        symlink_ct++;
        /* now libunixbyt.a */
        utstring_renew(src_file);
        utstring_printf(src_file, "%s/ocaml/libunixbyt.a",
                        switch_lib);
        int rc = access(utstring_body(src_file), F_OK);
        if (rc == 0) {
#if defined(PROFILE_fastbuild)
            LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(src_file));
#endif
            utstring_renew(dst_file);
            utstring_printf(dst_file, "%s/libunixbyt.a",
                            utstring_body(dst_dir));
            /* LOG_DEBUG(0, "SYMLINKING %s to %s\n", */
            /*           utstring_body(src_file), */
            /*           utstring_body(dst_file)); */
            rc = symlink(utstring_body(src_file),
                         utstring_body(dst_file));
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src_file),
                              utstring_body(dst_file));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                } else {
                    /* ??? */
                }
            }
            symlink_ct++;
        } else {
            /* libunixbyt.a not found after libunixnat.a found */
#if defined(PROFILE_fastbuild)
            LOG_WARN(0, YEL "FOUND: %s" CRESET, utstring_body(src_file));
#endif
        }
    } else {
        /* libunixnat.a not found, try libunix.a (pre 5.1.0) */
#if defined(PROFILE_fastbuild)
        LOG_WARN(0, YEL "NOT FOUND: %s" CRESET, utstring_body(src_file));
#endif
        utstring_renew(src_file);
        utstring_printf(src_file, "%s/ocaml/libunix.a",
                        switch_lib);
        int rc = access(utstring_body(src_file), F_OK);
        if (rc == 0) {
            utstring_renew(dst_file);
            utstring_printf(dst_file, "%s/libunix.a",
                            utstring_body(dst_dir));
            /* LOG_DEBUG(0, "SYMLINKING %s to %s\n", */
            /*           utstring_body(src_file), */
            /*           utstring_body(dst_file)); */
            rc = symlink(utstring_body(src_file),
                         utstring_body(dst_file));
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src_file),
                              utstring_body(dst_file));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                } else {
                    /* ??? */
                }
            }
            symlink_ct++;
        } else {
            /* libunix.a NOT found */
        }
        /* libunixnat.a NOT found */
    }
}

void _symlink_ocaml_unix(UT_string *unix_dir, char *tgtdir)
{
    TRACE_ENTRY_MSG("src: %s", unix_dir);
    /* LOG_DEBUG(0, "unix_dir: %s", utstring_body(unix_dir)); */
    /* LOG_DEBUG(0, "tgtdir: %s", tgtdir); */
    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(unix_dir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking unix: %s\n",
                utstring_body(unix_dir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* link files starting with "unix." etc. */
            if (strncmp("unix.", direntry->d_name, 5) != 0)
                if (strncmp("unixLabels.", direntry->d_name, 11) != 0)
                    if (strncmp("libunix", direntry->d_name, 7) != 0)
                        continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(unix_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* LOG_DEBUG(0, "symlinking %s to %s", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
    TRACE_EXIT;
}

/* ***************************************** */
EXPORT void emit_ocaml_c_ffi_pkg(UT_string *dst_dir,
                                 char *switch_lib)
{
    TRACE_ENTRY;
    (void)switch_lib;

    UT_string *dst_file;
    utstring_new(dst_file);
    utstring_printf(dst_file,
                    "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    write_buf(ocamlsdk_c_ffi_BUILD,
              ocamlsdk_c_ffi_BUILD_len,
              utstring_body(dst_file));
    /* copy_buildfile(rf_rlocation(BAZEL_CURRENT_REPOSITORY "/templates/ocaml/ocaml_c_ffi.BUILD"), */
    /*                dst_file); */

    _symlink_ocaml_c_hdrs(switch_lib, utstring_body(dst_dir));

    utstring_free(dst_file);
    TRACE_EXIT;
}

void _symlink_ocaml_c_hdrs(char *switch_lib, char *tgtdir)
{
    TRACE_ENTRY;
    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/caml", switch_lib);

    UT_string *obazldir;
    utstring_new(obazldir);
    utstring_printf(obazldir, "%s/caml", tgtdir);
    mkdir_r(utstring_body(obazldir));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking c ffi: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if (direntry->d_type==DT_REG){
            /* if (strncmp("lib", direntry->d_name, 3) != 0) */
            /*     continue;       /\* no match *\/ */
            //FIXME: check for .h?

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(obazldir),
                            direntry->d_name);

            if (verbosity > log_symlinks) {
                fprintf(INFOFD, GRN "INFO" CRESET
                        " symlink: %s -> %s\n",
                        utstring_body(src),
                        utstring_body(dst));
            }
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    log_error("ERROR: %s", strerror(errno));
                    log_error("src: %s, dst: %s",
                              utstring_body(src),
                              utstring_body(dst));
                    log_error("exiting");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

void _symlink_ocaml_runtime_libs(char *switch_lib, char *tgtdir)
{
    TRACE_ENTRY;
#if defined(PROFILE_fastbuild)
    LOG_DEBUG(0, "\tswitch_lib: %s\n", switch_lib);
    LOG_DEBUG(0, "\ttgtdir    : %s\n", tgtdir);
#endif

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", switch_lib);

    UT_string *obazldir;
    utstring_new(obazldir);
    utstring_printf(obazldir, "%s", tgtdir);
    mkdir_r(utstring_body(obazldir));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking c_libs: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            if (strncmp("lib", direntry->d_name, 3) != 0) {
                if (strncmp("stdlib.a",
                            direntry->d_name, 8) != 0) {
                    continue;
                }
            }
            /* if (strncmp("libasm", direntry->d_name, 6) != 0) { */
            /*     if (strncmp("libcaml", */
            /*                 direntry->d_name, 7) != 0) { */
            /*         continue;       /\* no match *\/ */
            /*     } */
            /* } */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(obazldir), /* tgtdir, */
                            direntry->d_name);
            LOG_DEBUG(1, "c_libs: symlinking %s to %s",
                      utstring_body(src), utstring_body(dst));
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    fprintf(stdout, "c_libs symlink errno: %d: %s\n",
                            errno, strerror(errno));
                    fprintf(stdout, "c_libs src: %s, dst: %s\n",
                            utstring_body(src),
                            utstring_body(dst));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

void _symlink_ocaml_runtime_compiler_libs(char *switch_lib, char *tgtdir)
{
    TRACE_ENTRY;
    LOG_DEBUG(0, "\tswitch_lib: %s\n", switch_lib);
    LOG_DEBUG(0, "\ttgtdir    : %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/compiler-libs", switch_lib);

    UT_string *obazldir;
    utstring_new(obazldir);
    utstring_printf(obazldir, "%s", tgtdir);
    mkdir_r(utstring_body(obazldir));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking c_libs: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            if (strncmp("ocaml", direntry->d_name, 5) != 0) {
                continue;
            }
            /* if (strncmp("libasm", direntry->d_name, 6) != 0) { */
            /*     if (strncmp("libcaml", */
            /*                 direntry->d_name, 7) != 0) { */
            /*         continue;       /\* no match *\/ */
            /*     } */
            /* } */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(obazldir), /* tgtdir, */
                            direntry->d_name);
            LOG_DEBUG(1, "c_libs: symlinking %s to %s",
                      utstring_body(src),
                      utstring_body(dst));
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            symlink_ct++;
            if (rc != 0) {
                if (errno != EEXIST) {
                    fprintf(stdout, "c_libs symlink errno: %d: %s\n",
                            errno, strerror(errno));
                    fprintf(stdout, "c_libs src: %s, dst: %s\n",
                            utstring_body(src),
                            utstring_body(dst));
                    fprintf(stderr, "exiting\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    closedir(d);
}

EXPORT void emit_ocaml_version(UT_string *dst_dir,
                               char *coswitch_lib,
                               char *version)
{
    TRACE_ENTRY;
    (void)coswitch_lib;
    UT_string *dst_file;
    utstring_new(dst_file);

    utstring_printf(dst_file,
                    "%s/BUILD.bazel",
                    utstring_body(dst_dir));
    LOG_DEBUG(0, "version file: %s", utstring_body(dst_dir));
    errno = 0;
    FILE *ostream;
    ostream = fopen(utstring_body(dst_file), "w");
    if (ostream == NULL) {
        log_error("fopen fail on %s: %s",
                  utstring_body(dst_file),
                  strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(ostream, "## generated file - DO NOT EDIT\n");

    fprintf(ostream,
            "load(\"@bazel_skylib//rules:common_settings.bzl\", \"string_setting\")\n\n");

    fprintf(ostream,
            "package(default_visibility=[\"//visibility:public\"])\n\n");

    fprintf(ostream,
            "string_setting(name = \"version\",\n"
            "               build_setting_default = \"%s\")\n",
            version);

    fclose(ostream);

    if (verbosity > log_writes) {
        fprintf(INFOFD, GRN "INFO" CRESET
                " wrote: %s\n", utstring_body(dst_file));
    }
    utstring_free(dst_file);
    TRACE_EXIT;
}

/*
  src: an opam <switch>/lib dir
  dst: same as src
  or XDG_DATA_HOME/obazl/opam/<switch>/
  or project-local _opam/
  bootstrap_FILE: remains open so opam pkgs can write to it
*/
/* EXPORT void emit_ocamlsdk_module(UT_string *registry, */
/*                                  char *compiler_version, */
/*                                  struct obzl_meta_package *pkgs, */
/*                                  char *switch_name, */
/*                                  char *switch_pfx, */
/*                                  char *coswitch_lib, */
/*                                  char *runfiles) */
/* { */
/*     TRACE_ENTRY; */
/*     LOG_DEBUG(0, "switch_name: %s", switch_name); */
/* #if defined(PROFILE_fastbuild) */
/*     /\* if (verbosity > 0) { *\/ */
/*         LOG_TRACE(0, BLU "EMIT_ocaml_workspace:" CRESET */
/*                   " switch_pfx:%s, dst: %s", */
/*                   switch_pfx, coswitch_lib); */
/*     /\* } *\/ */
/* #endif */

/*     char *switch_lib = opam_switch_lib(switch_name); */

/*     /\* this emits reg record for both ocaml and stublibs pkgs *\/ */
/*     emit_registry_record(registry, compiler_version, NULL, pkgs); */

/*     UT_string *dst_file; */
/*     utstring_new(dst_file); */
/*     /\* utstring_concat(dst_file, coswitch_lib); // pfx); *\/ */
/*     utstring_printf(dst_file, "%s/ocamlsdk", coswitch_lib); */
/*     mkdir_r(utstring_body(dst_file)); */

/*     /\* utstring_printf(dst_file, "/WORKSPACE.bazel"); *\/ */

/*     /\* FILE *ostream = fopen(utstring_body(dst_file), "w"); *\/ */
/*     /\* if (ostream == NULL) { *\/ */
/*     /\*     log_error("fopen: %s: %s", strerror(errno), *\/ */
/*     /\*               utstring_body(dst_file)); *\/ */
/*     /\*     fprintf(stderr, "fopen: %s: %s", strerror(errno), *\/ */
/*     /\*             utstring_body(dst_file)); *\/ */
/*     /\*     fprintf(stderr, "exiting\n"); *\/ */
/*     /\*     /\\* perror(utstring_body(dst_file)); *\\/ *\/ */
/*     /\*     exit(EXIT_FAILURE); *\/ */
/*     /\* } *\/ */
/*     /\* fprintf(ostream, "# generated file - DO NOT EDIT\n"); *\/ */

/*     /\* fclose(ostream); *\/ */
/*     /\* if (verbosity > log_writes) *\/ */
/*     /\*     fprintf(INFOFD, GRN "INFO" CRESET *\/ */
/*     /\*             " wrote %s\n", utstring_body(dst_file)); *\/ */

/*     // now MODULE.bazel */
/*     utstring_renew(dst_file); */
/*     utstring_printf(dst_file, */
/*                     "%s/ocamlsdk/MODULE.bazel", */
/*                     coswitch_lib); */
/*     FILE *ostream = fopen(utstring_body(dst_file), "w"); */
/*     if (ostream == NULL) { */
/*         log_error("%s", strerror(errno)); */
/*         perror(utstring_body(dst_file)); */
/*         exit(EXIT_FAILURE); */
/*     } */
/*     fprintf(ostream, "## generated file - DO NOT EDIT\n\n"); */
/*     fprintf(ostream, "module(\n"); */
/*     fprintf(ostream, "    name = \"ocamlsdk\", version = \"%s\",\n", ocaml_version); */
/*     fprintf(ostream, "    compatibility_level = %d,\n", default_compat); */
/*     fprintf(ostream, "    bazel_compatibility = [\">=%s\"]\n", bazel_compat); */
/*     fprintf(ostream, ")\n"); */
/*     fprintf(ostream, "\n"); */
/*     fprintf(ostream, */
/*             "bazel_dep(name = \"platforms\", version = \"%s\")\n", platforms_version); */
/*     fprintf(ostream, */
/*             "bazel_dep(name = \"bazel_skylib\", version = \"%s\")\n", skylib_version); */
/*     fprintf(ostream, */
/*             "bazel_dep(name = \"rules_ocaml\", version = \"%s\")\n", rules_ocaml_version); */

/*     fprintf(ostream, */
/*             "bazel_dep(name = \"stublibs\", version = \"%s\")\n", */
/*             default_version); */

/*     fclose(ostream); */
/*     if (verbosity > log_writes) */
/*         fprintf(INFOFD, GRN "INFO" CRESET */
/*                 " wrote %s\n", utstring_body(dst_file)); */



/*     /\* now drop WORKSPACE.bazel from path *\/ */
/*     utstring_renew(dst_file); */
/*     utstring_printf(dst_file, "%s/ocamlsdk", coswitch_lib); */

/*     /\* */
/*       if we are in a bazel env get runfiles dir */
/*       else runfiles dir is <switch_pfx>/share/obazl/templates */
/*      *\/ */

/*     /\* UT_string *xrunfiles; *\/ */
/*     /\* utstring_new(xrunfiles); *\/ */
/*     /\* LOG_DEBUG(0, "PWD: %s", getcwd(NULL,0)); *\/ */
/*     /\* LOG_DEBUG(0, "OPAM_SWITCH_PREFIX: %s", *\/ */
/*     /\*           getenv("OPAM_SWITCH_PREFIX")); *\/ */
/*     /\* LOG_DEBUG(0, "BAZEL_CURRENT_REPOSITORY: %s", *\/ */
/*     /\*           BAZEL_CURRENT_REPOSITORY); *\/ */
/*     /\* if (getenv("BUILD_WORKSPACE_DIRECTORY")) { *\/ */
/*     /\*     if (strlen(BAZEL_CURRENT_REPOSITORY) == 0) { *\/ */
/*     /\*         utstring_printf(xrunfiles, "%s", *\/ */
/*     /\*                         realpath("new", NULL)); *\/ */
/*     /\*     } else { *\/ */
/*     /\*         LOG_DEBUG(0, "XXXXXXXXXXXXXXXX %s", *\/ */
/*     /\*                   BAZEL_CURRENT_REPOSITORY); *\/ */
/*     /\*         char *rp = realpath("external/" *\/ */
/*     /\*                             BAZEL_CURRENT_REPOSITORY, *\/ */
/*     /\*                             NULL); *\/ */
/*     /\*         LOG_DEBUG(0, "PWD: %s", getcwd(NULL,0)); *\/ */
/*     /\*         LOG_DEBUG(0, "AAAAAAAAAAAAAAAA %s", rp); *\/ */
/*     /\*         utstring_printf(xrunfiles, "%s", rp); *\/ */
/*     /\* LOG_DEBUG(0, "XXXXXXXXXXXXXXXX %s", *\/ */
/*     /\*           utstring_body(xrunfiles)); *\/ */
/*     /\*     } *\/ */
/*     /\* } else { *\/ */
/*     /\*     utstring_printf(xrunfiles, "%s/share/obazl", *\/ */
/*     /\*                     switch_pfx); *\/ */
/*     /\*     // runfiles = <switch-pfx>/share/obazl *\/ */
/*     /\*     /\\* runfiles = "../../../share/obazl"; *\\/ *\/ */
/*     /\* } *\/ */

/*     /\* char *runfiles = utstring_body(xrunfiles); // strdup? *\/ */
/*     LOG_DEBUG(0, "RUNFILES root: %s", rf_root()); */

/*     UT_string *bld_file; */
/*     utstring_new(bld_file); */
/*     utstring_printf(bld_file, "%s/ocamlsdk/bin", coswitch_lib); */
/*     mkdir_r(utstring_body(bld_file)); */
/*     utstring_printf(bld_file, "/BUILD.bazel"); */
/*     emit_ocaml_bin_dir(utstring_body(bld_file)); */

/*     UT_string *dst_dir; */
/*     utstring_new(dst_dir); */
/*     utstring_printf(dst_dir, "%s/ocamlsdk/bin", coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     _emit_ocaml_bin_symlinks(dst_dir, switch_pfx); //, coswitch_lib); */

/*     /\* UT_string *templates; *\/ */
/*     /\* utstring_new(templates); *\/ */
/*     /\* utstring_printf(templates, "%s/templates", runfiles); *\/ */

/*     /\* emit_ocaml_platform_buildfiles(/\\* templates, *\\/ coswitch_lib); *\/ */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/toolchain", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_toolchain_buildfiles(dst_dir); */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/stdlib", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_stdlib_pkg(dst_dir, switch_lib); */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/stublibs", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_stublibs(dst_dir, switch_pfx); */

/*     // for <switch>lib/stublibs: */
/*     char *switch_stublibs = opam_switch_stublibs(switch_name); */
/*     emit_lib_stublibs_pkg(registry, */
/*                           switch_stublibs, coswitch_lib); */
/*     /\* aliases in <switch>/lib/compiler-libs *\/ */
/*     /\* utstring_renew(dst_dir); *\/ */
/*     /\* utstring_printf(dst_dir, *\/ */
/*     /\*                 "lib", *\/ */
/*     /\*                 coswitch_lib); *\/ */
/*     /\* mkdir_r(utstring_body(dst_dir)); *\/ */
/*     /\* emit_compiler_libs_pkg(dst_dir, coswitch_lib); *\/ */

/*      utstring_renew(dst_dir); */
/*      utstring_printf(dst_dir, */
/*                      "%s/ocamlsdk/lib/compiler-libs", */
/*                      coswitch_lib); */
/*      mkdir_r(utstring_body(dst_dir)); */
/*      emit_ocaml_compiler_libs_pkg(dst_dir, */
/*                                   switch_lib); */
/*                                  /\* coswitch_lib); *\/ */

/*      /\* Bigarray integrated into stdlib as of 4.07 *\/ */
/*     emit_ocaml_bigarray_pkg(runfiles, switch_lib, coswitch_lib); */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/dynlink", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_dynlink_pkg(dst_dir, switch_lib); */

/*     /\* num lib split out of core distrib starting 4.06.0 *\/ */
/*     /\* emit_ocaml_num_pkg(runfiles, switch_lib, coswitch_lib); *\/ */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/ocamldoc", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_ocamldoc_pkg(dst_dir, switch_lib); */

/*     // 5.0.0+ has <switch>/lib/ocaml/profiling/META */
/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/profiling", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_profiling_pkg(dst_dir, switch_lib); //, coswitch_lib); */
/*     // pre-5.0.0: no <switch>/lib/ocaml/profiling */
/*     // always emit <coswitch>/lib/ocaml/profiling */
/*     /\* emit_ocaml_profiling_alias_pkg(dst_dir, switch_lib, coswitch_lib); *\/ */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/runtime_events", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_rtevents_pkg(dst_dir, switch_lib); */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/str", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_str_pkg(dst_dir, switch_lib); */

/*     //NB: vmthreads removed in v. 4.08.0? */
/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/threads", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_threads_pkg(dst_dir, switch_lib); */
/*     /\* if (!ocaml_prev5) *\/ */
/*         /\* emit_registry_record(registry, compiler_version, *\/ */
/*         /\*                      NULL, pkgs); *\/ */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/unix", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_unix_pkg(dst_dir, switch_lib); */

/*     /\* this is an obazl invention: *\/ */
/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/runtime", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_runtime_pkg(dst_dir, switch_lib); */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/lib/ffi", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_c_ffi_pkg(dst_dir, switch_lib); */

/*     utstring_renew(dst_dir); */
/*     utstring_printf(dst_dir, */
/*                     "%s/ocamlsdk/version", */
/*                     coswitch_lib); */
/*     mkdir_r(utstring_body(dst_dir)); */
/*     emit_ocaml_version(dst_dir, coswitch_lib, compiler_version); */

/*     utstring_free(dst_dir); */

/*     TRACE_EXIT; */
/* } */
