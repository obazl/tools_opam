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

extern char *switch_pfx;
extern char *switch_lib;

EXPORT void xmkdir_r(const char *dir) {
    TRACE_ENTRY;
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

UT_array *cc_stubs;

void ext_emit_ocamlsdk_module(/* char *switch_id, */
                              char *_ocaml_version,
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

    utstring_renew(dst_dir);
    utstring_printf(dst_dir, "lib/compiler-libs");
    mkdir_r(utstring_body(dst_dir));
    emit_ocaml_compiler_libs_pkg(obazl_pfx,
                                 dst_dir,
                                 switch_lib);

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
