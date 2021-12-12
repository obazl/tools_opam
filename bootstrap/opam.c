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
#include "opam.h"

int errnum;
bool local_opam;

/* global: we write on new_local_repository rule per build file */
FILE *repo_rules_FILE;

#if INTERFACE
#define OBAZL_OPAM_ROOT ".obazl.d/opam"
#endif

char *obazl_opam_root = OBAZL_OPAM_ROOT;

UT_array *opam_packages;

char *run_cmdx(char *cmd)
{
    static char buf[PATH_MAX];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return NULL;
    }

    while (fgets(buf, sizeof buf, fp) != NULL) {
        /* printf("SWITCH: %s\n", buf); */
        buf[strcspn(buf, "\n")] = 0;
    }

    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
        return NULL;
    }
    return buf;
}

/*
  FIXME: run this on a background thread. Some (many) opam commands
  take a long time. loop in foreground thread printing '.' every half
  second, to give user liveness feedback.
 */
char * run_cmd(char *executable, char **argv)
{
    /* log_debug("run_cmd %s", argv[0]); */

    pid_t pid;
    /* char *argv[] = { */
    /*     "codept", */
    /*     "-args", codept_args_file, */
    /*     NULL}; */
    int rc;

    extern char **environ;

    /* FIXME: write stderr to log instead of dev/null? */
    int DEVNULL_FILENO = open("/dev/null", O_WRONLY);

    int cout_pipe[2];
    int cerr_pipe[2];

    if(pipe(cout_pipe) || pipe(cerr_pipe)) {
        log_error("pipe returned an error.");
        exit(EXIT_FAILURE);
    }

    posix_spawn_file_actions_t action;
    posix_spawn_file_actions_init(&action);

    /* child inherits open FDs, so: */
    /* close read end of pipes on child */
    posix_spawn_file_actions_addclose(&action, cout_pipe[0]);
    posix_spawn_file_actions_addclose(&action, cerr_pipe[0]);

    /* dup write-ends on child-side, connect stdout/stderr */
    posix_spawn_file_actions_adddup2(&action, cout_pipe[1],
                                     STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&action, cerr_pipe[1],
                                     STDERR_FILENO);

    /* close write end on child side */
    posix_spawn_file_actions_addclose(&action, cout_pipe[1]);
    posix_spawn_file_actions_addclose(&action, cerr_pipe[1]);

    /* now child will not inherit open pipes, but its stdout/stderr
       FDs will be connected to the write ends of the pipe.
     */

    // FIXME: restrict environ

    /* log_debug("spawning %s", executable); */
    rc = posix_spawnp(&pid, executable, &action, NULL, argv, environ);

    if (rc != 0) {
        /* does not set errno */
        log_fatal("run_command posix_spawn error rc: %d, %s",
                  rc, strerror(rc));
        exit(EXIT_FAILURE);
    }

    /* now close the write end on parent side */
    close(cout_pipe[1]);
    close(cerr_pipe[1]);

    /* https://github.com/pixley/InvestigativeProgramming/blob/114b698339fb0243f50cf5bfbe5d5a701733a125/test_spawn_pipe.cpp */

    // Read from pipes
    static char stdout_buffer[BUFSZ] = "";
    memset(stdout_buffer, '\0', BUFSZ);

    static char stderr_buffer[BUFSZ] = "";
    memset(stderr_buffer, '\0', BUFSZ);

    struct timespec timeout = {10, 0};

    fd_set read_set;
    memset(&read_set, 0, sizeof(read_set));
    FD_SET(cout_pipe[0], &read_set);
    FD_SET(cerr_pipe[0], &read_set);

    int larger_fd = (cout_pipe[0] > cerr_pipe[0])
        ? cout_pipe[0]
        : cerr_pipe[0];

    rc = pselect(larger_fd + 1, &read_set, NULL, NULL, &timeout, NULL);
    //thread blocks until either packet is received or the timeout goes through
    if (rc == 0) {
        fprintf(stderr, "pselect timed out.\n");
        /* return 1; */
        exit(EXIT_FAILURE);
    }

    int bytes_read = read(cerr_pipe[0], &stderr_buffer[0], BUFSZ);
    if (bytes_read > 0) {
        fprintf(stderr, "cerr_pipe readed message: %s\n", stderr_buffer);
        fprintf(stderr, "readed %d stderr bytes\n", bytes_read);
    } else {
        fprintf(stderr, "no stderr bytes readed\n");
    }

    /* fix me: read all the bytes */
    bytes_read = read(cout_pipe[0], &stdout_buffer[0], BUFSZ);
    if (bytes_read > 0){
        fprintf(stderr, "readed %d stdout bytes\n", bytes_read);
        stdout_buffer[bytes_read] = '\0';
        /* fprintf(stdout, "%s\n", */
        /*         //bytes_read, */
        /*         stdout_buffer); */
    } else {
        fprintf(stdout, "no stdout bytes readed\n");
    }

    waitpid(pid, &rc, 0);
    if (rc) {
        log_error("run_command rc: %d", rc);
        posix_spawn_file_actions_destroy(&action);
        /* exit(EXIT_FAILURE); */
    }

    /* fprintf(stdout,  "exit code: %d\n", rc); */

    posix_spawn_file_actions_destroy(&action);
    return stdout_buffer;
}

void init_opam_resolver_raw()
{
    /* we're going to write out a scheme file that our dune conversion
       routines can use to resolve opam pkg names to obazl target
       labels. */

    char * raw_resolver_file = ".obazl.d/opam/opam_resolver_raw.scm";
    /* if (access(raw_resolver_file, R_OK) != 0) return; */

    //FIXME: config the correct outfile name
    extern FILE *opam_resolver;  /* in emit_build_bazel.c */
        opam_resolver = fopen(raw_resolver_file, "w");
    if (opam_resolver == NULL) {
        perror("opam_resolver fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(opam_resolver, ";; CAVEAT: may contain duplicate entries\n");

    /* fprintf(opam_resolver, "("); */

    /* write predefined opam pkg mappings */
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs", "@ocaml//compiler-libs");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.common", "@ocaml//compiler-libs/common");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.bytecomp", "@ocaml//compiler-libs/bytecomp");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.optcomp", "@ocaml//compiler-libs/optcomp");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.toplevel", "@ocaml//compiler-libs/toplevel");
    fprintf(opam_resolver, "(%s . %s)\n",
            "compiler-libs.native-toplevel",
            "@ocaml//compiler-libs/native-toplevel");

    fprintf(opam_resolver, "(%s . %s)\n",
            "bigarray", "@ocaml//bigarray");
    fprintf(opam_resolver, "(%s . %s)\n",
            "dynlink", "@ocaml//dynlink");
    fprintf(opam_resolver, "(%s . %s)\n",
            "str", "@ocaml//str");
    fprintf(opam_resolver, "(%s . %s)\n",
            "unix", "@ocaml//unix");

    fprintf(opam_resolver, "(%s . %s)\n",
            "threads.posix", "@ocaml//threads");
    fprintf(opam_resolver, "(%s . %s)\n",
            "threads.vm", "@ocaml//threads");
}

char *get_workspace_name()
{
    char *exe, *result;

    exe = "bazel";
    char *ws_argv[] = {
        "bazel", "info",
        "execution_root",
        NULL
    };

    result = run_cmd(exe, ws_argv);
    if (result == NULL) {
        fprintf(stderr, "FAIL: run_cmd(bazel info execution_root)\n");
        exit(EXIT_FAILURE);
    } else {
        /* printf("BAZEL EXECUTION ROOT %s\n", result); */
        char *wsname = basename(result);
        printf("wsname '%s'\n", wsname);
        int n = strlen(wsname);
        wsname[n-1] = '\0'; // remove trailing newline
        printf("wsname '%s'\n", wsname);
        return wsname;
    }
}

/* opam_export */
EXPORT void opam_export(char *manifest)
{
    log_debug("opam_export: %s", manifest);

    UT_string *manifest_name;
    utstring_new(manifest_name);
    if (manifest) {
        utstring_printf(manifest_name, "%s", manifest);
    } else {
        char *workspace = get_workspace_name();
        printf("wsn: '%s'\n", workspace);
        utstring_printf(manifest_name, "%s.opam.manifest", workspace);
    }

    char *exe, *xresult = NULL;

    if (access(".opam", F_OK) != 0) {
        //FIXME: print error msg
        log_error("Project-local OPAM root '.opam' not found.");
        printf("Project-local OPAM root '.opam' not found.\n");
        exit(EXIT_FAILURE);
    } else {
        exe = "opam";
        char *export_argv[] = {
            "opam", "switch",
            "export",
            "--cli=2.1",
            "--root=./.opam",
            "--switch", "obazl",
            "--freeze", "--full",
            utstring_body(manifest_name),
            NULL
        };
        utstring_body(manifest_name);
        printf("Exporting %s\n", utstring_body(manifest_name));
        xresult = run_cmd(exe, export_argv);
        if (xresult == NULL) {
            fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch obazl)\n");
            exit(EXIT_FAILURE);
        /* } else { */
        /*     printf("export result: %s\n", xresult); */
        }
    }
}

/* opam_import */
EXPORT void opam_import(char *manifest)
{
    log_debug("opam_import: %s", manifest);
    printf("opam_import: %s\n", manifest);

    UT_string *manifest_name;
    utstring_new(manifest_name);
    if (manifest) {
        utstring_printf(manifest_name, "%s", manifest);
    } else {
        char *workspace = get_workspace_name();
        printf("wsn: '%s'\n", workspace);
        utstring_printf(manifest_name, "%s.opam.manifest", workspace);
    }

    char *exe, *result = NULL;

    if (access(".opam", F_OK) != 0) {
        //FIXME: print error msg
        log_error("Project-local OPAM root '.opam' not found.");
        printf("Project-local OPAM root '.opam' not found.\n");
        exit(EXIT_FAILURE);
    } else {
        exe = "opam";
        char *import_argv[] = {
            "opam", "switch",
            "--cli=2.1",
            "--root=./.opam",
            "--switch", "obazl",
            "--yes",
            "import",
            utstring_body(manifest_name),
            NULL
        };
        utstring_body(manifest_name);
        printf("Importing %s\n", utstring_body(manifest_name));

        result = run_cmd(exe, import_argv);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(opam import --root .opam --switch obazl %s)\n", 8);
            exit(EXIT_FAILURE);
        } else {
            printf("export result: %s\n", result);
        }
    }
}

/* opam_init_project_switch
   init a new opam installation rooted at ./opam, switch 'obazl',
   and install pkgs
 */
EXPORT void opam_init_project_switch(bool force, char *_opam_switch)
{
    log_debug("opam_init_project_switch");
    log_debug("  force: %d, switch: %s", force, _opam_switch);

    UT_string *opam_switch;
    utstring_new(opam_switch);
    if (_opam_switch == NULL) {
        printf("_opam_switch is NULL\n");
        if (access(".obazl.d/opam.switch", R_OK) == 0) {
            printf("using .obazl.d/opam.switch\n");
            char buff[512];
            FILE *f = fopen(".obazl.d/opam.switch", "r");
            fgets(buff, 512, f);
            printf("String read: '%s'\n", buff);
            int len = strnlen(buff, 512);
            if (buff[len-1] == '\n') { // remove newline
                buff[len - 1] = '\0';
            }
            utstring_printf(opam_switch, "%s", buff);
            fclose(f);
        } else {
            printf(".obazl.d/opam.switch not found\n");
            exit(EXIT_FAILURE);
        }
    } else {
        utstring_printf(opam_switch, "%s", _opam_switch);
    }
    printf("opam_switch: '%s'\n", utstring_body(opam_switch));

    char *exe, *result;

    if (access(".opam", F_OK) == 0) {

        log_info("Project-local OPAM root '.opam' already initialized.");
        printf("Project-local OPAM root '.opam' already initialized.\n");
        if (force) {
            printf("reinitialization not yet implemented\n");
            /* /\* FIXME use fts API? *\/ */
            /* exe = "rm"; */
            /* char *rm_argv[] = { */
            /*     "rm", "-vr", */
            /*     ".opam", */
            /*     NULL */
            /* }; */

            /* result = run_cmd(exe, rm_argv); */
            /* if (result == NULL) { */
            /*     fprintf(stderr, "FAIL: run_cmd(rm -r .opam)\n"); */
            /*     exit(EXIT_FAILURE); */
            /* } else { */
            /*     printf("rm result: %s\n", result); */
            /* } */
        } else {
            utstring_free(opam_switch);
            exit(EXIT_SUCCESS);
        }
    } else {

        exe = "opam";
        char *init_argv[] = {
            "opam", "init",
            "--cli=2.1",
            "--root=./.opam",
            "--bare",
            "--no-setup", "--no-opamrc",
            "--bypass-checks",
            "--yes", "--quiet",
            NULL
        };

        if (verbose) {
            printf("OPAM initialization: ");
            int len = (sizeof(init_argv) / sizeof(char*)) - 1;
            for (int i =0; i < len; i++) {
                printf("%s ", init_argv[i]);
            }
            printf("\n");
        }
        result = run_cmd(exe, init_argv);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch obazl)\n");
            utstring_free(opam_switch);
            exit(EXIT_FAILURE);
        } else {
            printf("%s\n", result);
        }

        char *switch_argv[] = {
            "opam", "switch",
            "--cli=2.1",
            "--root=./.opam",
            "create", "obazl",
            utstring_body(opam_switch),
            NULL
        };

        if (verbose) {
            printf("OPAM switch creation: ");
            int len = (sizeof(switch_argv) / sizeof(char*)) - 1;
            for (int i =0; i < len ; i++) {
                printf("%s ", switch_argv[i]);
            }
            printf("\n");
        }

        result = run_cmd(exe, switch_argv);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(opam switch --root .opam create obazl)\n");
            utstring_free(opam_switch);
            exit(EXIT_FAILURE);
        } else {
            printf("RESULT: %s\n", result);
        }
        utstring_free(opam_switch);

    }
}

EXPORT void opam_ingest(char *_opam_switch, char *obazl_opam_root)
{
    log_info("opam_ingest obazl_opam_root: %s", obazl_opam_root);

    utarray_new(opam_packages, &ut_str_icd);

    init_opam_resolver_raw();
    /* _initialize_skipped_pkg_list(); */

    /* we're going to write one 'new_local_repository' target per
       build file: */
    UT_string *repo_rules_filename = NULL;
    utstring_new(repo_rules_filename);
    utstring_printf(repo_rules_filename, "%s/opam_repos.bzl", obazl_opam_root);
    log_debug("repo_rules_filename: %s",
              utstring_body(repo_rules_filename));

    repo_rules_FILE = fopen(utstring_body(repo_rules_filename), "w");
    if (repo_rules_FILE == NULL) {
        perror(utstring_body(repo_rules_filename));
        exit(EXIT_FAILURE);
    }
    utstring_free(repo_rules_filename);
    fprintf(repo_rules_FILE, "load(\"@obazl_rules_ocaml//ocaml/_repo_rules:new_local_pkg_repository.bzl\",\n");
    fprintf(repo_rules_FILE, "     \"new_local_pkg_repository\")\n");
    fprintf(repo_rules_FILE, "\n");
    fprintf(repo_rules_FILE, "def fetch():\n");

    /* first discover current switch info */
    char *opam_switch;

    UT_string *switch_bin;
    utstring_new(switch_bin);

    UT_string *switch_lib;
    utstring_new(switch_lib);

    char *cmd, *result;
    /* if (_opam_switch == NULL) { */
    if (local_opam) {
        log_info("using local opam");
        opam_switch = "obazl"; // strndup(_opam_switch, PATH_MAX);
        utstring_printf(switch_bin, "./.opam/%s/bin", opam_switch);
        utstring_printf(switch_lib, "./.opam/%s/lib", opam_switch);

    } else {
        log_info("using current switch of sys opam");
        cmd = "opam var switch";

        result = run_cmdx(cmd);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmdx(%s)\n", cmd);
        } else
            opam_switch = strndup(result, PATH_MAX);

        cmd = "opam var bin";
        result = NULL;
        result = run_cmdx(cmd);
        if (result == NULL) {
            log_fatal("FAIL: run_cmdx(%s)\n", cmd);
            exit(EXIT_FAILURE);
        } else
            utstring_printf(switch_bin, "%s", result);

        cmd = "opam var lib";
        result = NULL;
        result = run_cmdx(cmd);
        if (result == NULL) {
            log_fatal("FAIL: run_cmdx(%s)\n", cmd);
            exit(EXIT_FAILURE);
        } else
            utstring_printf(switch_lib, "%s", result);

    }

    log_debug("switch: %s", opam_switch);
    log_debug("switch_bin: %s", utstring_body(switch_bin));
    log_debug("switch_lib: %s", utstring_body(switch_lib));

    /* now link srcs */
    mkdir_r(obazl_opam_root);        /* make sure obazl_opam_root exists */
    UT_string *bzl_bin_link;
    utstring_new(bzl_bin_link);
    utstring_printf(bzl_bin_link, "%s/bin", obazl_opam_root);

    UT_string *bzl_lib_link;
    utstring_new(bzl_lib_link);
    utstring_printf(bzl_lib_link, "%s/_lib", obazl_opam_root);

    log_debug("bzl_bin_link: %s", utstring_body(bzl_bin_link));
    log_debug("bzl_lib_link: %s", utstring_body(bzl_lib_link));


    /* link to opam bin, lib dirs. we could do this in starlark, but
       then we would not be able to test independently. */
    /* rc = symlink(utstring_body(switch_bin), utstring_body(bzl_bin_link)); */
    /* if (rc != 0) { */
    /*     errnum = errno; */
    /*     if (errnum != EEXIST) { */
    /*         perror(utstring_body(bzl_bin_link)); */
    /*         log_error("symlink failure for %s -> %s\n", utstring_body(switch_bin), utstring_body(bzl_bin_link)); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /* } */

    /* rc = symlink(utstring_body(switch_lib), utstring_body(bzl_lib_link)); */
    /* if (rc != 0) { */
    /*     errnum = errno; */
    /*     if (errnum != EEXIST) { */
    /*         perror(utstring_body(bzl_lib_link)); */
    /*         log_error("symlink failure for %s -> %s\n", utstring_body(switch_lib), utstring_body(bzl_lib_link)); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /* } */

    /*  now set output paths (in @ocaml) */
    mkdir_r(obazl_opam_root); /* make sure obazl_opam_root exists */
    UT_string *bzl_bin;
    utstring_new(bzl_bin);
    utstring_printf(bzl_bin, "%s/bin", obazl_opam_root);

    UT_string *bzl_lib;
    utstring_new(bzl_lib);
    utstring_printf(bzl_lib, "%s/buildfiles", obazl_opam_root);

    log_debug("bzl_bin: %s", utstring_body(bzl_bin));
    log_debug("bzl_lib: %s", utstring_body(bzl_lib));

    // FIXME: always convert everything. otherwise we have to follow
    // the deps to make sure they are all converted.
    // (for dev/test, retain ability to do just one dir)
    if (utarray_len(opam_packages) == 0) {
        meta_walk(utstring_body(switch_lib),
                  obazl_opam_root,
                  false,      /* link files? */
                  // "META",     /* file_to_handle */
                  handle_lib_meta); /* callback */
    } else {
        /* WARNING: only works for top-level pkgs */
        log_debug("converting listed opam pkgs in %s",
                  utstring_body(switch_lib));
        UT_string *s;
        utstring_new(s);
        char **a_pkg = NULL;
        /* log_trace("%*spkgs:", indent, sp); */
        while ( (a_pkg=(char **)utarray_next(opam_packages, a_pkg))) {
            utstring_clear(s);
            utstring_concat(s, switch_lib);
            utstring_printf(s, "/%s/%s", *a_pkg, "META");
            /* log_debug("src root: %s", utstring_body(s)); */
            /* log_trace("%*s'%s'", delta+indent, sp, *a_pkg); */
            if ( ! access(utstring_body(s), R_OK) ) {
                /* log_debug("FOUND: %s", utstring_body(s)); */
                handle_lib_meta(utstring_body(switch_lib),
                                obazl_opam_root,
                                /* obzl_meta_package_name(pkg), */
                                *a_pkg,
                                "META");
            } else {
                log_fatal("NOT found: %s", utstring_body(s));
                exit(EXIT_FAILURE);
            }
        }
        utstring_free(s);
    }

#ifdef DEBUG_TRACE
    char **p;
    p = NULL;
    while ( (p=(char**)utarray_next(pos_flags, p))) {
        log_debug("pos_flag: %s", *p);
    }
    p = NULL;
    while ( (p=(char**)utarray_next(neg_flags, p))) {
        log_debug("neg_flag: %s", *p);
    }
#endif
    /* emit_bazel_config_setting_rules(obazl_opam_root); */

    utarray_free(pos_flags);
    utarray_free(neg_flags);
    utstring_free(switch_bin);
    utstring_free(switch_lib);
    utstring_free(bzl_bin);
    utstring_free(bzl_lib);

    /* fprintf(opam_resolver, ")\n"); */
    fclose(opam_resolver);

    fclose(repo_rules_FILE);

    /* _free_skipped_pkg_list(); */
}

/* opam_install -p <pkg>
   init a new opam installation rooted at ./opam, switch 'obazl',
   and install pkgs
 */
EXPORT void opam_install(char *_package)
{
    log_debug("opam_install %s", _package);

    char *exe, *result;

    if (access(".opam", F_OK) != 0) {
        //FIXME: print error msg
        log_error("Project-local OPAM root '.opam' not found.");
        printf("Project-local OPAM root '.opam' not found.\n");
        exit(EXIT_FAILURE);
    } else {

        exe = "opam";
        char *install_argv[] = {
            "opam", "install",
            "-v",
            "--yes",
            "--cli=2.1",
            "--root=./.opam",
            "--switch", "obazl",
            "--require-checksums",
            _package,
            NULL
        };

        printf("Installing package %s\n", _package);
        result = run_cmd(exe, install_argv);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(opam install --root .opam --switch obazl --require-checksums %s)\n", _package);
            exit(EXIT_FAILURE);
        } else {
            printf("install result: %s\n", result);
        }
    }
}

EXPORT void opam_remove(char *_package)
{
    log_debug("opam_remove: %s", _package);

    char *exe, *result;

    if (access(".opam", F_OK) != 0) {
        //FIXME: print error msg
        log_error("Project-local OPAM root '.opam' not found.");
        printf("Project-local OPAM root '.opam' not found.\n");
        exit(EXIT_FAILURE);
    } else {

        exe = "opam";
        char *remove_argv[] = {
            "opam", "remove",
            "-v",
            "--yes",
            "--cli=2.1",
            "--root=./.opam",
            "--switch", "obazl",
            _package,
            NULL
        };

        printf("Removing package %s\n", _package);
        result = run_cmd(exe, remove_argv);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(opam remove --root .opam --switch obazl %s)\n", _package);
            exit(EXIT_FAILURE);
        } else {
            printf("remove result: %s\n", result);
        }
    }
}

EXPORT void opam_status(void) // char *_opam_switch, char *obazl_opam_root)
{
    printf("@opam//status\n");
    printf("\troot:   .opam\n");
    printf("\tswitch: obazl\n");
    log_info("opam_status");

    if (access(".opam", F_OK) != 0) {
        log_info("Project-local OPAM root '.opam' not found.\n");
        printf("Project-local OPAM root '.opam' not found.\n");
    } else {

        char *exe, *result;

        exe = "opam";
        char *var_argv[] = {
            "opam", "var",
            "--root", ".opam",
            "--switch", "obazl",
            NULL // null-terminated array of ptrs to null-terminated strings
        };

        result = run_cmd(exe, var_argv);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(opam var --root .opam --switch obazl)\n");
            exit(EXIT_FAILURE);
        } else {
            printf("%s\n", result);
        }

        //*var_argv = NULL; //FIXME: otherwise run_cmd gets confused
        char *list_argv[] = {
            "opam", "list",
            "--root", ".opam",
            "--switch", "obazl",
            "--columns", "name,version",
            NULL
        };

        result = run_cmd(exe, list_argv);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(opam list --root .opam --switch obazl)\n");
            exit(EXIT_FAILURE);
        } else {
            printf("RESULT: %s\n", result);
        }

    }
}
