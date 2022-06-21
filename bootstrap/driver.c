#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
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

#include "utstring.h"

#include "log.h"
#include "driver.h"

int errnum;
/* bool local_opam; */

bool g_ppx_pkg;

/* global: we write on new_local_repository rule per build file */
/* FILE *bootstrap_FILE; */

char *pfxdir = NULL; // ""; // buildfiles";

#if EXPORT_INTERFACE
#include <stdbool.h>
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

char *package_name_from_file_name(char *fname)
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

/* skip pkgs "distributed with OCaml" - repo rule installs them into @ocaml
*/
bool _skip_pkg(char *pkg)
{
    log_debug("_skip_pkg: %s", pkg);

    static const char * skips[] = {
        "lib/bigarray/META",
        "lib/compiler-libs/META",
        "lib/dynlink/META",
        "lib/num/META",
        "lib/ocamldoc/META",
        "lib/stdlib/META",
        "lib/str/META",
        "lib/threads/META",
        "lib/unix/META",
        "lib/raw_spacetime/META",
    };

#define skips_ct (sizeof (skips) / sizeof (const char *))

    int len = strlen(pkg);
    int skiplen;

    for (int i = 0; i < skips_ct; i++) {
        skiplen = strlen(skips[i]);
        if (strncmp(pkg + len - skiplen,
                    skips[i], skiplen) == 0) {
            log_warn("SKIPPING %s", skips[i]);
            return true;
        }
    }


    /* avoid matching ocaml-compiler-libs */
    if (strncmp(pkg + len - 22, "lib/compiler-libs/META", 16) == 0) {
        log_warn("SKIPPING compiler-libs/META");
        return true;
    }

    if (strncmp(pkg + len - 16, "lib/dynlink/META", 16) == 0) {
        log_warn("SKIPPING dynlink/META");
        return true;
    }
    if (strncmp(pkg + len - 22, "lib/ocamldoc/META", 22) == 0) {
        log_warn("SKIPPING ocamldoc/META");
        return true;
    }
    if (strncmp(pkg + len - 21, "lib/threads/META", 21) == 0) {
        log_warn("SKIPPING threads/META");
        return true;
    }
    if (strncmp(pkg + len - 18, "lib/unix/META", 18) == 0) {
        log_warn("SKIPPING unix/META");
        return true;
    }

    // version-dependent: bigarray, num, raw_spacetime

    // Bigarray moved to standard lib in v. 4.07
    // so no need to list as explicit dep?
    if (strncmp(pkg + len - 22, "lib/bigarray/META", 22) == 0) {
        log_warn("SKIPPING bigarray/META");
        return true;
    }

    // raw_spacetime - removed in v. 4.12(?)
    if (strncmp(pkg + len - 27, "lib/raw_spacetime/META", 27) == 0) {
        log_warn("SKIPPING raw_spacetime/META");
        return true;
    }

    // num - moved from core to separate lib in v. 4.06.0
    // skip if version < 4.06 ?
    // however, opam seems to install num* in lib/ocaml "for backward compatibility", and installs lib/num/META.
    /* "New applications that need arbitrary-precision arithmetic should use the Zarith library (https://github.com/ocaml/Zarith) instead of the Num library, and older applications that already use Num are encouraged to switch to Zarith." */
    // https://www.dra27.uk/blog/platform/2018/01/31/num-system.html
    if (strncmp(pkg + len - 17, "lib/num/META", 17) == 0) {
        log_warn("SKIPPING num/META");
        return true;
    }

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

    struct meta_lexer_s *meta_lexer = malloc(sizeof(struct meta_lexer_s));
    meta_lexer_init(meta_lexer, fname, buffer);

    void* pMetaParser = ParseAlloc (malloc);
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
    MAIN_PKG->path      = dirname(fname);
    MAIN_PKG->directory = MAIN_PKG->name; // dirname(fname);
    MAIN_PKG->metafile  = fname;

    while ( (tok = get_next_meta_token(meta_lexer, mtok)) != 0 ) {
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
        Parse(pMetaParser, tok, mtok, MAIN_PKG); // , &sState);

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

    /* log_trace("lex: end of input"); */

    Parse(pMetaParser, 0, mtok, MAIN_PKG); // , &sState);
    ParseFree(pMetaParser, free );

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

/* char *run_cmd(char *cmd) */
/* { */
/*     static char buf[PATH_MAX]; */
/*     FILE *fp; */

/*     if ((fp = popen(cmd, "r")) == NULL) { */
/*         printf("Error opening pipe!\n"); */
/*         return NULL; */
/*     } */

/*     while (fgets(buf, sizeof buf, fp) != NULL) { */
/*         /\* printf("SWITCH: %s\n", buf); *\/ */
/*         buf[strcspn(buf, "\n")] = 0; */
/*     } */

/*     if(pclose(fp))  { */
/*         printf("Command not found or exited with error status\n"); */
/*         return NULL; */
/*     } */
/*     return buf; */
/* } */

/*
  special cases: digestif, threads
 */
int handle_lib_meta(char *switch_lib,
                    char *obazl_opam_root,
                    /* char *_imports_path, */
                    char *pkgdir,
                    char *metafile)
{
    log_debug("================ HANDLE_LIB_META ================");
    log_debug("  switch_lib: %s; obazl_opam_root: %s; pkgdir: %s; metafile: %s",
              switch_lib, obazl_opam_root, pkgdir, metafile);

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
    /* log_debug("PARSING: %s", buf); */
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
            /* emit_build_bazel_ppx(obazl_opam_root, host_repo, "lib", "", pkg); */
        } else {
            log_debug("handling normal package: %s", obzl_meta_package_name(pkg));
            g_ppx_pkg = true;
        }

        /* skip ocaml core pkgs, we do them by hand */
        if (_skip_pkg(buf)) {
            log_debug("SKIPPING pkg %s", buf);
            return 0;
        }

        /* emit_new_local_pkg_repo(bootstrap_FILE, /\* _pkg_prefix, *\/ */
        /*                         pkg); */
        emit_local_repo_decl(bootstrap_FILE, pkg);

        UT_string *imports_path;
        utstring_new(imports_path);
        /* utstring_printf(imports_path, "_lib/%s", */
        utstring_printf(imports_path, "%s",
                        obzl_meta_package_name(pkg));
        log_debug("emitting buildfile for pkg: %s", pkg->name);
        /* dump_package(0, pkg); */
        emit_build_bazel(host_repo,
                         obazl_opam_root,      /* _repo_root: "." or "./tmp/opam" */
                         NULL, // "buildfiles",        /* _pkg_prefix */
                         utstring_body(imports_path),
                        /* "",      /\* pkg-path *\/ */
                         pkg);
        log_debug("emittED buildfile for pkg: %s", pkg->name);
    }
    return 0;
}
