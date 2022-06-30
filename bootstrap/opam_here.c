#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utstring.h"

#include "log.h"
#include "opam_here.h"

#if INTERFACE
#define HERE_OPAM_ROOT ".opam"
#define HERE_COSWITCH_ROOT OBAZL_ROOT "/opam"
#define HERE_COMPILER "/here.compiler"
#define HERE_COMPILER_FILE HERE_COSWITCH_ROOT HERE_COMPILER
//FIXME: rename, HERE_OBAZL_OPAM_WSS?
#define HERE_SWITCH_BAZEL_ROOT HERE_COSWITCH_ROOT "/here"
#define HERE_OBAZL_OPAM_WSS_OCAML HERE_COSWITCH_ROOT "/here/ocaml"
#define COSWITCH_LIB "/lib"
#define HERE_OBAZL_OPAM_WSS_PKGS HERE_SWITCH_BAZEL_ROOT COSWITCH_LIB
//"/here/pkgs"
#define HERE_OBAZL_OPAM_WSS_STUBLIBS HERE_COSWITCH_ROOT "/here/stublibs"
#define HERE_SWITCH_NAME "here"

#define OPAM_SRC_ROOT HERE_OPAM_ROOT "/" HERE_SWITCH_NAME "/.opam-switch/sources"

#endif

void write_here_compiler_file(char *compiler_version)
{
#if defined(DEBUG_TRACE)
    log_trace("write_here_compiler_file: %s", compiler_version);
#endif
    if (compiler_version == NULL) {
        compiler_version = get_compiler_version(NULL);
        /* compiler_version = run_cmd("opam var" */
        /*                            " ocaml-base-compiler:version" */
        /*                            " --root " HERE_OPAM_ROOT */
        /*                            " --switch " HERE_SWITCH_NAME, */
        /*                            false); */
        if (verbose)
            printf("here switch ocamlc version: %s\n", compiler_version);
    }

    char *compiler_variants = get_compiler_variants(NULL);

    if (dry_run) {
        printf("Wrote compiler version %s to %s\n",
               compiler_variants ?compiler_variants :compiler_version,
               HERE_COMPILER_FILE);
        return;
    }
    mkdir_r(HERE_COSWITCH_ROOT);
    /* printf("opam_init_here opening " HERE_COMPILER_FILE "\n"); */
    FILE *here_compiler_file = fopen(HERE_COMPILER_FILE, "w");
    if (here_compiler_file == NULL) { /* fail */
        if (errno == EACCES) {
            int rc = unlink(HERE_COMPILER_FILE);
            if (rc == 0) {
                /* log_debug("unlinked existing %s\n", HERE_COMPILER_FILE); */
                here_compiler_file = fopen(HERE_COMPILER_FILE, "w");
                if (here_compiler_file == NULL) {
                    perror(HERE_COMPILER_FILE);
                    exit(EXIT_FAILURE);
                }
                /* log_debug("opened %s for w\n", HERE_COMPILER_FILE); */
            } else {
                log_error("fopen %s: %s",
                          HERE_COMPILER_FILE,
                          strerror(errno));
                fprintf(stderr, "fopen %s: %s",
                        HERE_COMPILER_FILE,
                        strerror(errno));
                exit(EXIT_FAILURE);
            }
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
    fprintf(here_compiler_file, "%s\n",
            compiler_variants ?compiler_variants :compiler_version);
    fclose(here_compiler_file);
    chmod(HERE_COMPILER_FILE, S_IRUSR|S_IRGRP|S_IROTH);
    if (debug)
        log_debug("wrote " HERE_COMPILER_FILE);
    if (opam_cmds_verbose)
        printf("Wrote compiler version %s to %s\n",
               compiler_variants ?compiler_variants :compiler_version,
               HERE_COMPILER_FILE);
}

char *read_here_compiler_file(void)
{
#if defined(DEBUG_TRACE)
    log_trace("read_here_compiler_file");
#endif
    /* char *here_compiler_file = HERE_COSWITCH_ROOT HERE_COMPILER; */
    /* printf("reading %s\n", here_compiler_file); */
    if (access(HERE_COMPILER_FILE, R_OK) == 0) {
        char here_version[512];
        FILE *f = fopen(HERE_COMPILER_FILE, "r");
        fgets(here_version, 512, f);
        /* printf("String read: '%s'\n", here_version); */
        int len = strnlen(here_version, 512);
        if (here_version[len-1] == '\n') { // remove newline
            here_version[len - 1] = '\0';
        }
        fclose(f);
#if defined(DEBUG_TRACE)
        log_debug("found %s specifying version: %s",
                  HERE_COMPILER_FILE, here_version);
#endif
        return strndup(here_version, len);
    } else {
#if defined(DEBUG_TRACE)
        log_trace(RED HERE_COMPILER_FILE CRESET " not found.");
#endif
        return NULL;
    }
}

void write_here_coswitch_file(void)
{
    printf("write_here_coswitch_file\n");

    UT_string *coswitch_file;
    utstring_new(coswitch_file);
    utstring_printf(coswitch_file, "%s/COSWITCH.bzl",
                    getcwd(NULL, 0));
    printf("coswitch_here: %s\n", utstring_body(coswitch_file));

    errno = 0;
    FILE *here_coswitch_FILE = fopen(utstring_body(coswitch_file), "w");
    if (here_coswitch_FILE == NULL) { /* fail */
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

    /* UT_string *cmd; */
    /* utstring_new(cmd); */
    /* utstring_printf(cmd, */
    /*                 "opam var ocaml:version --root .opam --switch here"); */
    /* char *compiler_version = run_cmd(utstring_body(cmd)); */

    char *compiler_version = get_compiler_version(NULL);
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

    printf("xxxx7\n");
    char *compiler_variants = get_compiler_variants(NULL);

    fprintf(here_coswitch_FILE, "# generated file - DO NOT EDIT\n");
    fprintf(here_coswitch_FILE, "# here\n");

    fprintf(here_coswitch_FILE, "#   compiler version: %s\n",
            compiler_version);
    if (compiler_variants != NULL) {
        fprintf(here_coswitch_FILE, "#   ocaml-variants:   %s\n\n",
                compiler_variants);
    } else {
        fprintf(here_coswitch_FILE, "\n\n");
    }

    fprintf(here_coswitch_FILE, "def register():\n");
    fprintf(here_coswitch_FILE, "    native.new_local_repository(\n");
    fprintf(here_coswitch_FILE, "    name       = \"coswitch\",\n");
    fprintf(here_coswitch_FILE, "    path       = \".obazl.d/opam/here\",\n");
    fprintf(here_coswitch_FILE, "    build_file_content = \"#\"\n");
    fprintf(here_coswitch_FILE, "    )");

    fclose(here_coswitch_FILE);
    chmod(utstring_body(coswitch_file), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (debug)
        log_debug("wrote %s", coswitch_file);
    utstring_free(coswitch_file);
}

EXPORT void opam_here_opam_clone(char *_package)
{
    log_debug("opam_install %s", _package);

    char *exe;
    int result;

    if (access(HERE_OPAM_ROOT, F_OK) != 0) {
        //FIXME: print error msg
        log_error("OPAM root '" HERE_OPAM_ROOT "' not found.");
        printf("Project-local OPAM root '" HERE_OPAM_ROOT "' not found.\n");
        exit(EXIT_FAILURE);
    } else {

        exe = "opam";
        char *argv[] = {
            "opam", "install",
            /* "-v", */
            "--yes",
            "--cli=2.1",
            "--root=./" HERE_OPAM_ROOT,
            "--switch", HERE_SWITCH_NAME,
            "--require-checksums",
            _package,
            NULL
        };

        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        if (verbose)
            printf("Installing OPAM package: %s\n", _package);
        result = spawn_cmd(exe, argc, argv);
        if (result != 0) {
            fprintf(stderr, "FAIL: spawn_cmd(opam install --root .opam --switch _here --require-checksums %s)\n", _package);
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("install result: %s\n", result); */
        }
    }
}

void opam_here_clean(void) {
    /* FIXME: check COSWITCH.bzl to see if here switch is active  */
    if (verbose)
        printf("cleaning here coswitch\n");
    char *argv[] = {
        "-d", "-R", "-f",
        opam_cmds_verbose? "-v" : "",
        HERE_COSWITCH_ROOT,
        NULL
    };
    if(debug) {
        int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
        UT_string *cmd_str;
        utstring_new(cmd_str);
        printf("args ct: %d\n", argc);
        for (int i=0; i < argc; i++) {
            utstring_printf(cmd_str, "%s ", (char*)argv[i]);
        }
        fprintf(stderr, "cmd: %s\n", utstring_body(cmd_str));
        fflush(stderr);
        utstring_free(cmd_str);
    }
    execvp("rm", argv);
}

void opam_here_expunge(void) {
    /* FIXME: check COSWITCH.bzl to see if here switch is active  */
    /* if it is, expunging will make builds fail */
    /* ask user for confirmation */
    if (verbose)
        printf("expunging here switch and coswitch\n");
    run_cmd("rm -rf " HERE_OPAM_ROOT, true);
    run_cmd("rm -rf " HERE_COSWITCH_ROOT, true);
}

void _obazl_here_coswitch_list(void)
{
    printf("obazl_here_coswitch_list\n");
    log_info("obazl_here_coswitch_list");

    printf("\n\t\t" UCYN "@opam//here:list" CRESET "\n\n");
    printf(UWHT "Coswitch: here" CRESET "\n");
    printf("root:\t./%s\n", HERE_OBAZL_OPAM_WSS_OCAML);
    printf("\n");

    /* coswitch workspaces */

    /* @ocaml listing */
    printf(UWHT "Coswitch workspaces:" CRESET "\n");
    printf(CYN "@ocaml" CRESET "\n");
    if (access(HERE_OBAZL_OPAM_WSS_OCAML, F_OK) != 0) {
        log_info("Project-local obazl root '"
                 HERE_OBAZL_OPAM_WSS_OCAML
                 "' not found.\n");
        fprintf(stdout, "Project-local obazl opam root '"
                HERE_OBAZL_OPAM_WSS_OCAML
                "' not found.\n");
    } else {

        DIR *d = opendir(HERE_OBAZL_OPAM_WSS_OCAML);
        if (d == NULL) {
            fprintf(stderr, "coswitch list: unable to opendir "
                    HERE_OBAZL_OPAM_WSS_OCAML "\n");
            return;
        }

        display_direntries(d);
        closedir(d);

        /* fprintf(stdout, "toolchain: " */
        /*         HERE_OBAZL_OPAM_WSS_OCAML */
        /*         "\n"); */
        printf(CRESET "\n");

        printf("Package workspaces:\n");
        //FIXME: sort
        d = opendir(HERE_OBAZL_OPAM_WSS_PKGS);
        if (d == NULL) {
            fprintf(stderr, "coswitch list: unable to opendir "
                    HERE_OBAZL_OPAM_WSS_PKGS "\n");
            return;
        }
        display_pkg_direntries(d);
        closedir(d);
        printf("\n");

        // stublibs
        // opam pkgs
        printf("\n");
    }
}

//FIXME: rename obazl_here_list or coswitch_here_list
EXPORT void opam_here_list(void)
{
    printf("opam_here_list\n");
    _obazl_here_coswitch_list();
    opam_here_switch_list();
}

/* ********************* */
/* LOCAL int _opam_init(void) */
/* { */
/*     char *exe; */
/*     int result; */

/*     if (verbose) { */
/*         printf("initializing opam root at: .opam"); */
/*         log_info("initializing opam root at: .opam"); */
/*     } */

/*     exe = "opam"; */
/*     char *init_argv[] = { */
/*         "opam", "init", */
/*         "--verbose", */
/*         "--cli=2.1", */
/*         "--root=./.opam", */
/*         "--bare", */
/*         "--no-setup", "--no-opamrc", */
/*         "--bypass-checks", */
/*         "--yes", "--quiet", */
/*         /\* dry_run? "--dry-run" : NULL, *\/ */
/*         NULL */
/*     }; */

/*     int argc = (sizeof(init_argv) / sizeof(init_argv[0])) - 1; */
/*     result = spawn_cmd(exe, argc, init_argv); */
/*     if (result != 0) { */
/*         fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch _here)\n"); */
/*     } */
/*     return result; */
/* } */

/* /\* **************************************************************** *\/ */
/* LOCAL int _opam_create_switch(char *compiler_version) */
/* { */
/*     /\* char *ws_name = get_workspace_name(); *\/ */
/*     /\* UT_string *desc; *\/ */
/*     /\* utstring_new(desc); *\/ */
/*     /\* utstring_printf(desc, "'here-switch for workspace %s'", ws_name); *\/ */

/*     char *exe; */
/*     int result; */

/*     if (verbose) */
/*         log_info("creating switch with compiler version %s", compiler_version); */

/*     exe = "opam"; */
/*     char *switch_argv[] = { */
/*         "opam", "switch", */
/*         "--cli=2.1", */
/*         "--root=./.opam", */
/*         /\* "--description", utstring_body(desc), *\/ */
/*         "create", HERE_SWITCH_NAME, // "obazl", */
/*         compiler_version, */
/*         /\* dry_run? "--dry-run" : NULL, *\/ */
/*         NULL */
/*     }; */

/*     int argc = (sizeof(switch_argv) / sizeof(switch_argv[0])) - 1; */
/*     result = spawn_cmd(exe, argc, switch_argv); */
/*     if (result != 0) { */
/*         fprintf(stderr, "FAIL: run_cmd(opam switch --root .opam create _here)\n"); */
/*     } */

/*     if (!dry_run) { */
/*         FILE *here_compiler_file = fopen(HERE_COMPILER_FILE, "w"); */
/*         if (here_compiler_file == NULL) { */
/*             if (errno == EACCES) { */
/*                 int rc = unlink(HERE_COMPILER_FILE); */
/*                 if (rc == 0) { */
/*                     /\* log_debug("unlinked existing %s\n", HERE_COMPILER_FILE); *\/ */
/*                     here_compiler_file = fopen(HERE_COMPILER_FILE, "w"); */
/*                     if (here_compiler_file == NULL) { */
/*                         perror(HERE_COMPILER_FILE); */
/*                         exit(EXIT_FAILURE); */
/*                     } */
/*                     /\* log_debug("opened %s for w\n", HERE_COMPILER_FILE); *\/ */
/*                 } else { */
/*                     perror(HERE_COMPILER_FILE); */
/*                     exit(EXIT_FAILURE); */
/*                 } */
/*             } else { */
/*                 perror(HERE_COMPILER_FILE); */
/*                 exit(EXIT_FAILURE); */
/*             } */
/*         } */
/*         fprintf(here_compiler_file, "%s\n", compiler_version); */
/*         fclose(here_compiler_file); */
/*         chmod(HERE_COMPILER_FILE, S_IRUSR|S_IRGRP|S_IROTH); */
/*         if (debug) */
/*             log_debug("wrote " HERE_COMPILER_FILE); */
/*     } */
/*     return result; */
/* } */
