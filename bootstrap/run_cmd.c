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

#if INTERFACE
#include "utstring.h"
#endif

#include "log.h"
#include "run_cmd.h"

extern int errnum;
/* bool local_opam; */

/* UT_array *opam_packages; */

/*
  use popen to run a cmd, returning output string, which must be freed
*/
char *run_cmd(char *cmd, bool mutator) /* FIXME: make sure clients free result */
{
#if defined(DEBUG_TRACE)
    log_debug("run_cmd: %s", cmd);
#endif
    if ((verbose) || dry_run)
        printf(YEL "EXEC: " CMDCLR "%s" CRESET "\n", cmd);

    if (dry_run && mutator) {
        printf("run_cmd dry run\n");
        return NULL;
    }

    char buf[256];
    memset(buf, '\0', 256);
    FILE *fp;

    errno = 0;
    if ((fp = popen(cmd, "r")) == NULL) {
        log_error("Error opening pipe for run_cmd!\n");
        printf("Error opening pipe for run_cmd!\n");
        return NULL;
    }

    errno = 0;
    while (fgets(buf, sizeof buf, fp) != NULL) {
        buf[strcspn(buf, "\n")] = 0;
    }

    /* pclose waits, returns rc of process */
    if(pclose(fp))  {
        fprintf(stderr, "ERROR: %s\n\tfor cmd: %s\n",
                strerror(errno), cmd);
        /* perror(cmd); */
        if (verbose || dry_run) {
            printf("    => ");
            printf(CMDCLR "NULL" CRESET "\n");
        }
        return NULL;
    }
#if defined(DEBUG_TRACE)
    log_trace("cmd stdout: '%s'", buf);
#endif
    if (verbose || dry_run) {
        printf("    => ");
        printf(CMDCLR "%s" CRESET "\n", buf);
    }
    return strdup(buf);
}
