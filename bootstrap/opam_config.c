#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>              /* open() */
#include <libgen.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if INTERFACE
#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>
#endif
#endif

#define BUFSZ 4096

#include "log.h"
#include "opam_config.h"

int errnum;
/* bool local_opam; */

#if INTERFACE
#define OPAM_BOOTSTRAP "BOOTSTRAP.bzl"
#endif
/* global: we write one local_repository rule per build file */
FILE *bootstrap_FILE;

#if INTERFACE
#include "utarray.h"
#include "utstring.h"
#endif

UT_string *opam_switch_root;
UT_string *opam_switch_name;
UT_string *opam_switch_pfx;
UT_string *opam_switch_bin;
UT_string *opam_switch_lib;

/* extern UT_string *bazel_pkg_root; */
/* extern UT_string *build_bazel_file; */

UT_string *bzl_lib;
UT_string *bzl_bin;
UT_string *bzl_bin_link;
UT_string *bzl_lib_link;

UT_array *opam_packages;

UT_string *bzl_switch_pfx; // outdir for build files etc. mirrors opam switch

void opam_config_init(void)
{
    utstring_new(opam_switch_root);
    utstring_new(opam_switch_name);
    utstring_new(opam_switch_pfx);
    utstring_new(opam_switch_bin);
    utstring_new(opam_switch_lib);
    utstring_new(bzl_switch_pfx);
}

void opam_config_free(void)
{
    utstring_free(opam_switch_root);
    utstring_free(opam_switch_name);
    utstring_free(opam_switch_pfx);
    utstring_free(opam_switch_bin);
    utstring_free(opam_switch_lib);
    utstring_free(bzl_switch_pfx);
}
