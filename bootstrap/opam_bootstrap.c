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

#include <sys/types.h>
#include <sys/stat.h>


#include "utarray.h"
#if INTERFACE
#include "uthash.h"
#endif
#include "utstring.h"

#include "log.h"

/* #include "obazl.h" */
#include "opam_bootstrap.h"

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

#if INTERFACE
struct config_flag {
    char name[32];              /* key */
    char repo[16];
    char package[64];
    char target[32];
    char label[64];
    UT_hash_handle hh;
};
#endif
struct config_flag *the_flag_table; /* FIXME: obsolete? */

UT_array *pos_flags;            /* string */
UT_array *neg_flags;            /* string */

char work_buf[PATH_MAX];

char outdir[PATH_MAX];
char tgtroot[PATH_MAX];
char tgtroot_bin[PATH_MAX];
char tgtroot_lib[PATH_MAX];


UT_array *opam_packages;

char basedir[PATH_MAX];
char coqlib[PATH_MAX];

char symlink_src[PATH_MAX];
char symlink_tgt[PATH_MAX];

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

/* **************************************************************** */
void initialize_config_flags()
{
    /* char name[32];              /\* key *\/ */
    /* char repo[16]; */
    /* char package[64]; */
    /* char target[32]; */
    /* char label[64]; */

    struct config_flag *a_flag;

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "byte", 4);
    strncpy(a_flag->repo, "@ocaml", 6);
    strncpy(a_flag->package, "mode", 4);
    strncpy(a_flag->target, "bytecode", 8);
    strncpy(a_flag->label, "@ocaml//mode:bytecode", 21);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "native", 6);
    strncpy(a_flag->repo, "@ocaml", 6);
    strncpy(a_flag->package, "mode", 4);
    strncpy(a_flag->target, "native", 6);
    strncpy(a_flag->label, "@ocaml//mode:native", 19);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "mt", 2);
    strncpy(a_flag->repo, "@opam", 5);
    strncpy(a_flag->package, "cfg/mt", 6);
    strncpy(a_flag->target, "default", 7);
    strncpy(a_flag->label, "@opam//cfg/mt:default", 21);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "mt_posix", 8);
    strncpy(a_flag->repo, "@opam", 5);
    strncpy(a_flag->package, "cfg/mt", 6);
    strncpy(a_flag->target, "posix", 5);
    strncpy(a_flag->label, "@opam//cfg/mt:posix", 19);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "mt_vm", 5);
    strncpy(a_flag->repo, "@opam", 5);
    strncpy(a_flag->package, "cfg/mt", 6);
    strncpy(a_flag->target, "vm", 2);
    strncpy(a_flag->label, "@opam//cfg/mt:vm", 16);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "gprof", 5);
    strncpy(a_flag->repo, "@opam", 5);
    strncpy(a_flag->package, "cfg", 3);
    strncpy(a_flag->target, "gprof", 5);
    strncpy(a_flag->label, "@opam//cfg:gprof", 16);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "ppx_driver", 10);
    strncpy(a_flag->repo, "@opam", 5);
    strncpy(a_flag->package, "cfg", 3);
    strncpy(a_flag->target, "driver", 6);
    strncpy(a_flag->label, "@opam//cfg:ppx_driver", 21);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    a_flag = calloc(sizeof(struct config_flag), 1);
    strncpy(a_flag->name, "custom_ppx", 10);
    strncpy(a_flag->repo, "@opam", 5);
    strncpy(a_flag->package, "cfg", 3);
    strncpy(a_flag->target, "custom", 6);
    strncpy(a_flag->label, "@opam//cfg:ppx_custom", 21);
    HASH_ADD_STR(the_flag_table, name, a_flag);

    /* these seem to be associated with camlp4; ignore for now: */
    /* toploop, create_toploop, preprocessor, syntax */
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

void opam_config(char *_opam_switch, char *outdir)
{
    log_info("opam_config outdir: %s", outdir);
    /*
      1. discover switch
         a. check env var OPAMSWITCH
         b. use -s option
         c. run 'opam var switch'
      2. discover lib dir: 'opam var lib'
     */

    char *opam_switch;
    /* char *srcroot_bin; */
    /* char *srcroot_lib; */
    UT_string *srcroot_bin;
    utstring_new(srcroot_bin);
    UT_string *srcroot_lib;
    utstring_new(srcroot_lib);

    char *tgt_bin = "bin";
    char *tgt_bin_hidden = "/_bin";
    char *tgt_lib = "lib";
    char *tgt_lib_hidden = "/_lib";

    char workbuf[PATH_MAX];

    tgtroot_bin[0] = '\0';
    mystrcat(tgtroot_bin, outdir);
    mystrcat(tgtroot_bin, "/bin");
    tgtroot_lib[0] = '\0';
    mystrcat(tgtroot_lib, outdir);
    mystrcat(tgtroot_lib, "/lib");

    /* FIXME: handle switch arg */
    char *cmd, *result;
    if (_opam_switch == NULL) {
        /* log_info("using current switch"); */
        cmd = "opam var switch";

        result = run_cmd(cmd);
        if (result == NULL) {
            fprintf(stderr, "FAIL: run_cmd(%s)\n", cmd);
        } else
            opam_switch = strndup(result, PATH_MAX);
    }

    cmd = "opam var bin";
    result = NULL;
    result = run_cmd(cmd);
    if (result == NULL) {
        log_fatal("FAIL: run_cmd(%s)\n", cmd);
        exit(EXIT_FAILURE);
    } else
        /* srcroot_bin = strndup(result, PATH_MAX); */
        utstring_printf(srcroot_bin, "%s", result);

    cmd = "opam var lib";
    result = NULL;
    result = run_cmd(cmd);
    if (result == NULL) {
        log_fatal("FAIL: run_cmd(%s)\n", cmd);
        exit(EXIT_FAILURE);
    } else
        /* srcroot_lib = strndup(result, PATH_MAX); */
        utstring_printf(srcroot_lib, "%s", result);

    // STEP 0: install root WORKSPACE, BUILD files

    // STEP 1: link opam bin, lib dirs
    // NOTE: root BUILD.bazel must contain 'exports_files([".bin/**"], ["_lib/**"])'

    // FIXME: these dir symlinks can be done in starlark code; which way is better?

    mkdir_r(outdir, "");
    workbuf[0] = '\0';
    mystrcat(workbuf, outdir);
    mystrcat(workbuf, tgt_bin_hidden);
    rc = symlink(utstring_body(srcroot_bin), workbuf); // tgtroot_bin);
    if (rc != 0) {
        errnum = errno;
        if (errnum != EEXIST) {
            perror(symlink_tgt);
            log_error("symlink failure for %s -> %s\n", utstring_body(srcroot_bin), tgtroot_bin);
            exit(EXIT_FAILURE);
        }
    }
    /* mkdir_r(tgtroot_lib, ""); */
    workbuf[0] = '\0';
    mystrcat(workbuf, outdir);
    mystrcat(workbuf, tgt_lib_hidden);
    rc = symlink(utstring_body(srcroot_lib), workbuf); // tgtroot_lib);
    if (rc != 0) {
        errnum = errno;
        if (errnum != EEXIST) {
            perror(symlink_tgt);
            log_error("symlink failure for %s -> %s\n", utstring_body(srcroot_lib), tgtroot_lib);
            exit(EXIT_FAILURE);
        }
    }

    /* always do bin */
    // FIXME: this has no effect - no BUILD.bazel pkgs in bin tree
    /* mirror_tree(utstring_body(srcroot_bin), */
    /*             /\* tgtroot_bin, *\/ */
    /*             false, NULL, NULL); */

    utarray_new(pos_flags, &ut_str_icd);
    utarray_new(neg_flags, &ut_str_icd);

    // FIXME: always convert everything. otherwise we have to follow
    // the deps to make sure they are all converted.
    // (for dev, retain ability to do just one)
    if (utarray_len(opam_packages) == 0) {
        /* log_debug("converting all META files in opam repo..."); */
        meta_walk(utstring_body(srcroot_lib),
                  false,      /* linkfiles */
                    /* "META",     /\* file_to_handle *\/ */
                  handle_lib_meta); /* callback */
    } else {
        log_debug("converting listed opam pkgs in %s", utstring_body(srcroot_lib));
        UT_string *s;
        utstring_new(s);
        char **a_pkg = NULL;
        /* log_trace("%*spkgs:", indent, sp); */
        while ( (a_pkg=(char **)utarray_next(opam_packages, a_pkg))) {
            utstring_clear(s);
            utstring_concat(s, srcroot_lib);
            utstring_printf(s, "/%s/%s", *a_pkg, "META");
            /* log_debug("src root: %s", utstring_body(s)); */
            /* log_trace("%*s'%s'", delta+indent, sp, *a_pkg); */
            if ( ! access(utstring_body(s), R_OK) ) {
                /* log_debug("FOUND: %s", utstring_body(s)); */
                handle_lib_meta(utstring_body(srcroot_lib), *a_pkg, "META");
            } else {
                log_fatal("NOT found: %s", utstring_body(s));
                exit(EXIT_FAILURE);
            }
        }
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
    emit_bazel_config_setting_rules(outdir);

    utarray_free(pos_flags);
    utarray_free(neg_flags);
    utstring_free(srcroot_bin);
    utstring_free(srcroot_lib);
}

void register_condition_name(char *_name, obzl_meta_flags *_flags)
{
    struct config_setting *a_condition;
    a_condition = calloc(sizeof(struct config_setting), 1);
    strncpy(a_condition->name, _name, 128);
    a_condition->flags = _flags;
    HASH_ADD_STR(the_config_settings, name, a_condition);
}

void register_flags(obzl_meta_flags *_flags)
{
    char **p;
    for (int i=0; i < obzl_meta_flags_count(_flags); i++) {
        obzl_meta_flag *flag = obzl_meta_flags_nth(_flags, i);
        char *flag_s = flag->s;
        if ( !strncmp(flag_s, "byte", 4) ) continue;
        if ( !strncmp(flag_s, "native", 6) ) continue;

        log_debug("registering flag: %s (%d)", flag_s, flag->polarity);

        utarray_sort(pos_flags,strsort);
        p = NULL;
        if (flag->polarity) { /* pos */
            log_debug("registering pos flag %s", flag->s);
            p = utarray_find(pos_flags, &flag_s, strsort);
            if ( p == NULL ) {
                log_debug("%s not found in pos_flags table; pushing.", flag_s);
                utarray_push_back(pos_flags, &flag_s);
            } else {
                log_debug("found %s in pos_flags table", flag_s);
            }
            continue;
        }
        /* else neg: */
        utarray_sort(neg_flags,strsort);
        log_debug("registering neg flag %s", flag->s);
        p = utarray_find(neg_flags, &flag_s, strsort);
        if ( p == NULL ) {
            log_debug("%s not found in neg_flags table; pushing.", flag_s);
            utarray_push_back(neg_flags, &flag_s);
        } else {
            log_debug("found %s in neg_flags table", flag_s);
        }
    }
}

/*
  special case: digestif
 */
int handle_lib_meta(char *rootdir,
                    char *pkgdir,
                    char *metafile)
{
    log_info("handle_lib_meta: %s ; %s ; %s", rootdir, pkgdir, metafile);
    log_info("outdir: %s", tgtroot_lib);

    char buf[PATH_MAX];
    buf[0] = '\0';
    mystrcat(buf, rootdir);
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
            /* emit_build_bazel_ppx(outdir, "opam", "lib", "", pkg); */
        } else {
            log_debug("handling normal package: %s", obzl_meta_package_name(pkg));
            g_ppx_pkg = true;
        }
        emit_build_bazel(outdir, "opam", "lib", "", pkg);
    }
    return 0;
}

void log_fn(log_Event *evt)
{
    /* typedef struct { */
    /*   va_list ap; */
    /*   const char *fmt; */
    /*   const char *file; */
    /*   struct tm *time; */
    /*   void *udata; */
    /*   int line; */
    /*   int level; */
    /* } log_Event; */

    /* don't use yet - somehow it ends up overwriting the output filename */

    /* char *v; */
    /* vasprintf(&v, evt->fmt, evt->ap); */

    /* char *_fname = basename((char*)evt->file); */

    UT_string *_fmt;
    utstring_new(_fmt);
    utstring_printf(_fmt, "bootstrapper/%s:%4d ", "foo", evt->line);
    /* utstring_printf(fmt, "%s", v); */
    /* utstring_printf(fmt, "\n"); */
    /* /\* fprintf(stderr, utstring_body(fmt), fname, evt->line, evt->ap); *\/ */
    /* fprintf(stderr, "%s", utstring_body(fmt)); */
    /* /\* vfprintf(stderr, "X %s:%d %s\n", fname, evt->line, (char*)evt->udata); *\/ */
    utstring_free(_fmt);
    fprintf(stdout, "XXXXXXXXXXXXXXXX");
}

int main(int argc, char *argv[]) // , char **envp)
{
    /* fprintf(stdout, "\nopam_bootstrap main\n"); */
    /* log_add_callback(log_fn, NULL, LOG_TRACE); */

#ifdef DEBUG
    log_set_quiet(false);
    log_set_level(LOG_TRACE);
#else
    log_set_quiet(true);
    log_set_level(LOG_INFO);
#endif

    /* for (char **env = envp; *env != 0; env++) { */
    /*     char *thisEnv = *env; */
    /*     printf("env: %s\n", thisEnv); */
    /* } */

    char *opam_switch;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        log_error("getcwd failure");
        exit(EXIT_FAILURE);
    }

    utarray_new(opam_packages, &ut_str_icd);

    initialize_config_flags();

#ifdef DEBUG
    char *opts = "b:o:p:s:vhx";
#else
    char *opts = "b:p:s:vhx";
#endif

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
#ifdef DEBUG
        case 'o':
            printf("option o: %s\n", optarg);
            if (optarg[0] == '/') {
                log_error("Absolute path not allowed for -o option.");
                exit(EXIT_FAILURE);
            }
            outdir[0] = '\0';
            mystrcat(outdir, cwd);
            mystrcat(outdir, "/");
            mystrcat(outdir, optarg);
            /* char *rp = realpath(optarg, outdir); */
            /* if (rp == NULL) { */
            /*     perror(optarg); */
            /*     log_fatal("realpath failed on %s", outdir); */
            /*     exit(EXIT_FAILURE); */
            /* } */
            break;
#endif
        default:
            ;
        }
    }
#ifdef DEBUG
    if (strlen(outdir) == 0) {
        /* mystrcat(outdir, cwd); */
        /* mystrcat(outdir, "/tmp"); */
    mystrcat(outdir, "./");
    }
#else
    mystrcat(outdir, "./");
#endif

    log_info("OUTDIR: %s", outdir);
    opam_config(opam_switch, outdir);

    /* log_debug("predefined flags:"); */
    struct config_flag *s, *tmp;
    HASH_ITER(hh, the_flag_table, s, tmp) {
        /* log_debug("\t%s", s->label); */
        HASH_DEL(the_flag_table, s);
        free(s);
    }

    /* log_debug("accumulated config settings:"); */
    struct config_setting *cd, *ctmp;
    HASH_ITER(hh, the_config_settings, cd, ctmp) {
        /* log_debug("\t%s: %s", cd->name, cd->label); */
        HASH_DEL(the_config_settings, cd);
        free(s);
    }

/* #ifdef DEBUG */
    log_info("outdir: %s", outdir);
    log_info("FINISHED");
/* #endif */
}
