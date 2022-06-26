#include <errno.h>
#include <dirent.h>

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

#include "emit_ocaml_repo.h"

/* **************************************************************** */
/* static int level = 0; */
/* static int spfactor = 4; */
/* static char *sp = " "; */

/* static int indent = 2; */
/* static int delta = 2; */

FILE *_open_buildfile(UT_string *ocaml_file) {
    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    ostream = fopen(utstring_body(ocaml_file), "w");
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

void _emit_toolchain_bin_symlinks(void) // UT_string *dst_dir, UT_string *src_dir)
{
    log_debug("_emit_toolchain_bin_symlinks");

    UT_string *dst_dir;
    utstring_new(dst_dir);
    utstring_concat(dst_dir, bzl_switch_pfx);
    utstring_printf(dst_dir, "/ocaml/bin");

    /* UT_string *src_dir; // relative to opam_switch_lib */
    /* utstring_new(src_dir); */
    /* utstring_printf(src_dir, "bin"); */

    mkdir_r(utstring_body(dst_dir));

    log_debug("src_dir: %s\n", utstring_body(opam_switch_bin));
    log_debug("dst_dir: %s\n", utstring_body(dst_dir));

    /* UT_string *opamdir; */
    /* utstring_new(opamdir); */
    /* utstring_printf(opamdir, "%s/bin", */
    /*                 utstring_body(opam_switch_pfx) */
    /*                 /\* utstring_body(src_dir) *\/ */
    /*                 ); */

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    log_debug("opening src_dir for read: %s\n",
              utstring_body(opam_switch_bin));
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

            /* log_debug("symlinking %s to %s", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
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
}

// arg 1 true: bytecode emitter
// arg 2 true: optimized binary;
// last arg true: emit "default" rule
void _emit_ocaml_toolchain_adapter(FILE *ostream,
                                   bool bc, bool opt, bool fallback)
{
    char *pfx    = fallback? "default" : "opam";
    char *emitter = fallback? "n" : bc? "bc" : "n";
    char *optstr  = fallback? "opt" : opt? "opt" : "nopt";

    fprintf(ostream, "ocaml_toolchain_adapter(\n");
    fprintf(ostream, "    name         = \"_%s_%s_%s\",\n",
            pfx, emitter, optstr);
    fprintf(ostream, "    compiler     = \"@ocaml//bin:%s%s\",\n",
            bc?  "ocamlc" : "ocamlopt",
            fallback? ".opt" : opt? ".opt" : "");
    fprintf(ostream, "    emitting     = \"%s\",\n",
            bc?  "bytecode" : "native");

    fprintf(ostream, "    ocamlc       = \"@ocaml//bin:ocamlc\",\n");
    fprintf(ostream, "    ocamlc_opt   = \"@ocaml//bin:ocamlc.opt\",\n");
    fprintf(ostream, "    ocamlopt     = \"@ocaml//bin:ocamlopt\",\n");
    fprintf(ostream, "    ocamlopt_opt = \"@ocaml//bin:ocamlopt.opt\",\n");
    fprintf(ostream, "    ocamllex     = \"@ocaml//bin:ocamllex\",\n");
    fprintf(ostream, "    ocamlyacc    = \"@ocaml//bin:ocamlyacc\",\n");
    fprintf(ostream, "    vmlibs       = \"@stublibs//:stublibs\",\n");
    fprintf(ostream, "    linkmode     = \"dynamic\"\n");
    fprintf(ostream, ")\n");
}

/*
  opt: compile mode optimized; bc: bytecode emitter
 */
void _emit_ocaml_toolchain_binding(FILE *ostream,
                                   bool macos, bool bc, bool opt,
                                   bool fallback)
{
    char *pfx    = fallback? "default" : "opam";
    char *emitter = fallback? "n" : bc? "bc" : "n";
    char *optstr  = fallback? "opt" : opt? "opt" : "nopt";

    fprintf(ostream, "##########\n");
    fprintf(ostream, "toolchain(\n");
    fprintf(ostream, "    name           = \"%s_%s_%s_%s\",\n",
            macos? "macos" : "linux", pfx, emitter, optstr);
    fprintf(ostream, "    toolchain      = \"_%s_%s_%s\",\n",
            pfx, emitter, optstr);
    fprintf(ostream, "    toolchain_type = \"@rules_ocaml//toolchain:type\",\n");
    fprintf(ostream, "    exec_compatible_with = [\n");
    fprintf(ostream, "        \"@platforms//os:%s\",\n",
            macos? "macos" : "linux");
    fprintf(ostream, "        \"@platforms//cpu:x86_64\",\n");
    fprintf(ostream, "        \"@opam//tc:opam\",\n");
    fprintf(ostream, "        \"@opam//tc:%s\",\n",
            fallback? "optimized" : opt? "optimized" : "unoptimized");
    fprintf(ostream, "        \"@opam//tc/emitter:%s\",\n",
            bc? "bytecode" : "native");
    fprintf(ostream, "    ],\n");
    /* fprintf(ostream, "    target_compatible_with = [\n"); */
    /* fprintf(ostream, "        \"@platforms//os:macos\",\n"); */
    /* fprintf(ostream, "        \"@platforms//cpu:x86_64\",\n"); */
    /* fprintf(ostream, "        \"@opam//tc:opam\",\n"); */
    /* fprintf(ostream, "        \"@opam//tc:%s\",\n", */
    /*         fallback? "optimized" : opt? "optimized" : "unoptimized"); */
    /* fprintf(ostream, "        \"@opam//tc/emitter:%s\",\n", */
    /*         bc? "bytecode" : "native"); */
    /* fprintf(ostream, "    ],\n"); */
    fprintf(ostream, "    visibility             = [\"//visibility:public\"],\n");
    fprintf(ostream, ")\n");

    fprintf(ostream, "\n");
}

void _emit_ocaml_default_toolchain_binding(FILE *ostream, bool macos)
{
    fprintf(ostream, "##########\n");
    fprintf(ostream, "toolchain(\n");
    fprintf(ostream, "    name           = \"default_%s\",\n",
            macos? "macos" : "linux");
    fprintf(ostream, "    toolchain      = \"_opam_n_opt\",\n");
    fprintf(ostream, "    toolchain_type = \"@rules_ocaml//toolchain:type\",\n");
    fprintf(ostream, "    exec_compatible_with = [\n");
    fprintf(ostream, "        \"@platforms//os:%s\",\n",
            macos? "macos" : "linux");
    fprintf(ostream, "        \"@platforms//cpu:x86_64\",\n");
    fprintf(ostream, "    ],\n");
    /* fprintf(ostream, "    target_compatible_with = [\n"); */
    /* fprintf(ostream, "        \"@platforms//os:macos\",\n"); */
    /* fprintf(ostream, "        \"@platforms//cpu:x86_64\",\n"); */
    /* fprintf(ostream, "    ],\n"); */
    fprintf(ostream, "    visibility             = [\"//visibility:public\"],\n");
    fprintf(ostream, ")\n");

    fprintf(ostream, "\n");
}

void X_emit_ocaml_toolchain_bindings(FILE *ostream, char *switch_name)
{
    fprintf(ostream,
    "load(\"@rules_ocaml//toolchain:adapter.bzl\", \"ocaml_toolchain_adapter\")\n");

    fprintf(ostream, "\n");
    fprintf(ostream, "## toolchain bindings (to be passed to 'register_toolchains' function)\n");
    fprintf(ostream, "\n");
    fprintf(ostream, "##########\n");
    fprintf(ostream, "toolchain(\n");
    fprintf(ostream, "    name           = \"opam_n_n\",\n");
    fprintf(ostream, "    toolchain      = \"_opam_n_n\",\n");
    fprintf(ostream, "    toolchain_type = \"@rules_ocaml//toolchain:type\",\n");
    // was: @rules_ocaml//ocaml:toolchain
    fprintf(ostream, "    exec_compatible_with   = [\n");
    fprintf(ostream, "        \"@platforms//os:macos\",\n");
        /* ## we could define a toolchain for 32-bit macos, but we do not */
        /* ## expect that will ever be needed. */
    fprintf(ostream, "    ],\n");
    fprintf(ostream, "    target_compatible_with = [\n");
    fprintf(ostream, "        \"@platforms//os:macos\",\n");
    fprintf(ostream, "    ],\n");
    fprintf(ostream, "    visibility             = [\"//visibility:public\"],\n");
    fprintf(ostream, ")\n");

    fprintf(ostream, "\n");
    fprintf(ostream, "##########\n");
    // ## == toolchain_binding (of defn to type-tag)
    fprintf(ostream, "toolchain(\n");
    /* fprintf(ostream, "    name                   = \"opam.%s_linux\",\n", switch_name); */
    fprintf(ostream, "    name                   = \"ocaml_linux\",\n");
    fprintf(ostream, "    visibility             = [\"//visibility:public\"],\n");
    fprintf(ostream, "    toolchain_type         = \"@rules_ocaml//toolchain:type\",\n");
    // # == toolchain_definition
    fprintf(ostream, "    toolchain              = \":opam_toolchain\",\n");
    fprintf(ostream, "    exec_compatible_with   = [\n");
    fprintf(ostream, "        \"@platforms//os:linux\",\n");
    fprintf(ostream, "    ],\n");
    fprintf(ostream, "    target_compatible_with = [\n");
    fprintf(ostream, "        \"@platforms//os:linux\",\n");
    fprintf(ostream, "    ],\n");
    fprintf(ostream, ")\n");
}

void _symlink_ocaml_stdlib(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_stdlib to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking stdlib: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            if (strncmp("stdlib", direntry->d_name, 6) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
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

/* void emit_ocaml_stdlib_pkg(char *switch_name) */
/* { */
/*     if (debug) */
/*         log_debug("emit_ocaml_stdlib_pkg"); */

/*     UT_string *ocaml_file; */
/*     utstring_new(ocaml_file); */
/*     utstring_concat(ocaml_file, bzl_switch_pfx); */
/*     utstring_printf(ocaml_file, "/lib/ocaml"); */
/*     mkdir_r(utstring_body(ocaml_file)); */

/*     _symlink_ocaml_stdlib(utstring_body(ocaml_file)); */

/*     utstring_printf(ocaml_file, "/BUILD.bazel"); */

/*     _copy_buildfile("ocaml_stdlib.BUILD", ocaml_file); */
/*     /\* _symlink_buildfile("ocaml_stdlib.BUILD", ocaml_file); *\/ */

/*     utstring_free(ocaml_file); */
/* } */

/* **************************************************************** */
void _emit_ocaml_stublibs_symlinks(char *_dir)
{
    log_debug("_emit_ocaml_stublibs_symlinks: %s", _dir);

    UT_string *dst_dir;
    utstring_new(dst_dir);
    utstring_concat(dst_dir, bzl_switch_pfx);
    utstring_printf(dst_dir, "%s", _dir); // "/ocaml/stublibs");
    mkdir_r(utstring_body(dst_dir));

    UT_string *src_dir; // relative to opam_switch_lib
    utstring_new(src_dir);
    utstring_printf(src_dir,
                    "%s%s",
                    /* "%s/ocaml/stublibs", */
                    utstring_body(opam_switch_lib),
                    _dir);

    log_debug("src_dir: %s\n", utstring_body(src_dir));
    log_debug("dst_dir: %s\n", utstring_body(dst_dir));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    log_debug("opening src_dir for read: %s\n",
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
        log_debug("stublib: %s, type %d",
                  direntry->d_name, direntry->d_type);
        if( (direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK) ){

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(src_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(dst_dir), direntry->d_name);

            if (debug) {
                log_debug("stublibs: symlinking %s to %s\n",
                          utstring_body(src),
                          utstring_body(dst));
            }

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
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
}

void _emit_stublibs_symlinks(char *_dir)
{
    log_debug("_emit_stublibs_symlinks: %s", _dir);

    UT_string *dst_dir;
    utstring_new(dst_dir);
    utstring_concat(dst_dir, bzl_switch_pfx);
    utstring_printf(dst_dir, "%s", "/lib/stublibs");
    mkdir_r(utstring_body(dst_dir));

    UT_string *src_dir; // relative to opam_switch_lib
    utstring_new(src_dir);
    utstring_printf(src_dir,
                    "%s%s",
                    utstring_body(opam_switch_lib),
                    "/stublibs"); // _dir);

    log_debug("src_dir: %s\n", utstring_body(src_dir));
    log_debug("dst_dir: %s\n", utstring_body(dst_dir));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    log_debug("opening src_dir for read: %s\n",
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
        log_debug("stublib: %s, type %d",
                  direntry->d_name, direntry->d_type);
        if( (direntry->d_type==DT_REG)
            || (direntry->d_type==DT_LNK) ){

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(src_dir),
                            direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(dst_dir), direntry->d_name);

            if (debug) {
                log_debug("stublibs: symlinking %s to %s\n",
                          utstring_body(src),
                          utstring_body(dst));
            }

            rc = symlink(utstring_body(src),
                         utstring_body(dst));
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
}

void emit_ocaml_stublibs(char *switch_name)
{
    log_debug("emit_ocaml_stublibs\n");
    UT_string *ocaml_file;

    /* **************** */
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/stublibs");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");

    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n");
    /* fprintf(ostream, "exports_files(glob([\"**\"]))\n"); */
    fprintf(ostream, "filegroup(\n");
    fprintf(ostream, "    name = \"stublibs\",\n");
    fprintf(ostream, "    srcs = glob([\"**\"]),\n");
    fprintf(ostream, ")\n");
    fclose(ostream);

    _emit_ocaml_stublibs_symlinks("/ocaml/stublibs");
}

void emit_lib_stublibs(char *switch_name)
{
    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/lib/stublibs");
    mkdir_r(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/WORKSPACE.bazel");

    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "workspace( name = \"stublibs\" )"
            "    # generated file - DO NOT EDIT\n");
    fclose(ostream);

    /* now BUILD.bazel */
    utstring_renew(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/lib/stublibs");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");

    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n\n");
    /* fprintf(ostream, "exports_files(glob([\"**\"]))\n"); */
    fprintf(ostream, "filegroup(\n");
    fprintf(ostream, "    name = \"stublibs\",\n");
    fprintf(ostream, "    srcs = glob([\"**\"]),\n");
    fprintf(ostream, "    visibility = [\"//visibility:public\"]\n");
    fprintf(ostream, ")\n");
    fclose(ostream);
    utstring_free(ocaml_file);
    /* **************************************************************** */
    _emit_stublibs_symlinks("/lib/stublibs");
}

void emit_ocaml_platform_buildfile(char *switch_name)
{
    log_debug("emit_ocaml_platform_buildfile\n");
    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/platform");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("ocaml_platform.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

void emit_ocaml_toolchain_buildfile(char *switch_name)
{
    log_debug("emit_ocaml_toolchain_buildfile\n");
    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchains");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");

    FILE *ostream;
    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        fprintf(stderr, "ERROR: fopen, emit_ocaml_toolchain_buildfile\n");
        perror(utstring_body(ocaml_file));
        exit(EXIT_FAILURE);
    }

    fprintf(ostream, "# generated file - DO NOT EDIT\n\n");
    fprintf(ostream,
    "load(\"@rules_ocaml//toolchain:adapter.bzl\", \"ocaml_toolchain_adapter\")\n");

    fprintf(ostream, "\n");
    fprintf(ostream, "## toolchain bindings (to be passed to 'register_toolchains' function)\n");
    fprintf(ostream, "\n");

    /* _emit_ocaml_toolchain_bindings(ostream, switch_name); */
    // arg 1 true: macos, false: linux
    // arg 2 true: bytecode emitter
    // arg 3 true: optimized binary;
    // last arg true: emit "default" rule
    // mac
    _emit_ocaml_toolchain_binding(ostream, true, false,  true, false); // n_n
    _emit_ocaml_toolchain_binding(ostream, true, true,  true,  false); // bc_n
    _emit_ocaml_toolchain_binding(ostream, true, true, false,  false); // bc_bc
    _emit_ocaml_toolchain_binding(ostream, true, false, false, false); // n_bc
    _emit_ocaml_default_toolchain_binding(ostream, true);
    // linux
    _emit_ocaml_toolchain_binding(ostream, false, false,  true, false); // n_n
    _emit_ocaml_toolchain_binding(ostream, false, true,  true,  false); // bc_n
    _emit_ocaml_toolchain_binding(ostream, false, true, false,  false); // bc_bc
    _emit_ocaml_toolchain_binding(ostream, false, false, false, false); // n_bc
    _emit_ocaml_default_toolchain_binding(ostream, false);



    // now the ocaml_toolchain_adapters
    fprintf(ostream, "\n");
    fprintf(ostream, "####################################\n");
    fprintf(ostream, "    #### toolchain adapters ####\n");

    _emit_ocaml_toolchain_adapter(ostream, false, true,  false); // n_n
    _emit_ocaml_toolchain_adapter(ostream, true,  true,  false); // bc_n
    _emit_ocaml_toolchain_adapter(ostream, true,  false, false); // bc_bc
    _emit_ocaml_toolchain_adapter(ostream, false, false, false); // n_bc
    /* _emit_ocaml_toolchain_adapter(ostream, false, false, true); //default */

    fclose(ostream);
    utstring_free(ocaml_file);

    /* **************************************************************** */
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/bin");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");

    ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "# generated file - DO NOT EDIT\n");
    fprintf(ostream, "exports_files(glob([\"**\"]))\n");
    fclose(ostream);
    utstring_free(ocaml_file);

    /* **************************************************************** */
    _emit_toolchain_bin_symlinks();
}

void _copy_buildfile(char *buildfile, UT_string *to_file) {
    UT_string *src;
    utstring_new(src);
    utstring_printf(src,
                    "%s/external/opam/bazelfiles/%s",
                    utstring_body(runfiles_root),
                    buildfile);
    int rc = access(utstring_body(src), F_OK);
    if (rc != 0) {
        log_error("not found: %s", utstring_body(src));
        fprintf(stderr, "not found: %s\n", utstring_body(src));
        return;
    }

    if (debug) {
        log_debug("copying %s to %s\n",
                  utstring_body(src),
                  utstring_body(to_file));
    }
    errno = 0;
    rc = copyfile(utstring_body(src),
                  utstring_body(to_file));
    if (rc != 0) {
        log_error("copyfile: %s", strerror(errno));
        fprintf(stderr, "ERROR copyfile: %s", strerror(errno));
        log_error("Exiting");
        fprintf(stderr, "Exiting\n");
        exit(EXIT_FAILURE);
    }
}

void _symlink_buildfile(char *buildfile, UT_string *to_file)
{
    UT_string *src;
    utstring_new(src);
    utstring_printf(src,
                    "%s/external/opam/bazelfiles/%s",
                    utstring_body(runfiles_root),
                    buildfile);
    int rc = access(utstring_body(src), F_OK);
    if (rc != 0) {
        log_error("not found: %s", utstring_body(src));
        fprintf(stderr, "not found: %s\n", utstring_body(src));
        return;
    }

    if (debug) {
        log_debug("c_libs: symlinking %s to %s\n",
                  utstring_body(src),
                  utstring_body(to_file));
    }
    errno = 0;
    rc = symlink(utstring_body(src),
                 utstring_body(to_file));
    if (rc != 0) {
        switch (errno) {
        case EEXIST:
            goto ignore;
        case ENOENT:
            log_error("symlink ENOENT: %s", strerror(errno));
            /* log_error("a component of '%s' does not name an existing file",  utstring_body(dst_dir)); */
            fprintf(stderr, "symlink ENOENT: %s\n", strerror(errno));
            /* fprintf(stderr, "A component of '%s' does not name an existing file.\n",  utstring_body(dst_dir)); */
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

void _symlink_ocaml_bigarray(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_bigarray to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking bigarray %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "bigarray") == NULL) */
            if (strncmp("bigarray", direntry->d_name, 8) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
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

void emit_ocaml_bigarray_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_bigarray_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/bigarray");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_bigarray(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_bigarray.BUILD", ocaml_file);
    /* _symlink_buildfile("ocaml_bigarray.BUILD", ocaml_file); */

/*     FILE *ostream; */
/*     ostream = fopen(utstring_body(ocaml_file), "w"); */
/*     if (ostream == NULL) { */
/*         perror(utstring_body(ocaml_file)); */
/*         exit(EXIT_FAILURE); */
/*     } */

/* #include "ocaml_bigarray.BUILD.h" */
/*     fprintf(ostream, "%s", bigarray_buildfile); */

/*     fclose(ostream); */
    utstring_free(ocaml_file);
}

void _symlink_ocaml_compiler_libs(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_compiler_libs to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/compiler-libs",
                    utstring_body(opam_switch_lib));

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
                            tgtdir, direntry->d_name);
            /* log_debug("symlinking %s to %s\n", */
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
}

void emit_ocaml_compiler_libs_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_compiler_libs_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/compiler-libs");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_compiler_libs(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_compiler-libs.BUILD", ocaml_file);
    /* _symlink_buildfile("ocaml_compiler-libs.BUILD", ocaml_file); */

    utstring_renew(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/compiler-libs/common");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/common.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/compiler-libs/bytecomp");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/bytecomp.BUILD", ocaml_file);

    utstring_renew(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/compiler-libs/optcomp");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/optcomp.BUILD", ocaml_file);

    utstring_free(ocaml_file);
}

/* ***************************************** */
void _symlink_ocaml_dynlink(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_dynlink to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking dynlink: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "dynlink") == NULL) */
            if (strncmp("dynlink", direntry->d_name, 7) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
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

void emit_ocaml_dynlink_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_dynlink_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/dynlink");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_dynlink(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_dynlink.BUILD", ocaml_file);
    /* _symlink_buildfile("ocaml_dynlinks.BUILD", ocaml_file); */

    utstring_free(ocaml_file);
}

/* ***************************************** */
void _symlink_ocaml_num(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_num to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking num: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "num") == NULL) */
            if (strncmp("nums.", direntry->d_name, 5) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
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

void emit_ocaml_num_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_num_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/num/core");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_num(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_num.BUILD", ocaml_file);

    utstring_free(ocaml_file);
}

/* ************************************* */
void _symlink_ocaml_str(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_str to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking str: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "str.") == NULL) */
            if (strncmp("str.", direntry->d_name, 4) != 0)
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
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

void emit_ocaml_str_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_str_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/str");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_str(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_str.BUILD", ocaml_file);
    /* _symlink_buildfile("ocaml_str.BUILD", ocaml_file); */

/*     FILE *ostream; */
/*     ostream = fopen(utstring_body(ocaml_file), "w"); */
/*     if (ostream == NULL) { */
/*         perror(utstring_body(ocaml_file)); */
/*         exit(EXIT_FAILURE); */
/*     } */

/* #include "ocaml_str.BUILD.h" */
/*     fprintf(ostream, "%s", str_buildfile); */

/*     fclose(ostream); */
    utstring_free(ocaml_file);
}

void _symlink_ocaml_threads(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_threads to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/threads",
                    utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking threads: %s\n",
                utstring_body(opamdir));
        log_error("Unable to opendir for symlinking threads: %s\n", utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    //FIXME: just open ocaml/threads directly instead of searching for it

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "threads") == NULL) */
            /*     continue; */

            /* log_debug("threads dirent: %s", direntry->d_name); */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
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

void emit_ocaml_threads_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_threads_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/threads");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_threads(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_threads.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

/* ************************************** */
void _symlink_ocaml_unix(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_unix to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking unix: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* printf("dirent: %s/%s\n", */
            /*        utstring_body(opamdir), direntry->d_name); */

            /* if (strncmp(direntry->d_name, "unix", 4) != 0) */
            /*      continue; */

            if (strncmp("unix", direntry->d_name, 4) != 0)
            /* if (strcasestr(direntry->d_name, "unix") == NULL) */
                continue;

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
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

void emit_ocaml_unix_pkg(char *switch_name)
{
    /* if (verbose) */
    /*     printf("emit_ocaml_unix_pkg: %s\n", switch_name); */
    if (debug)
        log_debug("emit_ocaml_unix_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/unix");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_unix(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_unix.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

/* ***************************************** */
void _symlink_ocaml_ocamldoc(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_ocamldoc to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/ocamldoc",
                    utstring_body(opam_switch_lib));

    UT_string *src;
    utstring_new(src);
    UT_string *dst;
    utstring_new(dst);
    int rc;

    DIR *d = opendir(utstring_body(opamdir));
    if (d == NULL) {
        fprintf(stderr, "Unable to opendir for symlinking ocamldoc: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strcasestr(direntry->d_name, "ocamldoc") == NULL) */
            /* if (strncmp("odoc", direntry->d_name, 4) != 0) */
            /*     continue; */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            tgtdir, direntry->d_name);
            /* printf("symlinking %s to %s\n", */
            /*        utstring_body(src), */
            /*        utstring_body(dst)); */
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

void emit_ocaml_ocamldoc_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_ocamldoc_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/ocamldoc");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_ocamldoc(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_ocamldoc.BUILD", ocaml_file);

    utstring_free(ocaml_file);
}

/* **************************************************************** */
void _symlink_ocaml_c_hdrs(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_c_hdrs to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml/caml",
                    utstring_body(opam_switch_lib));

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
        fprintf(stderr, "Unable to opendir for symlinking str: %s\n",
                utstring_body(opamdir));
        /* exit(EXIT_FAILURE); */
        /* this can happen if a related pkg is not installed */
        /* example, see topkg and topkg-care */
        return;
    }

    struct dirent *direntry;
    while ((direntry = readdir(d)) != NULL) {
        if(direntry->d_type==DT_REG){
            /* if (strncmp("lib", direntry->d_name, 3) != 0) */
            /*     continue;       /\* no match *\/ */
            //FIXME: check for .h?

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(obazldir), /* tgtdir, */
                            direntry->d_name);
            if (debug) {
                log_debug("c_hdrs: symlinking %s to %s\n",
                          utstring_body(src),
                          utstring_body(dst));
            }
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
            if (rc != 0) {
                if (errno != EEXIST) {
                    fprintf(stdout, "c_hdrs: symlink errno: %d: %s\n",
                            errno, strerror(errno));
                    fprintf(stdout, "c_hdrs src: %s, dst: %s\n",
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

void _symlink_ocaml_c_libs(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_c_libs to %s\n", tgtdir);

    UT_string *opamdir;
    utstring_new(opamdir);
    utstring_printf(opamdir, "%s/ocaml",
                    utstring_body(opam_switch_lib));

    UT_string *obazldir;
    utstring_new(obazldir);
    utstring_printf(obazldir, "%s/lib", tgtdir);
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
            if (strncmp("lib", direntry->d_name, 3) != 0)
                continue;       /* no match */

            utstring_renew(src);
            utstring_printf(src, "%s/%s",
                            utstring_body(opamdir), direntry->d_name);
            utstring_renew(dst);
            utstring_printf(dst, "%s/%s",
                            utstring_body(obazldir), /* tgtdir, */
                            direntry->d_name);
            if (debug) {
                log_debug("c_libs: symlinking %s to %s\n",
                          utstring_body(src),
                          utstring_body(dst));
            }
            errno = 0;
            rc = symlink(utstring_body(src),
                         utstring_body(dst));
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

void emit_ocaml_c_api_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_c_api_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/c");
    _symlink_ocaml_c_libs(utstring_body(ocaml_file));
    _symlink_ocaml_c_hdrs(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_c_api.BUILD", ocaml_file);
    /* _symlink_buildfile("ocaml_c_api.BUILD", ocaml_file); */

    utstring_free(ocaml_file);
}

/* **************************************************************** */
void emit_ocaml_bootstrap(char *opam_switch, FILE *bootstrap_FILE)
{
    log_debug("emit_ocaml_bootstrap");

    fprintf(bootstrap_FILE, "    native.local_repository(\n");
    fprintf(bootstrap_FILE, "        name       = \"ocaml\",\n");

    fprintf(bootstrap_FILE, "        path       = ");
    fprintf(bootstrap_FILE, "\"%s/%s\",\n",
            utstring_body(bzl_switch_pfx),
            "ocaml");
    fprintf(bootstrap_FILE, "    )\n\n");

    /* we also always have lib/stublibs, which is not an OPAM pkg */
    fprintf(bootstrap_FILE, "    native.local_repository(\n");
    fprintf(bootstrap_FILE, "        name       = \"stublibs\",\n");

    fprintf(bootstrap_FILE, "        path       = ");
    fprintf(bootstrap_FILE, "\"%s/%s\",\n",
            utstring_body(bzl_switch_pfx),
            "lib/stublibs");
    fprintf(bootstrap_FILE, "    )\n\n");

}

/*
  precondition: bzl_switch_pfx set to here or xdg coswitch
 */
void emit_ocaml_workspace(char *switch_name, FILE *bootstrap_FILE)
{
    if (debug)
        log_debug("emit_ocaml_repo");
    /* printf("emit_ocaml_repo, bzl_switch_pfx: %s\n", */
    /*        utstring_body(bzl_switch_pfx)); */

    emit_ocaml_bootstrap(switch_name, bootstrap_FILE);

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml");
    mkdir_r(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/WORKSPACE.bazel");

    /* printf("writing ws file: %s\n", utstring_body(ocaml_file)); */

    FILE *ostream = fopen(utstring_body(ocaml_file), "w");
    if (ostream == NULL) {
        log_error("fopen: %s: %s", strerror(errno),
                  utstring_body(ocaml_file));
        fprintf(stderr, "fopen: %s: %s", strerror(errno),
                utstring_body(ocaml_file));
        fprintf(stderr, "exiting\n");
        /* perror(utstring_body(ocaml_file)); */
        exit(EXIT_FAILURE);
    }
    fprintf(ostream, "workspace( name = \"ocaml\" )"
            "    # generated file - DO NOT EDIT\n");

    fclose(ostream);

    utstring_renew(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml");

    emit_ocaml_platform_buildfile(switch_name);
    emit_ocaml_toolchain_buildfile(switch_name);

    /* emit_ocaml_stdlib_pkg(switch_name); */

    emit_ocaml_stublibs(switch_name);
    emit_lib_stublibs(switch_name);

    emit_ocaml_compiler_libs_pkg(switch_name);

    emit_ocaml_bigarray_pkg(switch_name);
    emit_ocaml_dynlink_pkg(switch_name);
    emit_ocaml_num_pkg(switch_name);
    emit_ocaml_ocamldoc_pkg(switch_name);
    emit_ocaml_str_pkg(switch_name);
    emit_ocaml_threads_pkg(switch_name);
    // vmthreads? removed in v. 4.08.0?
    emit_ocaml_unix_pkg(switch_name);

    emit_ocaml_c_api_pkg(switch_name);

    // special case:  @ocaml//ffi (csdk) => lib/ocaml/caml
    // special case:  @ctypes ?

    // stdlib?
    // version-dependent: bigarray, num, raw_spacetime

    // bigarray moved to standard lib in v. 4.07, so no need to list
    // as explicit dep. but we expose it as a package anyway.
    /* emit_ocaml_bigarray_buildfile(switch_name); */

    /* utstring_printf(ocaml_file, "/BUILD.bazel"); */

    /* ostream = fopen(utstring_body(ocaml_file), "w"); */
    /* if (ostream == NULL) { */
    /*     perror(utstring_body(ocaml_file)); */
    /*     exit(EXIT_FAILURE); */
    /* } */

    /* fprintf(ostream, */
    /*         "load(\"@rules_ocaml//toolchain:rules.bzl\", \"ocaml_toolchain\")\n"); */
    /* emit_bazel_hdr(ostream); */

    /* fprintf(repo_rules_FILE, "        name       = "); */

    /* char *_pkg_prefix = NULL; */
    /* // ???? */
    /* if (_pkg_prefix == NULL) */
    /*     fprintf(repo_rules_FILE, "\"%s\",\n", pkg_name); */
    /* else { */
    /*     char *s = _pkg_prefix; */
    /*     char *tmp; */
    /*     while(s) { */
    /*         tmp = strchr(s, '/'); */
    /*         if (tmp == NULL) break; */
    /*         *tmp = '.'; */
    /*         s = tmp; */
    /*     } */
    /*     fprintf(repo_rules_FILE, "\"%s.%s\",\n", */
    /*             _pkg_prefix, pkg_name); */
    /* } */

    /* /\* fprintf(repo_rules_FILE, "        environ = [\"OPAM_SWITCH_PREFIX\"],\n"); *\/ */

    /* fprintf(repo_rules_FILE, "        build_file = "); */
    /* if (_pkg_prefix == NULL) */
    /*     fprintf(repo_rules_FILE, */
    /*             "\"@//%s/%s:BUILD.bazel\",\n", */
    /*             /\* buildfile_prefix, *\/ */
    /*             utstring_body(bzl_switch_pfx), */
    /*             pkg_name); */
    /* else */
    /*     fprintf(repo_rules_FILE, */
    /*             "\"@//%s/%s/%s:BUILD.bazel\",\n", */
    /*             /\* buildfile_prefix, *\/ */
    /*             utstring_body(bzl_switch_pfx), */
    /*             _pkg_prefix, pkg_name); */

    /* fprintf(repo_rules_FILE, "        path       = "); */
    /* if (_pkg_prefix == NULL) */
    /*     fprintf(repo_rules_FILE, "\"%s\",\n", */
    /*             /\* opam_switch_prefix, *\/ */
    /*             pkg_name); */
    /* else */
    /*     fprintf(repo_rules_FILE, "\"%s/%s\",\n", */
    /*             /\* opam_switch_prefix, *\/ */
    /*             _pkg_prefix, pkg_name); */

    /* /\* **************************************************************** *\/ */
    /* /\*  now subpackages *\/ */
    /* /\* **************************************************************** *\/ */

    /* if ((strncmp(_pkg->name, "threads", 7) == 0) */
    /*     && strlen(_pkg->name) == 7) { */
    /*     ; /\* special case: skip threads subpkgs *\/ */
    /* } else { */
    /*     int subpkg_ct = obzl_meta_package_subpkg_count(_pkg); */
    /*     if (subpkg_ct > 0) { */
    /*         fprintf(repo_rules_FILE, "        subpackages = {\n"); */
    /*         emit_new_local_subpkg_entries(repo_rules_FILE, */
    /*                                       _pkg, */
    /*                                       NULL, */
    /*                                       _pkg_prefix, */
    /*                                       _pkg->name); /\* filedeps_path *\/ */
    /*         fprintf(repo_rules_FILE, "        }\n"); */
    /*     } */
    /* } */
    /* /\* **************************************************************** *\/ */

    /* fprintf(repo_rules_FILE, "    )\n\n"); */
    /* fclose(ostream); */
    /* utstring_free(ocaml_file); */

    /* UT_string *dst_dir; */
    /* utstring_new(dst_dir); */
    /* utstring_concat(dst_dir, bzl_switch_pfx); */
    /* utstring_printf(dst_dir, "/bin"); */

    /* UT_string *src_dir; // relative to opam_switch_lib */
    /* utstring_new(src_dir); */
    /* utstring_printf(src_dir, "bin"); */
}
