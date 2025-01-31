#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <glob.h>
#include <libgen.h>
/* #include <regex.h> */

#if EXPORT_INTERFACE
#include <stdio.h>
#endif

#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* #include "libcoswitch.h" */
#include "opamc.h"
/* #include "librunfiles.h" */

#include "liblogc.h"
/* #if EXPORT_INTERFACE */
/* #include "semver.h" */
#include "utarray.h"
#include "utstring.h"
/* #endif */

#include "findlibc.h"

/* #include "tools_opam.h" */
#include "ext_emit_build_bazel.h"

/* **************************************************************** */
/* /\* const char *errmsg = NULL; *\/ */

extern int level;
extern int spfactor;
extern char *sp;

extern int log_writes;
extern int log_symlinks;

#define DEBUG_LEVEL debug_opam
extern int  DEBUG_LEVEL;
#define TRACE_FLAG trace_opam
extern bool TRACE_FLAG;
extern int indent;
extern int delta;

/* extern */ char *ocaml_ws;
/* extern char *dst_root; */

extern bool verbose;
extern int debug;
extern bool trace;

UT_string *bzl_switch_pfx;
UT_string *bazel_pkg_root;
UT_string *build_bazel_file;

int symlink_ct;

extern char *default_version;
extern int   default_compat;
extern char *bazel_compat;

bool distrib_pkg;

/* extern char *switch_id; */
extern char *switch_pfx;
extern char *switch_lib;

EXPORT void xmkdir_r(const char *dir) {
    /* log_debug("xmkdir_r %s", dir); */
    char tmp[512];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

//UT_array *cc_stubs;


EXPORT void ext_emit_module_file(UT_string *module_file,
                                 struct obzl_meta_package *_pkg,
                                 bool alias)
{
    TRACE_ENTRY_MSG("%s", _pkg->name);
    LOG_DEBUG(0, "module file: %s", utstring_body(module_file));
    findlib_version_t *semv;
    char version[256];
    semv = findlib_pkg_version(_pkg);
    sprintf(version, "%d.%d.%d",
            semv->major, semv->minor, semv->patch);

    FILE *ostream;
    ostream = fopen(utstring_body(module_file), "w");
    if (ostream == NULL) {
        LOG_ERROR(0, "fopen fail: %s %s",
                  utstring_body(module_file),
                  strerror(errno));
        perror(utstring_body(module_file));
        exit(EXIT_FAILURE);
    }

    fprintf(ostream, "## generated file - DO NOT EDIT\n\n");

    fprintf(ostream, "module(\n");
    if (alias) {
        fprintf(ostream, "    name = \"%s\",\n", _pkg->module_name);
    } else {
        fprintf(ostream, "    name = \"opam.%s\",\n", _pkg->module_name);
    }
    fprintf(ostream, "    version = \"%s\",  # %s\n",
            default_version, version);
    fprintf(ostream, "    compatibility_level = %d, # %d\n",
            default_compat, semv->major);
    fprintf(ostream, "    bazel_compatibility = [\">=%s\"]\n",
            bazel_compat);
    fprintf(ostream, ")\n");
    fprintf(ostream, "\n");

    /* opam bzlmodules depend on ocaml_import */
    /* if (!alias) { */
    fprintf(ostream, "bazel_dep(name = \"rules_ocaml\", version = \"%s\")\n", rules_ocaml_version);
    /* } */
    /* modules that depend on built-ins, e.g. str, threads, unix, etc. */
    LOG_DEBUG(0, "running findlib", "");
    UT_array *pkg_deps = findlib_pkg_deps(_pkg, true);
    if (pkg_deps == NULL) {
        LOG_DEBUG(0, "pkg has no deps", "");
        fprintf(ostream, "\n");
        fclose(ostream);
        TRACE_EXIT;
        return;
    }
    LOG_DEBUG(0, "pkg deps ct: %d", utarray_len(pkg_deps));
    char **p = NULL;
    if (!alias) {
        if (pkg_deps) {
            while ( (p=(char**)utarray_next(pkg_deps, p))) {
                if ((strncmp(*p, "compiler-libs", 13) == 0)
                    || (strncmp(*p, "dynlink", 7) == 0)
                    || (strncmp(*p, "ocamldoc", 8) == 0)
                    || (strncmp(*p, "str", 3) == 0)
                    || (strncmp(*p, "threads", 7) == 0)
                    || (strncmp(*p, "unix", 4) == 0)
                    ){
                    fprintf(ostream, "bazel_dep(name = \"ocamlsdk\",       version = \"%s\")\n", ocaml_version);
                    break;
                }
            }
        }
        /* fprintf(ostream, "          version = \"0.0.0\")\n"); */
        fprintf(ostream, "\n");

        // get **repo** deps: direct deps of pkg and all subpkgs
        if (pkg_deps) {
            p = NULL;
            /* struct obzl_meta_package *pkg; */
            /* LOG_DEBUG(0, "HASH CT: %d", HASH_COUNT(_pkgs)); */
            /* exit(0); */
            while ( (p=(char**)utarray_next(pkg_deps, p))) {
                LOG_DEBUG(0, "pkg dep: %s", *p);
                if ((strncmp(*p, "compiler-libs", 13) == 0)
                    || (strncmp(*p, "dynlink", 7) == 0)
                    || (strncmp(*p, "ocamldoc", 8) == 0)
                    || (strncmp(*p, "str", 3) == 0)
                    || (strncmp(*p, "threads", 7) == 0)
                    || (strncmp(*p, "unix", 4) == 0)
                    ){
                    continue;
                }
                if (strncmp(*p, _pkg->name, 512) != 0) {
                    /* for now ignore actual versions */
                    sprintf(version, "%d.%d.%d", 0,0,0);
                    /* HASH_FIND_STR(_pkgs, *p, pkg); */
                    /* if (pkg) { */
                    /*     free(semv); */
                    /*     version[0] = '\0'; */
                    /*     semv = findlib_pkg_version(pkg); */
                    /*     sprintf(version, "%d.%d.%d", */
                    /*             semv->major, semv->minor, semv->patch); */
                    /* } else { */
                    /*     //FIXME: pkg 'compiler-libs' (a dep of */
                    /*     // ppxlib.astlib etc.) is a pseudo-pkg, */
                    /*     // referring to lib/ocaml/compiler-libs, */
                    /*     // so it has neither pkg entry nor version */
                    /*     if (strncmp(*p, "compiler-libs", 13) == 0) { */
                    /*         sprintf(version, "%d.%d.%d", 0,0,0); */
                    /*     } else { */
                    /*         sprintf(version, "%d.%d.%d", -1, -1 , -1); */
                    /*     } */
                    /* } */
                    fprintf(ostream, "bazel_dep(name = \"%s\", version = \"%s\")\n",
                            *p, default_version);
                    /* fprintf(ostream, "bazel_dep(name = \"opam.%s\", version = \"%s\") # %s\n", */
                    /*         *p, default_version, version); */
                    /* fprintf(ostream, "          version = \"%s\")\n", */
                    /*         default_version); */
                }
            }
        }
    }
    /* HACK ALERT! This hideous code deals with ppx_runtime_deps,
       and must check to prevent duplicates.
     */
    char **already = NULL;      /* for searching */
    if (pkg_deps) {
        utarray_sort(pkg_deps,strsort);
    }
    UT_array *pkg_codeps = findlib_pkg_codeps(_pkg, true);
    if (pkg_codeps) {
        char **p = NULL;
        /* struct obzl_meta_package *pkg; */
        /* LOG_DEBUG(0, "HASH CT: %d", HASH_COUNT(_pkgs)); */
        /* exit(0); */
        while ( (p=(char**)utarray_next(pkg_codeps, p))) {
            if (strncmp(*p, _pkg->name, 512) != 0) {
                sprintf(version, "%d.%d.%d", 0,0,0);
                /* HASH_FIND_STR(_pkgs, *p, pkg); */
                /* if (pkg) { */
                /*     free(semv); */
                /*     version[0] = '\0'; */
                /*     semv = findlib_pkg_version(pkg); */
                /*     sprintf(version, "%d.%d.%d", */
                /*             semv->major, semv->minor, semv->patch); */
                /* } else { */
                /*     //FIXME: pkg 'compiler-libs' (a dep of */
                /*     // ppxlib.astlib etc.) is a pseudo-pkg, */
                /*     // referring to lib/ocaml/compiler-libs, */
                /*     // so it has neither pkg entry nor version */
                /*     if (strncmp(*p, "compiler-libs", 13) == 0) { */
                /*         sprintf(version, "%d.%d.%d", 0,0,0); */
                /*     } else { */
                /*         sprintf(version, "%d.%d.%d", -1, -1 , -1); */
                /*     } */
                /* } */
                if (pkg_deps) {
                    already = NULL;
                    already = (char**)utarray_find(pkg_deps, p, strsort);
                    if (already == NULL) {
                        fprintf(ostream, "bazel_dep(name = \"%s\", version = \"%s\")\n",
                                *p, default_version);
                        /* fprintf(ostream, "bazel_dep(name = \"%s\", # %s\n", */
                        /*         *p, version); */
                        /* fprintf(ostream, "          version = \"%s\") #codep\n", */
                        /*         default_version); */
                    }
                }
            }
        }
    }

    fprintf(ostream, "\n");

    fclose(ostream);
    if (verbosity > log_writes) {
        fprintf(INFOFD, GRN "INFO" CRESET
                " wrote: %s\n", utstring_body(module_file));
    }
    if (pkg_deps) {
        utarray_free(pkg_deps);
    }
    if (pkg_codeps) {
        utarray_free(pkg_codeps);
    }
    TRACE_EXIT;
}

/* ############################################### */
void ext_emit_ocamlsdk_module(char *_ocaml_version,
                              char *switch_pfx,
                              char *obazl_pfx)
{
    TRACE_ENTRY;
    LOG_DEBUG(0, "switch_pfx: %s", switch_pfx);

    char *coswitch_lib = "lib";

    UT_string *dst_file;
    utstring_new(dst_file);
    utstring_printf(dst_file, "MODULE.bazel");
    FILE *ostream = fopen(utstring_body(dst_file), "w");
    if (ostream == NULL) {
        log_error("%s", strerror(errno));
        perror(utstring_body(dst_file));
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "## generated file - DO NOT EDIT\n\n");
    fprintf(ostream, "module(\n");
    fprintf(ostream, "    name = \"ocamlsdk\", version = \"%s\",\n", ocaml_version);
    fprintf(ostream, "    compatibility_level = %d,\n", default_compat);
    fprintf(ostream, "    bazel_compatibility = [\">=%s\"]\n", bazel_compat);
    fprintf(ostream, ")\n");
    fprintf(ostream, "\n");
    fprintf(ostream,
            "bazel_dep(name = \"platforms\", version = \"%s\")\n", platforms_version);
    fprintf(ostream,
            "bazel_dep(name = \"bazel_skylib\", version = \"%s\")\n", skylib_version);
    fprintf(ostream,
            "bazel_dep(name = \"rules_ocaml\", version = \"%s\")\n", rules_ocaml_version);

    fprintf(ostream,
            "bazel_dep(name = \"stublibs\", version = \"%s\")\n",
            default_version);

    fclose(ostream);
    if (verbosity > log_writes)
        fprintf(INFOFD, GRN "INFO" CRESET
                " wrote %s\n", utstring_body(dst_file));


    /*
      if we are in a bazel env get runfiles dir
      else runfiles dir is <switch_pfx>/share/obazl/templates
     */

    /* UT_string *xrunfiles; */
    /* utstring_new(xrunfiles); */
    /* LOG_DEBUG(0, "PWD: %s", getcwd(NULL,0)); */
    /* LOG_DEBUG(0, "OPAM_SWITCH_PREFIX: %s", */
    /*           getenv("OPAM_SWITCH_PREFIX")); */
    /* LOG_DEBUG(0, "BAZEL_CURRENT_REPOSITORY: %s", */
    /*           BAZEL_CURRENT_REPOSITORY); */
    /* if (getenv("BUILD_WORKSPACE_DIRECTORY")) { */
    /*     if (strlen(BAZEL_CURRENT_REPOSITORY) == 0) { */
    /*         utstring_printf(xrunfiles, "%s", */
    /*                         realpath("new", NULL)); */
    /*     } else { */
    /*         LOG_DEBUG(0, "XXXXXXXXXXXXXXXX %s", */
    /*                   BAZEL_CURRENT_REPOSITORY); */
    /*         char *rp = realpath("external/" */
    /*                             BAZEL_CURRENT_REPOSITORY, */
    /*                             NULL); */
    /*         LOG_DEBUG(0, "PWD: %s", getcwd(NULL,0)); */
    /*         LOG_DEBUG(0, "AAAAAAAAAAAAAAAA %s", rp); */
    /*         utstring_printf(xrunfiles, "%s", rp); */
    /* LOG_DEBUG(0, "XXXXXXXXXXXXXXXX %s", */
    /*           utstring_body(xrunfiles)); */
    /*     } */
    /* } else { */
    /*     utstring_printf(xrunfiles, "%s/share/obazl", */
    /*                     switch_pfx); */
    /*     // runfiles = <switch-pfx>/share/obazl */
    /*     /\* runfiles = "../../../share/obazl"; *\/ */
    /* } */

    /* char *runfiles = utstring_body(xrunfiles); // strdup? */
    /* LOG_DEBUG(1, "RUNFILES root: %s", rf_root()); */

    UT_string *bld_file;
    utstring_new(bld_file);
    utstring_printf(bld_file, "bin");
    mkdir_r(utstring_body(bld_file));
    utstring_printf(bld_file, "/BUILD.bazel");
    emit_ocaml_bin_dir(utstring_body(bld_file));

    UT_string *dst_dir;
    utstring_new(dst_dir);
    utstring_printf(dst_dir, "bin");
    _emit_ocaml_bin_symlinks(dst_dir, switch_pfx); //, coswitch_lib);

    /* UT_string *templates; */
    /* utstring_new(templates); */
    /* utstring_printf(templates, "%s/templates", runfiles); */

    /* emit_ocaml_platform_buildfiles(/\* templates, *\/ coswitch_lib); */

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "toolchain");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_toolchain_buildfiles(obazl_pfx, dst_dir);

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/stdlib");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_stdlib_pkg(dst_dir, switch_lib);

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/stublibs");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_stublibs(dst_dir, switch_pfx);

    //  <switch>lib/stublibs: not an ocamlsdk subdir,
    // must be installed separately
    /* char *switch_stublibs = opam_switch_stublibs(switch_id); */
    /* emit_lib_stublibs_pkg(NULL, switch_stublibs, coswitch_lib); */

    // aliases <switch>lib/compiler-libs:
    /* NB: under opam_dep extension this must
       be built as a distinct dep
     */
    /* emit_compiler_libs_pkg(NULL, coswitch_lib); */

    utstring_renew(dst_dir);
    utstring_printf(dst_dir,
                    "lib/compiler-libs");
                    /* coswitch_lib); */
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_compiler_libs_pkg(obazl_pfx,
                                 dst_dir,
                                 switch_lib);
                                 /* coswitch_lib); */

    emit_ocaml_bigarray_pkg(NULL, obazl_pfx,
                            switch_lib, coswitch_lib);

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/dynlink");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_dynlink_pkg(dst_dir, switch_lib);

    /* num lib split out of core distrib starting 4.06.0 */
    /* emit_ocaml_num_pkg(NULL, switch_lib, coswitch_lib); */

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/ocamldoc");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_ocamldoc_pkg(dst_dir, switch_lib);

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/profiling");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_profiling_pkg(dst_dir, switch_lib);

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/runtime_events");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_rtevents_pkg(dst_dir, switch_lib);

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/str");
    emit_ocaml_str_pkg(dst_dir, switch_lib);

    //NB: vmthreads removed in v. 4.08.0?
    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/threads");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_threads_pkg(dst_dir, switch_lib);
    /* if (!ocaml_prev5) */
        /* emit_registry_record(registry, compiler_version, */
        /*                      NULL, pkgs); */

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/unix");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_unix_pkg(dst_dir, switch_lib);

    /* this is an obazl invention: */
    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "runtime");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_runtime_pkg(dst_dir, switch_lib);

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/ffi");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_c_ffi_pkg(dst_dir, switch_lib);

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "version");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_version(dst_dir, coswitch_lib, _ocaml_version);

    TRACE_EXIT;
}
