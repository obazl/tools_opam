/* This file was automatically generated.  Do not edit! */
#undef INTERFACE
#include "uthash.h"
extern char basedir[PATH_MAX];
#include <stdbool.h>
#include "utarray.h"
#define LOCAL static
typedef int(*file_handler)(char *rootdir,char *pkg,char *metafile);
LOCAL int meta_walk_impl(char *basedir,char *bazel_pkg,char *directory,bool linkfiles,file_handler handle_meta);
int meta_walk(char *srcroot,bool linkfiles,file_handler handle_meta);
char *mkdir_r(char *base,char *path);
#define EXPORT
char *mystrcat(char *dest,char *src);
#define INTERFACE 0
extern int rc;
extern int rc;
#if defined(LINUX                    /* FIXME */)
#include <linux/limits.h>
#endif
#if !(defined(LINUX                    /* FIXME */))
#include <limits.h>
#endif
extern int errnum;
extern int errnum;
extern int errnum;
#define EXPORT_INTERFACE 0
