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
#include <sys/types.h>
#include <sys/wait.h>
#else
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

bool clean = false;

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

UT_string *bzl_switch_pfx; // will be set to either xdg or .obazl.d locn

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


char *get_compiler_version(char *opam_switch) {
#if defined(DEBUG_TRACE)
    log_trace("get_compiler_version for switch: %s", opam_switch);
#endif

    UT_string *cmd;
    utstring_new(cmd);

    utstring_printf(cmd,
           "opam var ocaml:version %s --switch %s",
                    (opam_switch==(void*)NULL)? "--root .opam" : "",
                    (opam_switch==(void*)NULL)? "here" : opam_switch);

    char *compiler_version = run_cmd(utstring_body(cmd), false);
    return compiler_version;
    utstring_free(cmd);
}

/* static void dump_pipe(int filedes) */
/* { */
/*     log_trace("dump_pipe"); */
/*     ssize_t read_ct; */
/*     char buf[1]; */

/*     char xbuf[4096]; */
/*     read_ct = read(filedes, xbuf, 4096); */
/*     /\* log_trace("XXXX: %s", xbuf); *\/ */
/*     /\* log_debug("xbuf[1]: %x", xbuf[5]); *\/ */
/*     if (strncmp(xbuf, */
/*                 "\033[31m" // RED */
/*                 "[ERROR]" */
/*                 CRESET    // \033[0m */
/*                 " Variable ocaml-variants:version not found", */
/*                 41) == 0) { */
/*         log_debug("ocaml-variants:version NOT FOUND"); */
/*         return; */
/*     } */
/*     /\* return; *\/ */

/*     log_trace("pipe output:"); */
/*     for (;;) */
/*     { */
/*         read_ct = read(filedes, buf, sizeof(buf)); */
/*         /\* printf("read_ct: %d\n", read_ct); *\/ */
/*         if (read_ct > 0) */
/*         { */
/*             printf("%c", buf[0]); */
/*         } */
/*         else */
/*         { */
/*             break; */
/*         } */
/*     } */
/*     printf("\"\"\"\n"); */
/*     log_trace("pipe end"); */
/* } */

int handle_errors(char *buf) {
    /* for (int i=0; i<10; i++) */
    /*     printf("%#x ", buf[i]); */
    /* printf("\n"); */
    /* log_debug("buf[1]: %#10x", buf[0]); */
    if (strncmp(buf,
                "\033[31m" // RED
                "[ERROR]"
                CRESET    // \033[0m
                " Variable ocaml-variants:version not found",
                41) == 0) {
        if (verbose || dry_run) {
            printf("    => ");
            printf(RED "NOT FOUND" CRESET "\n");
        }
#if defined(DEBUG_TRACE)
        log_debug("ocaml-variants:version NOT FOUND");
#endif
        return -1;

        //FIXME: check for other error conditions
    }
    if (strncmp(buf,
                "[ERROR] Variable ocaml-variants:version not found",
                41) == 0) {
        if (verbose || dry_run) {
            printf("    => ");
            printf(RED "NOT FOUND" CRESET "\n");
        }
#if defined(DEBUG_TRACE)
        log_debug("ocaml-variants:version NOT FOUND");
#endif
        return -1;

        //FIXME: check for other error conditions
    }
    else {
        return 0;
    }
}

char *get_compiler_variants(char *opam_switch) {
#if defined(DEBUG_TRACE)
    log_trace("get_compiler_variants for switch: %s", opam_switch);
#endif

    char *exe = "opam";
    char *argv[] = {
        "opam", "var",
        "--cli=2.1",
        "--switch",
        (opam_switch==NULL)? "here" : opam_switch,
        "ocaml-variants:version",
        /* "--root=./" HERE_OPAM_ROOT, */
        /* "--switch", HERE_SWITCH_NAME, */
        /* "--yes", */
        /* "import", */
        /* utstring_body(manifest_name), */
        (opam_switch==NULL)? "--root" : NULL,
        (opam_switch==NULL)? ".opam" :NULL,
        NULL
    };
    /* utstring_body(manifest_name); */
    /* printf("Importing %s\n", utstring_body(manifest_name)); */

    int argc = (sizeof(argv) / sizeof(argv[0])) - 1;

    if (verbose || dry_run) {
        UT_string *cmd_str;
        utstring_new(cmd_str);
        for (int i =0; i < argc; i++) {
            utstring_printf(cmd_str, "%s ", (char*)argv[i]);
        }
        /* log_info("obazl:"); */
#if defined(DEBUG_TRACE)
        log_trace("cmd: %s", utstring_body(cmd_str));
#endif
        printf(YEL "EXEC: " CMDCLR);
        printf("%s ", utstring_body(cmd_str));
        printf(CRESET "\n");
        utstring_free(cmd_str);
    }

    /* if (dry_run) return NULL; */

    pid_t pid;
    int rc;

    char buf[4096];             /* for reading pipes */

    int stdout_pipe[2];
    int stderr_pipe[2];

    errno = 0;
    rc = pipe(stdout_pipe);
    if (rc != 0) {
        log_error("pipe(stdout_pipe) fail: %s", strerror(errno));
        fprintf(stdout, "pipe(stdout_pipe) fail: %s\n", strerror(errno));
    }

    errno = 0;
    rc = pipe(stderr_pipe);
    if (rc != 0) {
        log_error("pipe(stderr_pipe) fail: %s", strerror(errno));
        fprintf(stdout, "pipe(stderr_pipe) fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    extern char **environ;

    /* tell posix_spawn to redirect stdout/stderr */
    posix_spawn_file_actions_t action;
    posix_spawn_file_actions_init(&action);

    /* int DEVNULL_FILENO = open("/dev/null", O_WRONLY); */

    /* stdout > stdout_pipe[1] */
    if ((rc = posix_spawn_file_actions_adddup2(&action,
                                               stdout_pipe[1],
                                               STDOUT_FILENO))) {
        perror("posix_spawn_file_actions_adddup2 stdout > stdout_pipe[1]");
        posix_spawn_file_actions_destroy(&action);
        return NULL;
    }
    posix_spawn_file_actions_addclose(&action, stdout_pipe[0]);

    /* stderr > stderr_pipe[1] */
    if ((rc = posix_spawn_file_actions_adddup2(&action,
                                               stderr_pipe[1],
                                               STDERR_FILENO))) {
        perror("posix_spawn_file_actions_adddup2 stderr > stderr_pipe[1]");
        posix_spawn_file_actions_destroy(&action);
        return NULL;
    }
   posix_spawn_file_actions_addclose(&action, stderr_pipe[0]);

    errno = 0;
    rc = posix_spawnp(&pid, exe, &action, NULL, argv, environ);

    if (rc == 0) {
        /* log_debug("posix_spawn child pid: %i\n", pid); */
        int waitrc = waitpid(pid, &rc, WNOHANG | WUNTRACED);
        if (waitrc == 0) {
            // child exit OK
            if ( WIFEXITED(rc) ) {
                // terminated normally by a call to _exit(2) or exit(3).
                /* log_debug("WEXITSTATUS: %d", WEXITSTATUS(rc)); */
                /* "widow" the pipe (delivers EOF to reader)  */
                /* close(stdout_pipe[1]); */
                /* log_debug("stdout_pipe[0]:"); */
                /* dump_pipe(stdout_pipe[0]); */
                /* close(stdout_pipe[0]); */

                /* "widow" the pipe (delivers EOF to reader)  */
                close(stderr_pipe[1]);
                /* log_debug("stderr_pipe[0]:"); */
                /* dump_pipe(stderr_pipe[0]); */

                /* ssize_t read_ct = */
                read(stderr_pipe[0], buf, 4096);
                if (handle_errors(buf) == 0) {
                    /* printf("NO ERRORS\n"); */
                    close(stdout_pipe[1]);
                    /* log_debug("stdout_pipe[0]:"); */
                    /* dump_pipe(stdout_pipe[0]); */

                    /* ssize_t read_ct = */
                    read(stdout_pipe[0], buf, 4096);
                    if (verbose || dry_run) {
                        printf("    => ");
                        printf(CMDCLR "%s" CRESET "\n", buf);
                    }
                    /* close(stderr_pipe[0]); */
                    return strdup(buf); //FIXME: strndup
                } else {
                    /* fprintf(stderr, "ERROR\n"); */
                    return NULL;
                }
            }
            else if (WIFSIGNALED(rc)) {
                // terminated due to receipt of a signal
                log_error("spawn_cmd rc: %d", rc);
                /* log_trace("waitpid */
            } else if (WIFSTOPPED(rc)) {
                /* process has not terminated, but has stopped and can
                   be restarted. This macro can be true only if the
                   wait call specified the WUNTRACED option or if the
                   child process is being traced (see ptrace(2)). */
            }
        }
        else if (waitrc == -1) {
            perror("spawn_cmd waitpid error");
            log_error("spawn_cmd");
            posix_spawn_file_actions_destroy(&action);
            return NULL;
        }
        else {
            log_error("spawn_cmd: stopped or terminated child pid: %d",
                      waitrc);
            posix_spawn_file_actions_destroy(&action);
            return NULL;
        }
    } else {
        /* posix_spawnp rc != 0; does not set errno */
        log_fatal("spawn_cmd error rc: %d, %s", rc, strerror(rc));
        posix_spawn_file_actions_destroy(&action);
        return NULL;
    }
    //  should not reach here?
    log_error("BAD FALL_THROUGH");
    return NULL;
}
