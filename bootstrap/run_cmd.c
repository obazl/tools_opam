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
#include "run_cmd.h"

int errnum;
/* bool local_opam; */

/* global: we write on new_local_repository rule per build file */
/* FILE *repo_rules_FILE; */

/* UT_array *opam_packages; */

/*
  use popen to run a cmd, returning output string, which must be freed
*/
char *run_cmd(char *cmd)
{
    char buf[PATH_MAX];
    FILE *fp;

    errno = 0;
    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return NULL;
    }

    errno = 0;
    while (fgets(buf, sizeof buf, fp) != NULL) {
        buf[strcspn(buf, "\n")] = 0;
    }

    /* pclose waits, returns rc of process */
    if(pclose(fp))  {
        perror(cmd);
        return NULL;
    }
    return strdup(buf);
}

/*
  use posix_spawn to run a command. since the commands are designed to
  be run by the user at a terminal, we do not need to capture output.
 */
int spawn_cmd(char *executable, int argc, char *argv[])
{
    /* always echo the command first */
    printf("obazl: ");
    for (int i =0; i < argc; i++) {
        printf("%s ", (char*)argv[i]);
    }
    printf("\n");

    if (dry_run) return 0;

    pid_t pid;
    int rc;

    extern char **environ;

    rc = posix_spawnp(&pid, executable, NULL, NULL, argv, environ);

    waitpid(pid, &rc, 0);

    return rc;
}
