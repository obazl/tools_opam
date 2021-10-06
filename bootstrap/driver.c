#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
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

#include "log.h"
#include "driver.h"

int errnum;

/* global: we write on new_local_repository rule per build file */
FILE *repo_rules_FILE;

char *rootdir = "buildfiles";

#if EXPORT_INTERFACE
#define TOKEN_NAME(x) (char*)#x
#endif

 char *token_names[256] = {
     [DESCRIPTION] = TOKEN_NAME(description),
     [DIRECTORY] = TOKEN_NAME(directory),
     [DQ] = TOKEN_NAME(dq),
     [EQ] = TOKEN_NAME(eq),
     [ERROR] = TOKEN_NAME(error),
     [LPAREN] = TOKEN_NAME(lparen),
     [PACKAGE] = TOKEN_NAME(package),
     [PLUSEQ] = TOKEN_NAME(pluseq),
     [REQUIRES] = TOKEN_NAME(requires),
     [RPAREN] = TOKEN_NAME(rparen),
     [VERSION] = TOKEN_NAME(version),
     [VNAME] = TOKEN_NAME(vname),
     [WARNING] = TOKEN_NAME(warning),
     [WORD]    = TOKEN_NAME(word),
     [WORDS]    = TOKEN_NAME(words),
 };

#if EXPORT_INTERFACE
struct logging {
    int verbosity;
    int log_level;
    int parse_verbosity;
    int parse_log_level;
    int lex_verbosity;
    int lex_log_level;
    bool quiet;
    bool log_color;
};
#endif

EXPORT struct logging logger;

/* char THE_METAFILE[PATH_MAX]; */
struct obzl_meta_package *MAIN_PKG;

LOCAL char *package_name_from_file_name(char *fname)
{
    char *bn = basename(fname);
    int x = strlen(bn) - 5;
    if ( strncmp(&bn[x], ".META", 5) == 0) {
        bn[x] = '\0';
        return bn;
    } else {
        return basename(dirname(fname));
    }
}

bool is_empty(const char *s)
{
  while (*s) {
    if (!isspace(*s))
      return false;
    s++;
  }
  return true;
}

UT_array *skipped_pkgs;

/* skip ocaml core pkgs plus a few others */
/* void _initialize_skipped_pkg_list() */
/* { */
/*     utarray_new(skipped_pkgs, &ut_str_icd); */
/*     const char *s, **p; */

/*     s = "compiler-libs/META"; */
/*     utarray_push_back(skipped_pkgs, &s); */

/*     s = "threads/META"; */
/*     utarray_push_back(skipped_pkgs, &s); */

/*     s = "digestif/META"; */
/*     utarray_push_back(skipped_pkgs, &s); */

/*     s = "ctypes/META"; */
/*     utarray_push_back(skipped_pkgs, &s); */

/*     s = "ptime/META"; */
/*     utarray_push_back(skipped_pkgs, &s); */

/*     printf("sorting\n"); */
/*     utarray_sort(skipped_pkgs,strsort); */
/* } */

/* void _free_skipped_pkg_list() */
/* { */
/* } */

bool _skip_pkg(char *pkg)
{
    int len = strlen(pkg);

    /* avoid matching ocaml-compiler-libs */
    if (strncmp(pkg + len - 19, "/compiler-libs/META", 19) == 0) {
        log_warn("SKIPPING compiler-libs/META");
        return true;
    }
    /* skip pkgs "distributed with OCaml" - repo rule installs them */
    /* if (strncmp(pkg + len - 12, "dynlink/META", 12) == 0) { */
    /*     log_warn("SKIPPING dynlink/META"); */
    /*     return true; */
    /* } */
    /* if (strncmp(pkg + len - 12, "threads/META", 12) == 0) { */
    /*     log_warn("SKIPPING threads/META"); */
    /*     return true; */
    /* } */
    /* if (strncmp(pkg + len - 9, "unix/META", 9) == 0) { */
    /*     log_warn("SKIPPING unixt/META"); */
    /*     return true; */
    /* } */


    /* TMP HACK: skip some pkgs for which we use template BUILD files */
    /* if (strncmp(pkg + len - 13, "digestif/META", 13) == 0) { */
    /*     log_warn("SKIPPING digestif/META"); */
    /*     return true; */
    /* } */

    /* if (strncmp(pkg + len - 13, "ptime/META", 10) == 0) { */
    /*     log_warn("SKIPPING ptime/META"); */
    /*     return true; */
    /* } */

    return false;
}

EXPORT struct obzl_meta_package *obzl_meta_parse_file(char *fname)
{
/* #ifdef DEBUG */
/*     log_set_quiet(false); */
/* #else */
/*     log_set_quiet(true); */
/* #endif */
    /* log_info("obzl_meta_parse_file: %s", fname); */
    FILE *f;

    f = fopen(fname, "r");
    if (f == NULL) {
        /* errnum = errno; */
        /* log_error("fopen failure for %s", fname); */
        /* log_error("Value of errno: %d", errnum); */
        /* log_error("fopen error %s", strerror( errnum )); */
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    const size_t fsize = (size_t) ftell(f);
    if (fsize == 0) {
        fclose(f);
        errno = -1;
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    char *buffer = (char*) malloc(fsize + 1);
    fread(buffer, 1, fsize, f);
    buffer[fsize] = 0;
    fclose(f);

    if (is_empty(buffer)) {
        fclose(f);
        errno = -2;
        return NULL;
    }

    /* THE_METAFILE[0] = '\0'; */
    /* mystrcat(THE_METAFILE, fname); */

    struct meta_lexer * lexer = malloc(sizeof(struct meta_lexer));
    lexer_init(lexer, fname, buffer);

    void* pParser = ParseAlloc (malloc);
    /* InitParserState(ast); */
    /* ParseTrace(stdout, "trace_"); */
    int tok;
    union meta_token *mtok = malloc(sizeof(union meta_token));

    /* if (logger.lex_verbosity == 0) */
    /*     log_set_quiet(true); */
    /* else */
    /*     log_set_quiet(logger.quiet); */
    log_set_level(logger.lex_log_level);

    /* log_set_quiet(false); */
    /* log_set_level(LOG_TRACE); */
    /* log_info("starting"); */
    /* log_set_quiet(true); */

    MAIN_PKG = (struct obzl_meta_package*)calloc(sizeof(struct obzl_meta_package), 1);
    MAIN_PKG->name      = package_name_from_file_name(fname);
    MAIN_PKG->directory = MAIN_PKG->name; // dirname(fname);
    MAIN_PKG->metafile  = fname;

    while ( (tok = get_next_token(lexer, mtok)) != 0 ) {
        /* log_set_quiet(true); */
#if defined(DEBUG_LEX)
        switch(tok) {
        case DIRECTORY:
            log_trace("lex DIRECTORY: %s", mtok->s); break;
        case FLAGS:
            log_trace("lex FLAGS: %s", mtok->s); break;
        case VNAME:
            log_trace("lex VNAME: %s", mtok->s); break;
        case WORD:
            log_trace("lex WORD: %s", mtok->s); break;
        case WORDS:
            log_trace("lex WORDS: %s", mtok->s); break;
        case DQ:
            log_trace("DQ"); break;
        case EQ:
            log_trace("lex EQ"); break;
        case PLUSEQ:
            log_trace("lex PLUSEQ"); break;
        case LPAREN:
            log_trace("lex LPAREN"); break;
        case RPAREN:
            log_trace("lex RPAREN"); break;
        case VERSION:
            log_trace("lex VERSION: %s", mtok->s);
            break;
        case DESCRIPTION:
            log_trace("lex DESCRIPTION: %s", mtok->s);
            break;
        case REQUIRES:
            log_trace("lex REQUIRES"); break;
        case PACKAGE:
            log_trace("lex PACKAGE: %s", mtok->s); break;
        case WARNING:
            log_trace("WARNING"); break;
        case ERROR:
            log_trace("ERROR"); break;
        default:
            log_trace("other: %d", tok); break;
        }
#endif
        Parse(pParser, tok, mtok, MAIN_PKG); // , &sState);

        mtok = malloc(sizeof(union meta_token));
        /* if (logger.lex_verbosity == 0) */
        /*     log_set_quiet(false); */
        /* else */
        /*     log_set_quiet(logger.quiet); */
        /*     log_set_level(logger.lex_log_level); */
    }

    /*     if (logger.parse_verbosity == 0) */
    /*         log_set_quiet(false); */
    /*     else */
    /*         log_set_quiet(logger.quiet); */
    /*         log_set_level(logger.parse_log_level); */
    /* log_set_quiet(true); */

    log_trace("lex: end of input");

    Parse(pParser, 0, mtok, MAIN_PKG); // , &sState);
    ParseFree(pParser, free );

    /* if (logger.verbosity == 0) */
    /*     log_set_quiet(false); */
    /* else */
    /*     log_set_quiet(logger.quiet); */
    /* log_set_level(logger.log_level); */

    /* log_set_quiet(false); */

    /* log_trace("PARSED %s", fname); */

    free(buffer);
    return MAIN_PKG;
}

EXPORT char *obzl_meta_version()
{
    return "0.1.0";
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

/* FIXME: rename to sth like ingest_opam_pkgs */
void opam_config(char *_opam_switch, char *bzlroot)
{
    log_info("opam_config bzlroot: %s", bzlroot);

    /* _initialize_skipped_pkg_list(); */

    /* we're going to write one 'new_local_repository' target per
       build file: */
    UT_string *repo_rules_filename = NULL;
    utstring_new(repo_rules_filename);
    utstring_printf(repo_rules_filename, "%s/opam_repos.bzl", bzlroot);
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
    mkdir_r(bzlroot, "");       /* make sure bzlroot exists */
    UT_string *bzl_bin;
    utstring_new(bzl_bin);
    utstring_printf(bzl_bin, "%s/bin", bzlroot);

    UT_string *bzl_lib;
    utstring_new(bzl_lib);
    utstring_printf(bzl_lib, "%s/buildfiles", bzlroot);

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

    fclose(repo_rules_FILE);

    /* _free_skipped_pkg_list(); */
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

        /* skip ocaml core pkgs, we do them by hand */
        if (_skip_pkg(buf)) {
            log_debug("SKIPPING pkg %s", buf);
            return 0;
        }

        emit_new_local_pkg_repo(repo_rules_FILE,
                                /* _pkg_prefix, */
                                pkg);

        UT_string *imports_path;
        utstring_new(imports_path);
        /* utstring_printf(imports_path, "_lib/%s", */
        utstring_printf(imports_path, "%s",
                        obzl_meta_package_name(pkg));
        emit_build_bazel(host_repo,
                         bzlroot,      /* _repo_root: "." or "./tmp/opam" */
                         NULL, // "buildfiles",        /* _pkg_prefix */
                         utstring_body(imports_path),
                        /* "",      /\* pkg-path *\/ */
                         pkg);
    }
    return 0;
}
