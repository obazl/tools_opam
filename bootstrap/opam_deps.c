#include <assert.h>
#include <errno.h>
#include <fcntl.h>              /* open() */
#if INTERFACE
#include <fts.h>
#endif
#if INTERFACE
#ifdef LINUX
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#else
#include <limits.h>
#endif
#endif

#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "utarray.h"
#include "utstring.h"

#include "log.h"
#include "opam_deps.h"

LOCAL int run_codept(char *codept_args_file, /* input */
                     char *codept_deps_file) /* output */
{
    printf("running codept from %s\n", getcwd(NULL, 0));

    if (access(codept_args_file, R_OK) != 0) {
        printf("codept.args file inaccessible\n");
        return -1;
    }

    int rc;

    /* tell posix_spawn to redirect stdout/stderr */
    /* FIXME: write stderr to log instead of dev/null? */
    int DEVNULL_FILENO = open("/dev/null", O_WRONLY);
    posix_spawn_file_actions_t action;
    posix_spawn_file_actions_init(&action);

    /* stdout > codept_deps_file */
    if ((rc = posix_spawn_file_actions_addopen
         (&action, STDOUT_FILENO, codept_deps_file,
          O_WRONLY | O_CREAT | O_TRUNC,
          S_IRUSR | S_IWUSR | S_IRGRP ))) {
        perror("posix_spawn_file_actions_adddopen");
        posix_spawn_file_actions_destroy(&action);
        return rc;
    }
    /* fi */
    /* stderr > /dev/null */
    if ((rc = posix_spawn_file_actions_adddup2(&action,
                                              DEVNULL_FILENO,
                                               STDERR_FILENO))) {
        perror("posix_spawn_file_actions_adddup2");
        posix_spawn_file_actions_destroy(&action);
        return rc;
    }

    char *exe = "codept";
    char *argv[] = {
        "codept",
        "-sexp",
        "-k",
        "-args", codept_args_file,
        NULL};

    /* echo command first */
    int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
    printf("obazl: ");
    for (int i =0; i < argc; i++) {
        printf("%s ", (char*)argv[i]);
    }
    printf("\n");

    extern char **environ;
    pid_t pid;
    rc = posix_spawn(&pid, exe, &action, NULL, argv, environ);

    if (rc == 0) {
        /* log_debug("posix_spawn child pid: %i\n", pid); */
        if (waitpid(pid, &rc, 0) != -1) {
            if ( !WIFEXITED(rc) ) {
                // signaled or stopped
                log_error("codept rc: %d", rc);
            } else {
                // normal termination
            }
        } else {
            perror("codept waitpid");
            log_error("run_codept posix_spawn");
        }
        posix_spawn_file_actions_destroy(&action);
        return 0;
    } else {
        /* does not set errno */
        log_fatal("run_codept posix_spawn error rc: %d, %s", rc, strerror(rc));
        posix_spawn_file_actions_destroy(&action);
        return rc;
    }
}

UT_array  *segs;
UT_string *group_tag;

void emit_codept_group_tag(FILE *out, FTSENT *ftsentry)
{
    /* printf("emit_codept_group_tag: %d. %s\n", */
    /*        ftsentry->fts_level, ftsentry->fts_name); */
    FTSENT *e = ftsentry;

    char *seg;

    utarray_clear(segs);
    utstring_renew(group_tag);

    while (e->fts_parent) {
        /* printf("parent: '%s'\n", e->fts_parent->fts_name); */
        if ((e->fts_name[0] == '.') && e->fts_namelen == 1) {
            e = e->fts_parent;
        } else {
            if (e->fts_level == 1) {
                /* printf("level: 1\n"); */
                /* truncate at version string suffix */
                char *dot1 = strchr(e->fts_name, '.');
                if (dot1) {
                    int len = dot1 - e->fts_name;
                    seg = strndup(e->fts_name, len);
                    /* printf("pushing (dot): %s\n", s); */
                    utarray_push_back(segs, &seg);
                    free(seg);
                } else {
                    /* printf("pushing: %s\n", e->fts_name); */
                    seg = strndup(e->fts_name, e->fts_namelen);
                    utarray_push_back(segs, &seg);
                    free(seg);
                }
                e = e->fts_parent;
            } else {
                /* printf("pushing lev: %s\n", e->fts_name); */
                seg = strndup(e->fts_name, e->fts_namelen);
                utarray_push_back(segs, &seg);
                free(seg);
                /* printf("%s|", e->fts_name); */
                e = e->fts_parent;
            }
        }
    }

    //FIXME: only print this if children include src files
    // to avoid foo.bar[]
    // easiest: post-process args file to remove lines terminating []
    int ct = utarray_len(segs);
    if (ct > 0) {
        char **p = NULL;
        for (int i = ct-1; i >= 0 ; i--) {
            fprintf(out, "%s", *(char**)utarray_eltptr(segs, i));
            if (i > 0) fprintf(out, ".");
        }
        fprintf(out, "[");
    }
}

void emit_codept_group_elts(FILE *out, FTS *tree)
{
    /* printf("emit_codept_group_elts %d\n", FTS_F); */

    FTSENT *child = fts_children(tree, FTS_NAMEONLY);
    /* printf("first child: %s\n", child->fts_name); */

    bool started = false;

    while (child) {
        /* printf("ext %s:  '%s'", */
        /*        child->fts_name, */
        /*        child->fts_name */
        /*        + (child->fts_namelen-3)); */

        if (strncmp(".ml",
                    child->fts_name + (child->fts_namelen-3),
                    3) == 0) {
            if (started) {fprintf(out, ",");} else {started = true;}
            fprintf(out, "%s%s", child->fts_path, child->fts_name);
        }
        else if (strncmp(".mli",
                    child->fts_name + (child->fts_namelen-4),
                    4) == 0) {
            if (started) fprintf(out, ","); else started = true;
            fprintf(out, "%s%s", child->fts_path, child->fts_name);
        }
        child = child->fts_link;
    }
    fprintf(out, "]\n");
}

EXPORT void emit_codept_args(FILE *out, FTS *tree)
{
    printf("emit_codept_args\n");
    FTSENT *ftsentry     = NULL;
    FTSENT *rootentry = fts_read(tree);

    /* FTSENT *wss = fts_children(tree, FTS_NAMEONLY); */
    /* while (wss) { */
    /*     printf("ws: %s\n", wss->fts_name); */
    /*     wss = wss->fts_link; */
    /* } */

    utarray_new(segs, &ut_str_icd);

    char *dot;
    if (NULL != tree) {
        while( (ftsentry = fts_read(tree)) != NULL)
            {
                switch (ftsentry->fts_info)
                    {
                    /* case FTS_DP : // dir visited in post-order */
                    /*     break; */
                    case FTS_D : // dir visited in pre-order
                        if (ftsentry->fts_name[0] == '.') {
                            // do not process children of hidden dirs
                            fts_set(tree, ftsentry, FTS_SKIP);
                            break;
                        }
                        if ((ftsentry->fts_namelen == 5)
                            && (strncmp(ftsentry->fts_name,
                                    "tests",
                                     5) == 0)) {
                            fts_set(tree, ftsentry, FTS_SKIP);
                            break;
                        }
                        /* skip compiler sources */
                        /* skip dune, dune-configurator? */
                        /* if (strncmp(ftsentry->fts_name, */
                        /*             "ocaml-base-compiler.", */
                        /*             20) == 0) { */
                        /*     fts_set(tree, ftsentry, FTS_SKIP); */
                        /*     break; */
                        /* } */
                        /* else if (strncmp(ftsentry->fts_name, */
                        /*             "dune.", */
                        /*             5) == 0) { */
                        /*     fts_set(tree, ftsentry, FTS_SKIP); */
                        /*     break; */

                        /* } */
                        /* else if (strncmp(ftsentry->fts_name, */
                        /*             "dune-configurator.", */
                        /*             18) == 0) { */
                        /*     fts_set(tree, ftsentry, FTS_SKIP); */
                        /*     break; */

                        /* } else { */
                            emit_codept_group_tag(out, ftsentry);
                            emit_codept_group_elts(out, tree);
                        /* } */
                        break;
                    case FTS_F : // regular file
                        if (ftsentry->fts_name[0] == '.') {
                            // do not process hidden files
                            fts_set(tree, ftsentry, FTS_SKIP);
                            break;
                        }
                        dot = strrchr(ftsentry->fts_name, '.');
                        if (dot) {
                            if ((strncmp(dot, ".ml", 3) == 0)) {
                                /* && (strlen(dot) == 3)) { */
                                /* _indent(ftsentry->fts_level); */
                                /* printf("%d. %s", */
                                /*        ftsentry->fts_level, */
                                /*        ftsentry->fts_name); */
                                /* FTSENT *p = ftsentry->fts_parent; */
                                /* while (p) { */
                                /*     printf("\t%s ", p->fts_name); */
                                /*     p = p->fts_parent; */
                                /* } */
                                /* printf("\n"); */
                            }
                        }
                        else
                            // save if dune or opam
                            if ((ftsentry->fts_namelen = 4)
                                &&(strncmp(ftsentry->fts_name, "dune", 4) == 0)){
                                /* _indent(ftsentry->fts_level); */
                                /* printf("%d. %s\n", */
                                /*        ftsentry->fts_level, */
                                /*        ftsentry->fts_name); */
                        }
                        break;
                    case FTS_SL: // symlink
                        /* _indent(ftsentry->fts_level); */
                        /* printf("SYMLINK: %s\n", ftsentry->fts_name); */
                        break;
                    default:
                        break;
                    }
            }
    }

    utarray_free(segs);

    return;
}

EXPORT void opam_crawl(FTS *tree)
{
    FTSENT *ftsentry     = NULL;

    FTSENT *rootentry = fts_read(tree);

    char *dot;
    if (NULL != tree) {
        while( (ftsentry = fts_read(tree)) != NULL)
            {
                switch (ftsentry->fts_info)
                    {
                    case FTS_D : // dir visited in pre-order
                        _indent(ftsentry->fts_level);
                        if (ftsentry->fts_name[0] == '.') {
                            printf("Skip dir: %s\n", ftsentry->fts_name);
                            // do not process children of hidden dirs
                            fts_set(tree, ftsentry, FTS_SKIP);
                        } else
                            printf("%d. %s  (dir)\n",
                                   ftsentry->fts_level,
                                   ftsentry->fts_name);

                        break;
                    case FTS_F : // regular file
                        dot = strrchr(ftsentry->fts_name, '.');
                        if (dot) {
                            if ((strncmp(dot, ".ml", 3) == 0)) {
                                /* && (strlen(dot) == 3)) { */
                                _indent(ftsentry->fts_level);
                                printf("%d. %s",
                                       ftsentry->fts_level,
                                       ftsentry->fts_name);
                                FTSENT *p = ftsentry->fts_parent;
                                while (p) {
                                    printf("\t%s ", p->fts_name);
                                    p = p->fts_parent;
                                }
                                printf("\n");
                            }
                        }
                        else
                            // save if dune or opam
                            if ((ftsentry->fts_namelen = 4)
                                &&(strncmp(ftsentry->fts_name, "dune", 4) == 0)){
                                _indent(ftsentry->fts_level);
                                printf("%d. %s\n",
                                       ftsentry->fts_level,
                                       ftsentry->fts_name);
                        }
                        break;
                    case FTS_SL: // symlink
                        _indent(ftsentry->fts_level);
                        printf("%s\n", ftsentry->fts_name);
                        break;
                    default:
                        break;
                    }
            }
    }
    return;
}

void print_workspaces(FTS *tree)
{
    FTSENT *wss = fts_children(tree, FTS_NAMEONLY);
    while (wss) {
        printf("ws: %d. %s\t\t%s, %s\n",
               wss->fts_level,
               wss->fts_name,
               wss->fts_path,
               wss->fts_accpath);
        wss = wss->fts_link;
    }
    return;
}

void opam_deps(char *_root)
{
    printf("opam_deps\n");
    log_debug("opam_deps");
    printf("cwd: %s\n", getcwd(NULL, 0));

    FTS* tree = NULL;
    FTSENT *ftsentry     = NULL;

    char * root;

    if (_root == NULL) {
        if (access(OPAM_SRC_ROOT, F_OK) != 0) {
            printf("root %s not found\n", OPAM_SRC_ROOT);
            printf("cwd: %s\n", getcwd(NULL, 0));
            exit(EXIT_FAILURE);
        }
        chdir(OPAM_SRC_ROOT);
        root = ".";
        /* root = OPAM_SRC_ROOT; */
    } else {
        chdir(_root);
        root = ".";
    }

    tree = fts_open(&root,
                    FTS_COMFOLLOW | FTS_NOCHDIR,
                    // NULL
                    &compare
                    );

    /* FTSENT *rootentry = fts_read(tree); */

    /* print_workspaces(tree); */

    /* opam_crawl(tree); */

    FILE *out;
    out = fopen("codept.args", "w");
    emit_codept_args(out, tree);
    fclose(out);
    fts_close(tree);

    /* now run codept ?? */
    /* int rc = run_codept("codept.args", "codept.deps"); */

    char cmd[PATH_MAX];
    sprintf(cmd, "codept -verbosity info -sexp -k -args %s 1> %s 2> /dev/null",
            "codept.args", "codept.deps");
            /* codept_args_file); */
    char *res = run_cmd(cmd, true);
    printf("codept res: %s\n", res);

    return;
}

int compare(const FTSENT** one, const FTSENT** two)
{
    return (strcmp((*one)->fts_name, (*two)->fts_name));
}

void _indent(int i)
{
    for (; i > 0; i--)
        printf("    ");
}


