#include <stdlib.h>

#include "log.h"
#include "utstring.h"

#include "xdg_config.h"

UT_string *xdg_cache_home;
UT_string *xdg_config_home;
UT_string *xdg_data_home;
UT_string *xdg_opam_root;

EXPORT void xdg_configure(void)
{
    log_debug("xdg_configure");

    char *home = getenv("HOME");
    if (home) {
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
        printf("$XDG_CACHE_HOME (from env): %s\n",
               utstring_body(xdg_cache_home));
    } else {
        utstring_printf(xdg_cache_home, "%s/.cache", home);
        printf("xdg_cache_home: %s\n", utstring_body(xdg_cache_home));
    }

    utstring_new(xdg_config_home);
    val = getenv("XDG_CONFIG_HOME");
    if (val) {
        utstring_printf(xdg_config_home, "%s", val);
        printf("$XDG_CONFIG_HOME (from env): %s\n",
               utstring_body(xdg_config_home));
    } else {
        utstring_printf(xdg_cache_home, "%s/.config", home);
        printf("xdg_config_home: %s\n", utstring_body(xdg_config_home));
    }

    utstring_new(xdg_data_home);
    val = getenv("XDG_DATA_HOME");
    if (val) {
        utstring_printf(xdg_data_home, "%s", val);
        printf("$XDG_DATA_HOME (from env): %s\n",
               utstring_body(xdg_data_home));
    } else {
        utstring_printf(xdg_data_home, "%s/.local/share", home);
        printf("xdg_data_home: %s\n", utstring_body(xdg_data_home));
    }

    utstring_new(xdg_opam_root);
    utstring_concat(xdg_opam_root, xdg_data_home);
    utstring_printf(xdg_opam_root, "/obazl/opam");
}
