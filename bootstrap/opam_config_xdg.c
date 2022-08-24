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
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#endif

#define BUFSZ 4096

#include "log.h"
#include "opam_config_xdg.h"

extern int errnum;
/* bool local_opam; */

#if INTERFACE
#include "utstring.h"
#endif
UT_string *xdg_cache_home;
UT_string *xdg_config_home;
UT_string *xdg_data_home;
UT_string *xdg_coswitch_root;

EXPORT void xdg_configure(void)
{
    char *home = getenv("HOME");
    if (home) {
        if (verbose)
            printf("$HOME: %s\n", home);
    } else {
        printf("$HOME not in env\n");
        exit(EXIT_FAILURE);
    }

    char *val;

    utstring_new(xdg_cache_home);
    val = getenv("XDG_CACHE_HOME");
    if (val) {
        utstring_printf(xdg_cache_home, "%s", val);
        if (verbose)
            printf("$XDG_CACHE_HOME (from env): %s\n",
                   utstring_body(xdg_cache_home));
    } else {
        utstring_printf(xdg_cache_home, "%s/.cache", home);
        if (verbose)
            printf("xdg_cache_home: %s\n", utstring_body(xdg_cache_home));
    }

    utstring_new(xdg_config_home);
    val = getenv("XDG_CONFIG_HOME");
    if (val) {
        utstring_printf(xdg_config_home, "%s", val);
        if (verbose)
            printf("$XDG_CONFIG_HOME (from env): %s\n",
                   utstring_body(xdg_config_home));
    } else {
        utstring_printf(xdg_cache_home, "%s/.config", home);
        if (verbose)
            printf("xdg_config_home: %s\n", utstring_body(xdg_config_home));
    }

    utstring_new(xdg_data_home);
    val = getenv("XDG_DATA_HOME");
    if (val) {
        utstring_printf(xdg_data_home, "%s", val);
        if (verbose)
            printf("$XDG_DATA_HOME (from env): %s\n",
                   utstring_body(xdg_data_home));
    } else {
        utstring_printf(xdg_data_home, "%s/.local/share", home);
        if (verbose)
            printf("xdg_data_home: %s\n", utstring_body(xdg_data_home));
    }

    utstring_new(xdg_coswitch_root);
    utstring_concat(xdg_coswitch_root, xdg_data_home);
    utstring_printf(xdg_coswitch_root, "/obazl/opam");
}

/* #define OPAM_LOADER "BOOTSTRAP.bzl" */


