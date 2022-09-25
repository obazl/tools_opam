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

void _emit_ocaml_bin_symlinks(void) // UT_string *dst_dir, UT_string *src_dir)
{
    log_debug("_emit_ocaml_bin_symlinks");

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

/* void emit_ocaml_stdlib_pkg(char *switch_name) */
/* { */
/*     if (debug) */
/*         log_debug("emit_ocaml_stdlib_pkg"); */

/*     UT_string *ocaml_file; */
/*     utstring_new(ocaml_file); */
/*     utstring_concat(ocaml_file, bzl_switch_pfx); */
/*     utstring_printf(ocaml_file, "/ocaml/lib/stdlib"); */
/*     mkdir_r(utstring_body(ocaml_file)); */

/*     _symlink_ocaml_stdlib(utstring_body(ocaml_file)); */

/*     utstring_printf(ocaml_file, "/BUILD.bazel"); */

/*     _copy_buildfile("ocaml_stdlib.BUILD", ocaml_file); */
/*     utstring_free(ocaml_file); */
/* } */

/* void _symlink_ocaml_stdlib(char *tgtdir) */
/* { */
/*     if (debug) */
/*         log_debug("_symlink_ocaml_stdlib to %s\n", tgtdir); */

/*     UT_string *opamdir; */
/*     utstring_new(opamdir); */
/*     utstring_printf(opamdir, "%s/ocaml", utstring_body(opam_switch_lib)); */

/*     UT_string *src; */
/*     utstring_new(src); */
/*     UT_string *dst; */
/*     utstring_new(dst); */
/*     int rc; */

/*     DIR *d = opendir(utstring_body(opamdir)); */
/*     if (d == NULL) { */
/*         fprintf(stderr, "Unable to opendir for symlinking stdlib: %s\n", */
/*                 utstring_body(opamdir)); */
/*         /\* exit(EXIT_FAILURE); *\/ */
/*         return; */
/*     } */

/*     struct dirent *direntry; */
/*     while ((direntry = readdir(d)) != NULL) { */
/*         if(direntry->d_type==DT_REG){ */
/*             if (strncmp("stdlib", direntry->d_name, 6) != 0) */
/*                 continue; */

/*             utstring_renew(src); */
/*             utstring_printf(src, "%s/%s", */
/*                             utstring_body(opamdir), direntry->d_name); */
/*             utstring_renew(dst); */
/*             utstring_printf(dst, "%s/%s", */
/*                             tgtdir, direntry->d_name); */
/*             /\* printf("symlinking %s to %s\n", *\/ */
/*             /\*        utstring_body(src), *\/ */
/*             /\*        utstring_body(dst)); *\/ */
/*             rc = symlink(utstring_body(src), */
/*                          utstring_body(dst)); */
/*             if (rc != 0) { */
/*                 if (errno != EEXIST) { */
/*                     perror(utstring_body(src)); */
/*                     fprintf(stderr, "exiting\n"); */
/*                     exit(EXIT_FAILURE); */
/*                 } */
/*             } */
/*         } */
/*     } */
/*     closedir(d); */
/* } */

void emit_ocaml_runtime_pkg(char *switch_name)
{
    if (debug)
        log_debug("emit_ocaml_runtime_pkg");

    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/runtime");
    mkdir_r(utstring_body(ocaml_file));

    _symlink_ocaml_runtime(utstring_body(ocaml_file));

    utstring_printf(ocaml_file, "/BUILD.bazel");

    _copy_buildfile("ocaml_runtime.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

void _symlink_ocaml_runtime(char *tgtdir)
{
    if (debug)
        log_debug("_symlink_ocaml_runtime to %s\n", tgtdir);

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
                if (strncmp("std_exit", direntry->d_name, 8) != 0)
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

void _emit_lib_stublibs_symlinks(char *_dir)
{
    log_debug("_emit_lib_stublibs_symlinks: %s", _dir);

    UT_string *dst_dir;
    utstring_new(dst_dir);
    utstring_concat(dst_dir, bzl_switch_pfx);
    utstring_printf(dst_dir, "%s", _dir); //"/lib/stublibs");
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
    utstring_printf(ocaml_file, "/lib/stublibs/stublibs");
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
    _emit_lib_stublibs_symlinks("/lib/stublibs/stublibs");
}

void emit_ocaml_platform_buildfiles(void)
{
    log_debug("emit_ocaml_platform_buildfiles\n");
    UT_string *ocaml_file;

    /* platform definitions */
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/host/bazel");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("host/bazel/BUILD.bazel", ocaml_file);

    utstring_renew(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/host/build");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("host/build/BUILD.bazel", ocaml_file);

    utstring_renew(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/host/target");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("host/target/BUILD.bazel", ocaml_file);

    /* toolchain selectors */
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/selectors/local");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/local/BUILD.bazel", ocaml_file);

    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/selectors/macos/x86_64");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/macos/x86_64/BUILD.bazel", ocaml_file);

    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/selectors/macos/arm");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/macos/arm/BUILD.bazel", ocaml_file);

    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/selectors/linux/x86_64");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/linux/x86_64/BUILD.bazel", ocaml_file);

    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/selectors/linux/arm");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/selectors/linux/arm/BUILD.bazel", ocaml_file);

    /* toolchain options */
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/profiles");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/profiles/BUILD.bazel", ocaml_file);

    /* toolchain adapters */
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/adapters/local");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/local/BUILD.bazel", ocaml_file);

    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/adapters/linux/x86_64");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/linux/x86_64/BUILD.bazel", ocaml_file);

    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/adapters/linux/arm");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/linux/arm/BUILD.bazel", ocaml_file);

    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/adapters/macos/x86_64");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/macos/x86_64/BUILD.bazel", ocaml_file);

    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchain/adapters/macos/arm");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("toolchain/adapters/macos/arm/BUILD.bazel", ocaml_file);

    utstring_free(ocaml_file);
}

void emit_ocaml_toolchain_buildfile(void)
{
    log_debug("emit_ocaml_toolchain_buildfile\n");
    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/toolchains");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("ocaml_toolchains.BUILD", ocaml_file);
    utstring_free(ocaml_file);
}

void emit_ocaml_bin_dir(void)
{
    log_debug("emit_ocaml_bin_dir\n");
    UT_string *ocaml_file;
    utstring_new(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/bin");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");

    FILE *ostream;
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
    _emit_ocaml_bin_symlinks();
}

void _copy_buildfile(char *buildfile, UT_string *to_file) {
    UT_string *src;
    utstring_new(src);
    utstring_printf(src,
                    "%s/external/opam/templates/%s",
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
                    "%s/external/opam/templates/%s",
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

    utstring_renew(ocaml_file);
    utstring_concat(ocaml_file, bzl_switch_pfx);
    utstring_printf(ocaml_file, "/ocaml/compiler-libs/toplevel");
    mkdir_r(utstring_body(ocaml_file));
    utstring_printf(ocaml_file, "/BUILD.bazel");
    _copy_buildfile("compiler_libs/toplevel.BUILD", ocaml_file);

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
        log_debug("emit_ocaml_workspace");
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

    emit_ocaml_bin_dir();

    emit_ocaml_platform_buildfiles();
    /* emit_ocaml_toolchain_buildfile(); */

    /* FIXME: decide on stdlib or runtime */
    /* emit_ocaml_stdlib_pkg(switch_name); */
    emit_ocaml_runtime_pkg(switch_name);

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
}
