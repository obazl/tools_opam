//FIXME: support -j (--jsoo-enable) flag

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

#if INTERFACE
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#endif
#include <sys/stat.h>
#include <unistd.h>

/* #include "cwalk.h" */
/* #include "gopt.h" */
#include "liblogc.h"
#include "findlibc.h"
#include "opamc.h"
/* #include "semver.h" */
#include "utarray.h"
#include "uthash.h"
#include "utstring.h"
/* #include "xdgc.h" */
/* #include "librunfiles.h" */

#include "tools_opam.h"
#include "config_pkg.h"

UT_string *imports_path;
UT_string *pkg_parent;

struct paths_s {
    UT_string *registry;
    UT_string *coswitch_lib;
    bool ocaml_dep;             /* depends on builtin e.g. str, threads, unix, etc. */
    struct obzl_meta_package *pkgs;
};

static UT_string *meta_path;
char *switch_name;
//static char *coswitch_name = NULL; // may be "local"

#define DEBUG_LEVEL debug_opam
int  DEBUG_LEVEL;
#define TRACE_FLAG trace_opam
bool TRACE_FLAG;
extern bool trace_findlibc;
extern int  debug_findlibc;
extern bool opamc_trace;
extern int  opamc_debug;
extern bool xdgc_trace;
extern int  xdgc_debug;

bool quiet;
bool verbose;
int  verbosity;

extern char *findlibc_version;

extern char *default_version;
extern int   default_compat;
extern char *bazel_compat;

extern char *platforms_version;
extern char *skylib_version;
extern char *rules_cc_version;

extern char *rules_ocaml_version;
extern char *ocaml_version;
extern char *compiler_version;

extern int log_writes; // threshhold for logging all writes
extern int log_symlinks;
extern bool enable_jsoo;

extern char *pkg_path; // = NULL;

/* LOCAL void _pkg_handler(char *switch_pfx, */
/*                         char *switch_lib, /\* switch_lib *\/ */
/*                         char *pkg_dir, */
/*                         void *_paths) */

/* NB: if we're precompiled and called by an extension,
   then we cannot discover a local _opam switch. so
   the extension must do that and pass the info.
 */

char *switch_id;
char *switch_pfx;
UT_string *_switch_lib;
char *switch_lib;
char *pkg_name;

void opam_pkg_handler(char *_switch_pfx,
                      char *_ocaml_version,
                      char *pkg_name,
                      char *obazl_pfx)
        /* .registry = registry, */
        /* .coswitch_lib = coswitch_lib */
// coswitch is always <switch_pfx>/share/obazl/registry
{
    TRACE_ENTRY;
    LOG_DEBUG(0, "switch_pfx: %s", _switch_pfx);
    /* printf("%s:%d OPAM switch_lib: %s\n", __FILE__, __LINE__, _switch_lib); */
    LOG_DEBUG(0, "pkg name: %s", pkg_name);

    /* switch_id = _switch_id; */
    switch_pfx = _switch_pfx; // opam_switch_prefix(switch_id);
    utstring_new(_switch_lib);
    utstring_printf(_switch_lib, "%s/lib", switch_pfx);
    switch_lib = utstring_body(_switch_lib);

    if ((strncmp(pkg_name, "ocamlsdk", 8) == 0)
        && strlen(pkg_name) == 8) {
        ext_emit_ocamlsdk_module(_ocaml_version, switch_pfx, obazl_pfx);
    }
    else if ((strncmp(pkg_name, "stublibs", 8) == 0)
        && strlen(pkg_name) == 8) {
        /* calls 'opam var stublibs --switch <switch id> */
        /* char *switch_stublibs = opam_switch_stublibs(switch_id); */
        UT_string *dst_dir;
        utstring_new(dst_dir);
        utstring_printf(dst_dir, "./");
        mkdir_r(utstring_body(dst_dir));
        emit_lib_stublibs_pkg(dst_dir, switch_lib);
        // switch_stublibs);
        /* ext_emit_lib_stublibs(switch_id, switch_pfx); */
    } else {
        opam_libpkg_handler(obazl_pfx, pkg_name);
    }
    TRACE_EXIT;
}

void opam_libpkg_handler(char *obazl_pfx, char *pkg_name)
{
    /* struct paths_s *paths = (struct paths_s*)_paths; */
    /* UT_string *registry = (UT_string*)paths->registry; */
    /* UT_string *coswitch_lib = (UT_string*)paths->coswitch_lib; */

    /* SPECIAL HANDLING for psuedo-pkgs:
       ocamlsdk, stublibs, thread, etc.
     */
    bool empty_pkg = false;

    /* local coswitch lib dir is just cwd */
    UT_string *coswitch_lib;
    utstring_new(coswitch_lib);
    utstring_printf(coswitch_lib, "./");
                    // "%s/share/obazl/registry/lib",
                    /* switch_pfx); */
    /* struct obzl_meta_package *pkgs */
    /*     = (struct obzl_meta_package*)paths->pkgs; */

    UT_string *coswitch_pkg_root;
    utstring_new(coswitch_pkg_root);
    utstring_printf(coswitch_pkg_root, ".");

    if (verbosity > 1) {
        log_debug("pkg_handler: %s", pkg_name);
        log_debug("site-lib: %s", switch_lib);
        /* log_debug("registry: %s", utstring_body(registry)); */
        log_debug("coswitch: %s", utstring_body(coswitch_lib));
        /* log_debug("pkgs ct: %d", HASH_COUNT(pkgs)); */
    }

    utstring_renew(meta_path);
    utstring_printf(meta_path, "%s/%s/META",
                    switch_lib,
                    pkg_name);
                    /* utstring_body(opam_switch_lib), pkg_name); */
    /* if (verbosity > 1) */
    LOG_DEBUG(0, "%s:%d meta_path: %s\n", __FILE__, __LINE__, utstring_body(meta_path));
    /* fflush(NULL); */

    errno = 0;
    if ( access(utstring_body(meta_path), F_OK) != 0 ) {
        // FIXME: report that pkg is not installed,
        // not just that META is missing
        // no META happens for e.g. <switch>/lib/stublibs
        log_warn("%s: %s",
                 strerror(errno), utstring_body(meta_path));
        fprintf(stdout, "ERROR: pkg %s not installed?\n", pkg_name);
        return;
    /* } else { */
    /*     /\* exists *\/ */
    /*     log_info("accessible: %s", utstring_body(meta_path)); */
    }
    empty_pkg = false;
    errno = 0;
    // pkg must be freed...
    struct obzl_meta_package *pkg
        = obzl_meta_parse_file(utstring_body(meta_path));

    if (pkg == NULL) {
        if ((errno == -1)
            || (errno == -2)) {
            empty_pkg = true;
            /* #if defined(TRACING) */
            if (verbosity > 2)
                log_warn("Empty META file: %s", utstring_body(meta_path));
            /* #endif */
            /* check dune-package for installed executables */
            /* chdir(old_cwd); */

            pkg = (struct obzl_meta_package*)calloc(sizeof(struct obzl_meta_package), 1);
            char *fname = strdup(utstring_body(meta_path));
            pkg->name      = package_name_from_file_name(fname);
            /* pkg->name      = pkg_dir; */
            char *x = strdup(pkg->name);
            char *p;
            // module names may not contain uppercase
            for (p = x; *p; ++p) *p = tolower(*p);
            pkg->module_name = strdup(x);
            free(x);
            x = strdup(fname);
            pkg->path      = strdup(dirname(x));
            free(x);
            pkg->directory = pkg->name; // dirname(fname);
            pkg->metafile  = utstring_body(meta_path);
            pkg->entries = NULL;
        } else {
            log_error("Error parsing %s", utstring_body(meta_path));
            return;
        }
    } else {
        if (verbosity > 2) {
            log_info("Parsed %d %s", verbosity, utstring_body(meta_path));
        }
    }

    LOG_DEBUG(0, "pkg_name: %s", pkg_name);

    /** emit module files for opam pkg **/
    // MODULE.bazel emitted later, after all pkgs parsed
    UT_string *mfile;
    utstring_new(mfile);
    utstring_printf(mfile, "./");
    /* "%s/%s", */
    /* utstring_body(coswitch_lib), */
    /* /\* version->major, version->minor, *\/ */
    /* /\* version->patch, *\/ */
    /* pkg_name); */
    xmkdir_r(utstring_body(mfile));
    utstring_printf(mfile, "/MODULE.bazel");
    /* utstring_body(mfile)); */
    //ext_emit_module_file(mfile, obazl_pfx, pkg, NULL);
    emit_module_file(mfile, obazl_pfx, pkg, NULL);
    utstring_free(mfile);

    if (!empty_pkg) {
        // then emit the BUILD.bazel files for the opam pkg
        utstring_new(imports_path);
        utstring_printf(imports_path, "%s", pkg_name);
        /* obzl_meta_package_name(pkg)); */

        utstring_new(pkg_parent);

        // for now:
        UT_string *_switch_lib;
        utstring_new(_switch_lib);
        utstring_printf(_switch_lib, "%s", switch_lib);

        /* emit @opam.foo */
        emit_build_bazel(_switch_lib, // switch_lib,
                         utstring_body(coswitch_lib),
                         utstring_body(coswitch_pkg_root),
                         /* utstring_body(ws_root), */
                         0,         /* indent level */
                         pkg_name, // pkg_root
                         pkg_parent, /* needed for handling subpkgs */
                         NULL, // "buildfiles",        /* _pkg_prefix */
                         utstring_body(imports_path),
                         /* "",      /\* pkg-path *\/ */
                         obazl_pfx,
                         pkg);
        log_info("emitted bazel pkg %s, to %s",
                 pkg_name, utstring_body(coswitch_lib));
    }
    UT_string *dst_dir;
    utstring_new(dst_dir);
    utstring_printf(dst_dir, "./");
    emit_pkg_bindir(dst_dir,
                    switch_pfx,
                    utstring_body(coswitch_lib),
                    pkg->name);
}

