#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "utarray.h"
#if INTERFACE
#include "uthash.h"
#endif
#include "utstring.h"

#include "log.h"

/* #include "obazl.h" */
#include "opam_bootstrap.h"

char *host_repo = "ocaml";

FILE *log_fp;
const char *logfile = "bootstrapper.log";
char *CWD;
char log_buf[512];

/* **************************************************************** */
static int level = 0;
static int spfactor = 4;
static char *sp = " ";

static int indent = 2;
static int delta = 2;

/* **************************************************************** */

static int verbosity = 0;
int errnum;
int rc;

bool g_ppx_pkg;

char work_buf[PATH_MAX];

UT_array *opam_packages;

char coqlib[PATH_MAX];

struct buildfile_s {
    char *name;                 /* set from strndup optarg; must be freed */
    char *path;                 /* set from getenv; do not free */
    UT_hash_handle hh;
} ;
struct buildfile_s *buildfiles = NULL;

struct fileset_s *filesets = NULL;

/* struct package_s *packages = NULL; */

int strsort(const void *_a, const void *_b)
{
    const char *a = *(const char* const *)_a;
    const char *b = *(const char* const *)_b;
    /* printf("strsort: %s =? %s\n", a, b); */
    return strcmp(a,b);
}

char *run_cmd(char *cmd)
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

void opam_config(char *_opam_switch, char *bzlroot)
{
    log_info("opam_config bzlroot: %s", bzlroot);

    /* first discover current switch info */
    char *opam_switch;

    UT_string *switch_bin;
    utstring_new(switch_bin);

    UT_string *switch_lib;
    utstring_new(switch_lib);

    char *cmd, *result;
    if (_opam_switch == NULL) {
        /* log_info("using current switch"); */
        cmd = "opam var switch";

        result = run_cmd(cmd);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(%s)\n", cmd);
        } else
            opam_switch = strndup(result, PATH_MAX);
    } else {
        /* FIXME: handle non-NULL _opam_switch arg */
    }

    cmd = "opam var bin";
    result = NULL;
    result = run_cmd(cmd);
    if (result == NULL) {
        log_fatal("FAIL: run_cmd(%s)\n", cmd);
        exit(EXIT_FAILURE);
    } else
        utstring_printf(switch_bin, "%s", result);

    cmd = "opam var lib";
    result = NULL;
    result = run_cmd(cmd);
    if (result == NULL) {
        log_fatal("FAIL: run_cmd(%s)\n", cmd);
        exit(EXIT_FAILURE);
    } else
        utstring_printf(switch_lib, "%s", result);

    log_debug("switch_bin: %s", utstring_body(switch_bin));
    log_debug("switch_lib: %s", utstring_body(switch_lib));

    /* now link srcs */
    mkdir_r(bzlroot, "");       /* make sure bzlroot exists */
    UT_string *bzl_bin_link;
    utstring_new(bzl_bin_link);
    utstring_printf(bzl_bin_link, "%s/bin", bzlroot);

    UT_string *bzl_lib_link;
    utstring_new(bzl_lib_link);
    utstring_printf(bzl_lib_link, "%s/_lib", bzlroot);

    log_debug("bzl_bin_link: %s", utstring_body(bzl_bin_link));
    log_debug("bzl_lib_link: %s", utstring_body(bzl_lib_link));


    /* link to opam bin, lib dirs. we could do this in starlark, but
       then we would not be able to test independently. */
    rc = symlink(utstring_body(switch_bin), utstring_body(bzl_bin_link));
    if (rc != 0) {
        errnum = errno;
        if (errnum != EEXIST) {
            perror(utstring_body(bzl_bin_link));
            log_error("symlink failure for %s -> %s\n", utstring_body(switch_bin), utstring_body(bzl_bin_link));
            exit(EXIT_FAILURE);
        }
    }

    rc = symlink(utstring_body(switch_lib), utstring_body(bzl_lib_link));
    if (rc != 0) {
        errnum = errno;
        if (errnum != EEXIST) {
            perror(utstring_body(bzl_lib_link));
            log_error("symlink failure for %s -> %s\n", utstring_body(switch_lib), utstring_body(bzl_lib_link));
            exit(EXIT_FAILURE);
        }
    }

    /*  now set output paths (in @ocaml) */
    mkdir_r(bzlroot, "");       /* make sure bzlroot exists */
    UT_string *bzl_bin;
    utstring_new(bzl_bin);
    utstring_printf(bzl_bin, "%s/bin", bzlroot);

    UT_string *bzl_lib;
    utstring_new(bzl_lib);
    utstring_printf(bzl_lib, "%s/lib", bzlroot);

    log_debug("bzl_bin: %s", utstring_body(bzl_bin));
    log_debug("bzl_lib: %s", utstring_body(bzl_lib));

    // FIXME: always convert everything. otherwise we have to follow
    // the deps to make sure they are all converted.
    // (for dev/test, retain ability to do just one dir)
    if (utarray_len(opam_packages) == 0) {
        meta_walk(utstring_body(switch_lib),
                  bzlroot,
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
                                bzlroot,
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
    /* emit_bazel_config_setting_rules(bzlroot); */

    utarray_free(pos_flags);
    utarray_free(neg_flags);
    utstring_free(switch_bin);
    utstring_free(switch_lib);
    utstring_free(bzl_bin);
    utstring_free(bzl_lib);
}

/*
  special cases: digestif, threads
 */
int handle_lib_meta(char *switch_lib,
                    char *bzlroot,
                    /* char *_imports_path, */
                    char *pkgdir,
                    char *metafile)
{
    log_debug("================ HANDLE_LIB_META ================");
    log_debug("  switch_lib: %s; bzlroot: %s; pkgdir: %s; metafile: %s",
              switch_lib, bzlroot, pkgdir, metafile);

    char buf[PATH_MAX];
    buf[0] = '\0';
    mystrcat(buf, switch_lib);
    mystrcat(buf, "/");
    mystrcat(buf, pkgdir);
    mystrcat(buf, "/");
    mystrcat(buf, metafile);

    /* mkdir_r(buf, "/"); */
    /* mystrcat(buf, "/BUILD.bazel"); */

    /* /\* log_debug("out buf: %s", buf); *\/ */
    /* FILE *f; */
    /* if ((f = fopen(buf, "w")) == NULL){ */
    /*     log_fatal("Error! opening file %s", buf); */
    /*     exit(EXIT_FAILURE); */
    /* } */
    /* fprintf(f, "## test\n"); //  "src: %s/%s\n", pkgdir, metafile); */
    /* fclose(f); */

    errno = 0;
    log_debug("PARSING: %s", buf);
    struct obzl_meta_package *pkg = obzl_meta_parse_file(buf);
    if (pkg == NULL) {
        if (errno == -1)
            log_warn("Empty META file: %s", buf);
        else
            if (errno == -2)
                log_warn("META file contains only whitespace: %s", buf);
            else
                log_error("Error parsing %s", buf);
    } else {
        log_warn("PARSED %s", buf);
        /* dump_package(0, pkg); */

        stdlib_root = false;

        if (obzl_meta_entries_property(pkg->entries, "library_kind")) {
            /* special handling for ppx packages */
            log_debug("handling ppx package: %s", obzl_meta_package_name(pkg));
            g_ppx_pkg = true;
            /* emit_build_bazel_ppx(bzlroot, host_repo, "lib", "", pkg); */
        } else {
            log_debug("handling normal package: %s", obzl_meta_package_name(pkg));
            g_ppx_pkg = true;
        }

        /* skip threads pkg, obsolete */
        int len = strlen(buf);
        if (strncmp(buf + len - 12, "threads/META", 12) == 0) {
            log_warn("SKIPPING threads/META");
            return 0;
        }
        /* TMP HACK: skip some pkgs for which we use template BUILD files */
        if (strncmp(buf + len - 13, "digestif/META", 13) == 0) {
            log_warn("SKIPPING digestif/META");
            return 0;
        }
        if (strncmp(buf + len - 13, "ctypes/META", 11) == 0) {
            log_warn("SKIPPING ctypes/META");
            return 0;
        }
        if (strncmp(buf + len - 13, "ptime/META", 10) == 0) {
            log_warn("SKIPPING ptime/META");
            return 0;
        }
        UT_string *imports_path;
        utstring_new(imports_path);
        utstring_printf(imports_path, "_lib/%s",
                        obzl_meta_package_name(pkg));
        emit_build_bazel(host_repo,
                         bzlroot,      /* _repo_root: "." or "./tmp/opam" */
                         "lib",        /* _pkg_prefix */
                         utstring_body(imports_path),
                        /* "",      /\* pkg-path *\/ */
                         pkg);
        //, NULL);
/* obzl_meta_package_name(pkg)); */
    }
    return 0;
}

void log_fn(log_Event *evt)
{
    /* DO NOT USE! It seems to corrupt data somehow... */
    /* happens when we use basename on evt->file */

    /* typedef struct { */
    /*   va_list ap; */
    /*   const char *fmt; */
    /*   const char *file; */
    /*   struct tm *time; */
    /*   void *udata; */
    /*   int line; */
    /*   int level; */
    /* } log_Event; */

    static char fname_buffer[MAXPATHLEN];
    memset(log_buf, '0', 512);
    /* DO NOT use allocating basename, it corrupts something... */
    basename_r((char*)evt->file, fname_buffer);
    snprintf(log_buf, 512, "%d %s:%d ",
            evt->level, (char*)&fname_buffer, evt->line);
    fprintf(log_fp, "%s", log_buf);
    vsprintf(log_buf, evt->fmt, evt->ap);
    fprintf(log_fp, "%s\n", log_buf);
}

void _config_logging(void)
{
    CWD = getcwd(NULL, 0);
    log_fp = fopen(logfile, "w");
    if (log_fp == NULL) {
        perror(logfile);
        log_error("fopen fail on %s", logfile);
        fflush(stderr); fflush(stdout);
        exit(EXIT_FAILURE);
    }
    /* one or the other: */
    /* log_add_fp(log_fp, LOG_TRACE); */
    log_add_callback(log_fn, NULL, LOG_TRACE);

#ifdef DEBUG
    log_set_quiet(true);
    log_set_level(LOG_TRACE);
#else
    log_set_quiet(true);
    log_set_level(LOG_INFO);
#endif

}

int main(int argc, char *argv[]) // , char **envp)
{
#if defined(DEBUG)
    char *wd = getenv("BUILD_WORKING_DIRECTORY");
    fprintf(stdout, "\nBUILD_WORKING_DIRECTORY: %s\n", wd);
    wd = getenv("BUILD_WORKSPACE_DIRECTORY");
    fprintf(stdout, "BUILD_WORKSPACE_DIRECTORY: %s\n", wd);
    wd = getcwd(NULL, 0);
    fprintf(stdout, "CWD: %s\n", wd);
#endif

    _config_logging();

    char *opam_switch;

    CWD = getcwd(NULL, 0);

    utarray_new(opam_packages, &ut_str_icd);

    initialize_config_flags();

#ifdef DEBUG
    char *opts = "b:p:s:vhx";
#else
    char *opts = "b:p:s:vhx";
#endif

    char bzlroot[PATH_MAX];

    int opt;
    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
        case '?':
            /* log_debug("uknown opt: %c", optopt); */
            exit(EXIT_FAILURE);
            break;
        case ':':
            /* log_debug("uknown opt: %c", optopt); */
            exit(EXIT_FAILURE);
            break;
        case 'b':
            /* build_files */
            printf("option b: %s\n", optarg);
            printf("build file: %s\n", getenv(optarg));
            struct buildfile_s *the_buildfile = (struct buildfile_s *)calloc(sizeof (struct buildfile_s), 1);
            if (the_buildfile == NULL) {
                errnum = errno;
                fprintf(stderr, "main calloc failure for struct buildfile_s *\n");
                perror(getenv(optarg));
                exit(1);
            }
            the_buildfile->name = strndup(optarg, 512);
            the_buildfile->path = getenv(optarg);
            HASH_ADD_STR(buildfiles, name, the_buildfile);
            break;
        case 'p':
            printf("option p: %s\n", optarg);
            /* if pkg name has form foo.bar, the META file will be in foo */

            utarray_push_back(opam_packages, &optarg);
            break;
        case 's':
            printf("option s: %s\n", optarg);
            opam_switch = strndup(optarg, PATH_MAX);
            break;
        case 'x':
            verbosity++;
            break;
        case 'v': // --version
            fprintf(stdout, "0.1.0\n");
            exit(EXIT_SUCCESS);
        case 'h':
            log_info("Usage: bootstrap_opam[options]");
#ifdef DEBUG
            log_info("\toptions: b:o:p:s:vh");
#else
            log_info("\toptions: b:p:s:v");
#endif
            exit(EXIT_SUCCESS);
            break;
        default:
            ;
        }
    }
#ifdef DEBUG_TEST
    mystrcat(bzlroot, "./tmp/opam");
#else
    mystrcat(bzlroot, ".");
#endif

    fprintf(stdout, "BZLROOT: %s\n", bzlroot);

    opam_config(opam_switch, bzlroot);

    dispose_flag_table();

#ifdef DEBUG
    log_info("bzlroot: %s", bzlroot);
    log_info("FINISHED");
#endif

    fclose(log_fp);
    fprintf(stdout, "logfile: %s/%s\n", CWD, logfile);
}
