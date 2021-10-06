/*
  routines for crawling a fs tree, processing ocamlfind META files
 */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>
#endif
#if EXPORT_INTERFACE
#include <stdbool.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "log.h"

#if EXPORT_INTERFACE
#include "uthash.h"
#include "utarray.h"
#endif

#include "meta_fs.h"

/* LOCAL int verbosity = 0; */
int errnum;
int rc;

#if INTERFACE
typedef int (*file_handler)(char *rootdir,
                            char *bzlroot,
                            /* char *imports_path, */
                            char *pkgdir,
                            char *metafile);
#endif

// FIXME: replace with utstrings
char* mystrcat( char* dest, char* src )
{
     while (*dest) dest++;
     while ( (*dest++ = *src++) );
     return --dest;
}

EXPORT char *mkdir_r(char *base, char *path)
{
    /* log_debug("entering mkdir_r base: '%s', path: '%s'\n", base, path); */

    char *buf_dirname[PATH_MAX];
    char *buf_basename[PATH_MAX];

    if ( access(base, R_OK) ) {
        /* base does not exist */
        /* log_debug("mkdir_r recurring to create base: %s, path: %s", */
        /*           dirname_r(base, buf_dirname), basename_r(base, buf_basename)); */
        mkdir_r(dirname_r(base, (char*)buf_dirname), basename_r(base, (char*)buf_basename));
    }
    /* now base should exist */
    if ( access(base, R_OK) ) {
        log_fatal("no base: %s", base);
        exit(EXIT_FAILURE);
    /* } else { */
        /* log_info("base exists: %s, %s", base, path); */
    }

    if ( strlen(path) == 0 ) return base;

    if ( !strncmp(path, ".", 1) ) return base;

    char work[PATH_MAX];
    work[0] = '\0';
    char last_seg[PATH_MAX];
    last_seg[0] = '\0';

    char *bn = basename(path);
    mystrcat(last_seg, bn);
    /* printf("mkdir_r last_seg: %s\n", last_seg); */
    if ( ! strncmp(last_seg, path, PATH_MAX) ) { // 0 (false) means equal, so !0 means true
        /* printf("mkdir_r bottomed out at %s\n", path); */
        sprintf(work, "%s/%s", base, last_seg);
        /* log_debug("making dir1: %s\n", work); */
        rc = mkdir(work, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        if (rc != 0) {
            errnum = errno;
            if (errnum == EEXIST) {
                /* already exists */
                ;
            } else {
                fprintf(stderr, "mkdir failure\n");
                perror(work);
                fprintf(stderr, "Value of errno: %d\n", errnum);
                fprintf(stderr, "mkdir error %s\n", strerror( errnum ));
                free(work);
                exit(EXIT_FAILURE);
            }
        }
        /* log_debug("mkdired 1: %s\n", work); */
        char *real = realpath(work, NULL);
        /* log_debug("\trealpath: %s\n", real); */
        free(real);
        return strndup(work, PATH_MAX);
    } else {
        /* chop off last seg and recur */
        char *d = dirname(path);

        // RECUR
        /* log_debug("mkdir_r recurring on %s with pending last_seg %s\n", d, last_seg); */
        char *so_far = mkdir_r(base, d);
        /* log_debug("mkdir_r resuming after %s with pending last_seg %s\n", d, last_seg); */

        /* log_debug("mkdir_r so far: %s\n", so_far); */
        sprintf(work, "%s/%s", so_far, last_seg);
        if ( access(work, R_OK) ) {
            /* work does not exist */
            /* log_info("mkdir_r mking dir: %s\n", work); */
            rc = mkdir(work, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            if (rc != 0) {
                errnum = errno;
                if (errnum == EEXIST) {
                    /* already exists (should not happen) */
                    ;
                } else {
                    fprintf(stderr, "mkdir failure");
                    perror(work);
                    free(so_far);
                    free(work);
                    exit(EXIT_FAILURE);
                }
            }
        }
        /* log_debug("mkdired 2: %s\n", work); */
        free(so_far);
        return strndup(work, PATH_MAX);
    }
}

EXPORT int meta_walk(char *srcroot,
                     char *bzlroot,
                     bool linkfiles,
                     /* char *file_to_handle, /\* 'META' or NULL *\/ */
                     file_handler handle_meta)
{
    if (handle_meta == NULL) {
        log_error("walk called with NULL callback fn ptr.");
        return -1;
    }
    return meta_walk_impl(srcroot, bzlroot,
                     "",        /* bazel_pkg */
                     "",        /* directory */
                     linkfiles,
                     /* file_to_handle, */
                     handle_meta);
}

/*
  starting at basedir:
      if it contains META, invoke callback
      for each directory in basedir, recur
 */
LOCAL int meta_walk_impl(char *basedir,
                     char *bzlroot,
                    char *bazel_pkg,
                    char *directory,
                    /* char *out_directory, */
                    bool linkfiles,
                    /* char *file_to_handle, */
                    file_handler handle_meta)
{
    /* log_trace("meta_walk_impl %s\n", basedir); //, out_directory); */
    /* log_trace("meta_walk_impl directory: %s\n", directory); */
    /* log_trace("meta_walk_impl bazel_pkg: %s\n", bazel_pkg); */

    char currdir[PATH_MAX];
    currdir[0] = '\0';
    mystrcat(currdir, basedir);
    if (strnlen(directory, PATH_MAX) > 0) {
        mystrcat(currdir, "/");
        mystrcat(currdir, directory);
    }

    char currpkg[PATH_MAX];
    currpkg[0] = '\0';
    if (strnlen(bazel_pkg, PATH_MAX) > 0) {
        mystrcat(currpkg, bazel_pkg);
        if (strnlen(directory, PATH_MAX) > 0) {
            mystrcat(currpkg, "/");
            mystrcat(currpkg, directory);
        }
    } else {
        mystrcat(currpkg, directory);
    }
    /* log_debug("currpkg: %s", currpkg); */

    struct dirent *dir_entry;
    errno = 0;
    /* printf("opening dir %s\n", currdir); */
    /* d = opendir("/usr/local/lib/coq/plugins"); // coqlib); */
    DIR *some_dir;
    some_dir = opendir(currdir);
    if (some_dir == NULL) {
        errnum = errno;
        printf("opendir failure for %s", currdir);
        fprintf(stderr, "Value of errno: %d\n", errnum);
        fprintf(stderr, "opendir error %s: %s\n", currdir, strerror( errnum ));
        exit(1);
        /* return(-1); */
    }
    /* printf("opened dir %s\n", currdir); */

    /* to cover all subdirs we have to readdir everything in the directory */
    /* NB: readdir_r is deprecated */
    if (some_dir) {
        while ((dir_entry = readdir(some_dir)) != NULL) {
            /* NB: dir_entry "remains valid until the next call to readdir" - do not use it after recurring.*/
            if ( dir_entry->d_type == DT_DIR
                 && ( (dir_entry->d_name[0] != '.')
                      || // deal with gramlib/.pack
                      ((dir_entry->d_name[1] != '.')
                      &&
                      (strlen(dir_entry->d_name) > 1)
                       ))
                 ) {
                /* log_debug("recurring on subdir %s for outdir: %s\n", dir_entry->d_name, outdir); */
                meta_walk_impl(currdir, bzlroot,
                               currpkg, dir_entry->d_name,
                               linkfiles,
                               /* file_to_handle, */
                               handle_meta);
                /* log_debug("handled subdir %s\n", dir_entry->d_name); */
            } else {
                if (dir_entry->d_type == DT_REG) {
                    if ( (strlen(dir_entry->d_name) == 4)
                         && (strncmp(dir_entry->d_name, "META", 4) == 0) ) {
                        int rc = handle_meta(basedir, bzlroot,
                                             currpkg, dir_entry->d_name);
                        if (rc) {
                            log_error("handle_meta fail for %s/%s", currpkg, dir_entry->d_name);
                            return rc;
                        }
                    } else {
                        // FIXME: check for foo.META
                    }
                } else {
                    /* not a dir nor a regular file */
                    //FIXME: what about symlinked (DT_LNK) META files and subdirs?
                    ;
                }
            }
        }
        closedir(some_dir);
    }
    return(0);
}


