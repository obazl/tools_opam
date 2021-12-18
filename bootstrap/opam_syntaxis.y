%include {
#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
/* #include "opam_package.h" */

#if INTERFACE
#ifndef YYMALLOCARGTYPE
#define YYMALLOCARGTYPE size_t
#endif
#endif

}

/* opam_parse_state->pkg->entries is a uthash f bindings. parse rules
update it directly */
%extra_argument { struct opam_parse_state_s *opam_parse_state }
/* %extra_argument { struct opam_package_s *opam_pkg } */

/* we piggyback on lemon to declare some type consts */
/* %token BINDING_BOOL. */
/* %token BINDING_INT. */
/* %token BINDING_STRING. */
/* %token BINDING_STRINGLIST. */

%token AMP.
%token AUTHORS.
%token AVAILABLE.
%token BANG.
%token BUG_REPORTS.
%token BOOL.
%token BUILD.
%token BUILD_DOC.
%token BUILD_ENV.
%token COLON.
%token COMMA.
%token CONFLICTS.
%token CONFLICT_CLASS.
%token DEPENDS.
%token DEPEXTS.
%token DEPOPTS.
%token DESCRIPTION.
%token DEV_REPO.
%token DOC.
%token DQ.
%token EQ.
%token ERROR.
%token EXTRA_FILES.
%token EXTRA_SOURCE.
%token FALSE.
%token FEATURES.
%token FILTER.
%token FLAGS.
%token HOMEPAGE.
%token IDENT.
%token IDENTCHAR.
%token INSTALL.
%token KEYWORD.
%token LBRACE.
%token LBRACKET.
%token LICENSE.
%token LOGOP.
%token LPAREN.
%token MAINTAINER.
%token MESSAGES.
%token OPAM_VERSION.
%token PACKAGE.
%token PATCHES.
%token PIN_DEPENDS.
%token PKGNAME.
%token POST_MESSAGES.
%token QMARK.
%token RBRACE.
%token RBRACKET.
%token REMOVE.
%token RELOP.
%token RPAREN.
%token RUN_TEST.
%token SETENV.
%token SQ.
%token STRING.
%token SUBSTS.
%token SYNOPSIS.
%token TAGS.
%token TRUE.
%token URL.
%token VARIDENT.
%token VERSION.

%left  LOGOP.
%right RELOP.

/* **************** */
%token_type { union opam_token_u } /* terminals */

/* %default_type { struct opam_entry_s * } /\* non-terminal default *\/ */

%type package { struct opam_package_s * }
%type binding { struct opam_binding_s * }
%type deps { struct opam_deps_s * }
%type stringlist { UT_array * }

%syntax_error {
    log_trace("**************** Syntax error! ****************");
    fprintf(stderr, "trying to recover...\n");
   /* exit(EXIT_FAILURE); */
}

%parse_accept {
    log_trace("Parsing complete");
}

%parse_failure {
    fprintf(stderr,"Giving up.  Parser is hopelessly lost...\n");
}

/* *******************  PARSE RULES  ********************* */
%start_symbol package

package ::= bindings . {
#if YYDEBUG
    log_debug("PACKAGE");
    printf("    bindings ct: %d\n", HASH_CNT(hh, opam_parse_state->pkg->entries));
#endif
}

%ifdef YYDEBUG_FILTER
package ::= fvf_expr . {
    log_debug("START fvf_expr");
    /* printf("    bindings ct: %d\n", */
    /*        HASH_CNT(hh, opam_parse_state->pkg->entries)); */
}
%endif

%ifdef YYDEBUG_FPF
package ::= filtered_package_formulas . {
    log_debug("START filtered_package_formulas");
    /* printf("    bindings ct: %d\n", */
    /*        HASH_CNT(hh, opam_parse_state->pkg->entries)); */
}
%endif

bindings ::= binding . {
/* #if YYDEBUG */
/*     printf("ONE BINDING\n"); */
/* #endif */
}

bindings(Bindings_lhs) ::= bindings(Bindings_rhs) binding . {
#if YYDEBUG
    printf("BINDINGS ct: %d\n",
           HASH_CNT(hh, opam_parse_state->pkg->entries));
#endif
    /* no need to do anything, opam_parse_state->pkg->entries already
       contains all bindings */
    Bindings_lhs = Bindings_rhs;
}

/* <field-binding> ::= <ident> : <value> */
/* <value> ::= <bool> | <int> | <string> | <ident> | <varident> | <operator> | <list> | <option> | "(" <value> ")" */

binding(Binding) ::= KEYWORD(Keyword) COLON STRING(String) . {
#if YYDEBUG
    printf("BINDING: %s: %s\n", Keyword.s, String.s);
#endif
    /* create a binding and add it to the pkg hashmap */
    Binding = calloc(sizeof(struct opam_binding_s), 1);
    Binding->name = Keyword.s;
    Binding->t = BINDING_STRING;
    Binding->val = strdup(String.s);
    HASH_ADD_KEYPTR(hh, // opam_parse_state->pkg->entries->hh,
                    opam_parse_state->pkg->entries,
                    Binding->name, strlen(Binding->name), Binding);
}

binding(Binding) ::= KEYWORD(Keyword) COLON LBRACKET stringlist(Stringlist) RBRACKET . {
#if YYDEBUG
    printf("BINDING stringlist %s, ct: %d\n",
           Keyword.s, utarray_len(Stringlist));
#endif
    /* create a binding and add it to the pkg hashmap */
    Binding = calloc(sizeof(struct opam_binding_s), 1);
    Binding->name = Keyword.s;
    Binding->t = BINDING_STRINGLIST;
    Binding->val = (void*)Stringlist;
    HASH_ADD_KEYPTR(hh,
                    opam_parse_state->pkg->entries,
                    Binding->name, strlen(Binding->name), Binding);
#if YYDEBUG
    printf("BINDING stringlist val ct: %d\n",
           utarray_len((UT_array*)Binding->val));
#endif
}

// depends: [ <filtered-package-formula> ... ]
    binding(Binding) ::=
        DEPENDS COLON LBRACKET filtered_package_formulas RBRACKET . {
#if YYDEBUG
        printf("BINDING depends\n");
#endif
    Binding = calloc(sizeof(struct opam_binding_s), 1);
    Binding->name = strndup("depends", 7);
    Binding->t = BINDING_DEPENDS;
    /* Binding->val =  */
    HASH_ADD_KEYPTR(hh,
                    opam_parse_state->pkg->entries,
                    Binding->name, strlen(Binding->name), Binding);
}

stringlist(Stringlist) ::= STRING(String) . {
#if YYDEBUG
    printf("STRINGLIST single: %s\n", String.s);
#endif
    utarray_new(Stringlist, &ut_str_icd);
    utarray_push_back(Stringlist, &String.s);
}

stringlist(Stringlist_lhs) ::= stringlist(Stringlist) STRING(String) . {
#if YYDEBUG
    printf("STRINGLIST multiple, ct: %d; new: %s\n",
           utarray_len(Stringlist), String.s);
#endif
    utarray_push_back(Stringlist, &String.s);
    Stringlist_lhs = Stringlist;
}

/* depends: [ <filtered-package-formula> ... ]
   <filtered-package-formula> ::=
   <filtered-package-formula> <logop> <filtered-package-formula>
   | ( <filtered-package-formula> )
   | <pkgname> { { <filtered-version-formula> }* }

    <pkgname>         ::= (") <ident> (")
*/
/*     pkgname ::= DQ IDENT DQ . { */
/* #if YYDEBUG */
/*         printf("pkgname\n"); */
/* #endif */
/*     } */

    filtered_package_formulas ::= filtered_package_formulas fpf . {
#if YYDEBUG
        log_debug("filtered_package_formulas leftrec");
#endif
    }

    filtered_package_formulas ::= . {
#if YYDEBUG
        printf("filtered_package_formulas: null\n");
#endif
    }

    fpf ::= fpf LOGOP fpf . {
#if YYDEBUG
        log_debug("fpf: fpf LOGOP fpf");
#endif
    }

    fpf ::= LPAREN fpf RPAREN . {
#if YYDEBUG
        log_debug("fpf: (fpf)");
#endif
    }

    fpf ::= PKGNAME fvf_expr . {
#if YYDEBUG
        log_debug("fpf: pkgname fvf_expr");
#endif
    }

    fpf ::= PKGNAME . {
#if YYDEBUG
        log_debug("fpf: pkgname");
#endif
    }

    // ################################################################
        fvf_expr ::= LBRACE fvf RBRACE . {
#if YYDEBUG
        log_debug("fvf_expr: braces");
#endif
    }
/*
   <filtered-version-formula> ::=
   <filtered-version-formula> <logop> <filtered-version-formula>
   | "!" <filtered-version-formula>
   | "?" <filtered-version-formula>
   | "(" <filtered-version-formula> ")"
   | <relop> <version>
   | <filter>
   | <relop> <filter>

   e.g.   "ocaml" {>= "4.04.0"}
*/
        fvf ::= fvf LOGOP fvf . {
#if YYDEBUG
        log_debug("fvf: fvf logop fvf");
#endif
    }

/*         fvf ::=  BANG fvf . { */
/* #if YYDEBUG */
/*         printf("fvf: relop version\n"); */
/* #endif */
/*     } */
/*         fvf ::= QMARK fvf . { */
/* #if YYDEBUG */
/*         printf("fvf: relop version\n"); */
/* #endif */
/*     } */

// | "(" <filtered-version-formula> ")"
        fvf ::=  LPAREN fvf RPAREN . {
#if YYDEBUG
        log_debug("fvf: parens");
#endif
    }

       fvf ::= fvf_base . {
#if YYDEBUG
        log_debug("fvf: fvf_base");
#endif
       }

        // | <relop> <version>
        fvf_base ::= RELOP VERSION . {
#if YYDEBUG
        log_debug("fvf_base: relop version");
#endif
    }
            fvf_base ::= VERSION . { // treated as <string>
#if YYDEBUG
        log_debug("fvf_base: version");
#endif
    }
        // | <filter>
        fvf_base ::= filter . {
#if YYDEBUG
            log_debug("fvf_base: filter");
#endif
    }
        // | <relop> <filter>
        fvf_base ::= RELOP filter . {
#if YYDEBUG
        log_debug("fvf_base: relop filter");
#endif
    }

    /*
   <filter> ::= <filter> <logop> <filter>
   | "!" <filter>
   | "?" <filter>
   | ( <filter> )
   | <filter> <relop> <filter>
   | <varident>
   | <string>
   | <int>
   | <bool>
    */

    filter ::= FILTER . { // varindent|string|int|bool
#if YYDEBUG
        log_debug("filter: FILTER");
#endif
    }

/*         filter ::= filter RELOP filter . { */
/* #if YYDEBUG */
/*         printf("filter: relop\n"); */
/* #endif */
/*     } */

/*         filter_paren ::= LPAREN filter RPAREN . { */
/* #if YYDEBUG */
/*         printf("filter: parens\n"); */
/* #endif */
/*     } */

/*         filter ::= filter LOGOP filter . { */
/* #if YYDEBUG */
/*         printf("filter: logop\n"); */
/* #endif */
/*     } */
        filter ::= BANG filter . {
#if YYDEBUG
        printf("filter: bang\n");
#endif
    }
        filter ::= QMARK filter . {
#if YYDEBUG
        printf("filter: qmark\n");
#endif
    }

/*
example:
depends: [
  "ocaml" {>= "4.04.0"}
  "dune" {>= "1.8"}
  "alcotest" {with-test & >= "0.8.1"}
  "bigstringaf"
  "result"
  "ppx_let" {with-test & >= "0.14.0"}
  "ocaml-syntax-shims" {build}
]
*/


