#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>              /* open() */
#include <libgen.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if INTERFACE
#ifdef LINUX
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#endif

#include "log.h"
#include "opam_coswitch.h"

int errnum;
/* bool local_opam; */

EXPORT void opam_coswitch_remove(char *coswitch) {
    printf("opam_coswitch_remove: %s\n", coswitch);

    if (access(utstring_body(xdg_coswitch_root), F_OK) != 0) {
        log_info("XDG coswitch root '%s' not found.\n",
                 utstring_body(xdg_coswitch_root));
        fprintf(stderr, "XDG coswitch root '%s' not found.\n",
                utstring_body(xdg_coswitch_root));
    } else {
        printf("xdg_coswitch_root: %s\n",
               utstring_body(xdg_coswitch_root));

        UT_string *coswitch_dir;
        utstring_new(coswitch_dir);
        utstring_printf(coswitch_dir,
                        "%s/%s",
                        utstring_body(xdg_coswitch_root),
                        coswitch);
        printf("target coswitch: %s\n",
               utstring_body(coswitch_dir));
        if (access(utstring_body(coswitch_dir), F_OK) != 0) {
            /* fixme: better msg */
            log_error("coswitch NOT FOUND: %s; exiting.",
                      utstring_body(coswitch_dir));
            fprintf(stderr, "coswitch NOT FOUND: %s; exiting.\n",
                    utstring_body(xdg_coswitch_root));
            utstring_free(coswitch_dir);
            exit(EXIT_FAILURE);
        }
        printf("removing coswitch: %s\n",
               utstring_body(coswitch_dir));
        UT_string *s;
        utstring_new(s);
        utstring_printf(s, "rm -rf %s", utstring_body(coswitch_dir));
        run_cmd(utstring_body(s), true);
        utstring_free(s);
    }
}

EXPORT void opam_coswitch_set(char *coswitch) {
    if (debug)
        printf("opam_coswitch_set: '%s'\n", coswitch);

    if (strncmp(coswitch, "here", 4) == 0) {
        write_here_coswitch_file();
        return;
    }

    if (access(utstring_body(xdg_coswitch_root), F_OK) != 0) {
        log_info("XDG coswitch root '%s' not found.\n",
                 utstring_body(xdg_coswitch_root));
        fprintf(stderr, "XDG coswitch root '%s' not found.\n",
                utstring_body(xdg_coswitch_root));
    } else {
        if (debug)
            printf("xdg_coswitch_root: %s\n",
                   utstring_body(xdg_coswitch_root));

        UT_string *coswitch_bootstrapper;
        utstring_new(coswitch_bootstrapper);
        utstring_printf(coswitch_bootstrapper,
                        "%s/%s/BOOTSTRAP.bzl",
                        utstring_body(xdg_coswitch_root),
                        coswitch);

        if (debug)
            printf("target bootstrapper: %s\n",
                   utstring_body(coswitch_bootstrapper));

        if (access(utstring_body(coswitch_bootstrapper), F_OK) != 0) {
            /* fixme: better msg */
            log_error("coswitch NOT FOUND: %s/%s; exiting.",
                      utstring_body(xdg_coswitch_root),coswitch);
            fprintf(stderr, "coswitch NOT FOUND: %s/%s; exiting.\n",
                      utstring_body(xdg_coswitch_root),coswitch);
            utstring_free(coswitch_bootstrapper);
            exit(EXIT_FAILURE);
        }

        UT_string *coswitch_file;
        utstring_new(coswitch_file);
        utstring_printf(coswitch_file, "%s/COSWITCH.bzl",
                        getcwd(NULL, 0));
        if (verbose)
            printf("writing: %s\n", utstring_body(coswitch_file));

        errno = 0;
        FILE *coswitch_FILE = fopen(utstring_body(coswitch_file), "w");
        if (coswitch_FILE == NULL) { /* fail */
            if (errno == EACCES) {
                printf("ERROR: failed to fopen %s: %s\n",
                       utstring_body(coswitch_file),
                       strerror(errno));
                exit(EXIT_FAILURE);
            } else {
                log_error("fopen %s: %s",
                          HERE_COMPILER_FILE,
                          strerror(errno));
                fprintf(stderr, "fopen %s: %s",
                        HERE_COMPILER_FILE,
                        strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        UT_string *cmd;
        utstring_new(cmd);
        utstring_printf(cmd,
                        "opam var ocaml:version --switch %s",
                        coswitch);
        char *compiler_version = run_cmd(utstring_body(cmd), false);
        if (compiler_version != NULL) {
            if (verbose) {
                log_info("coswitch compiler version: %s\n",
                         compiler_version);
                printf("coswitch compiler version: %s\n",
                       compiler_version);
            }
        } else {
            if (verbose) {
                log_info("Coswitch compiler version: %s\n",
                         compiler_version);
                printf("Coswitch compiler version: %s\n",
                       compiler_version);
            }
        }

        char *compiler_variants = get_compiler_variants(coswitch);
        /* utstring_renew(cmd); */
        /* utstring_printf(cmd, */
        /*                 "opam var ocaml-variants:version --switch %s", */
        /*                 coswitch); */
        /* char *compiler_variants = run_cmd(utstring_body(cmd)); */
        if (compiler_variants != NULL) {
            if (verbose) {
                log_info("coswitch ocaml-variants: %s\n",
                         compiler_version);
                printf("coswitch ocaml-variants: %s\n",
                       compiler_version);
            }
        }
        utstring_free(cmd);

        fprintf(coswitch_FILE, "# generated file - DO NOT EDIT - DO NOT SAVE TO VERSION CONTROL\n");
        fprintf(coswitch_FILE, "# coswitch: shared switch %s\n\n",
                coswitch);

        /* fprintf(coswitch_FILE, "# switch name: %s\txdg: %s\n", */
        /*         coswitch, utstring_body(xdg_coswitch_root)); */
        /* fprintf(coswitch_FILE, "#   compiler version: %s\n", */
        /*         compiler_version); */
        /* if (compiler_variants != NULL) */
        /*     fprintf(coswitch_FILE, "#   ocaml-variants:   %s\n\n", */
        /*             compiler_variants); */

        fprintf(coswitch_FILE, "def register():\n");
        fprintf(coswitch_FILE, "    native.new_local_repository(\n");
        fprintf(coswitch_FILE, "    name       = \"coswitch\",\n");
        fprintf(coswitch_FILE, "    path       = \"%s/%s\",\n",
                utstring_body(xdg_coswitch_root), coswitch);
        fprintf(coswitch_FILE, "    build_file_content = \"#\"\n");
        fprintf(coswitch_FILE, "    )\n");

        fclose(coswitch_FILE);
        chmod(utstring_body(coswitch_file), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (debug)
            log_debug("wrote %s", coswitch_file);
        utstring_free(coswitch_file);
    }
}

EXPORT void opam_coswitch_show(void) {
    printf("opam_coswitch_show\n");

    UT_string *coswitch_file;
    utstring_new(coswitch_file);
    utstring_printf(coswitch_file, "%s/COSWITCH.bzl",
                    getcwd(NULL, 0));
    printf("COSWITCH.bzl: %s\n", utstring_body(coswitch_file));

    errno = 0;
    FILE *coswitch_FILE = fopen(utstring_body(coswitch_file), "r");
    if (coswitch_FILE == NULL) { /* fail */
        if (errno == EACCES) {
            printf("ERROR: failed to fopen %s: %s\n",
                   utstring_body(coswitch_file),
                   strerror(errno));
            exit(EXIT_FAILURE);
        } else {
            log_error("fopen %s: %s",
                      HERE_COMPILER_FILE,
                      strerror(errno));
            fprintf(stderr, "fopen %s: %s",
                    HERE_COMPILER_FILE,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    char buf[1024] = {0};
    int len = 0;
    char *result;

    for (int i=0; i<4; i++) {
        result = fgets(buf, 1024, coswitch_FILE);
        if (result == NULL) {
            log_error("fgets fail on %s, err: %s\n", coswitch_FILE);
            fprintf(stderr, "ERROR: fgets fail on %s, err: %s\n", coswitch_FILE);
            exit(EXIT_FAILURE);
        }
        printf(buf);
    }

    fclose(coswitch_FILE);
    utstring_free(coswitch_file);
}
