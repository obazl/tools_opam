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
%token FVF_LOGOP.
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
%token TERM.
%token TERM_STRING.
%token TERM_VARIDENT.
%token TRUE.
%token URL.
%token VARIDENT.
%token VERSION.

%right RELOP.
%right BANG.
%right QMARK.

%right LOGOP.

/* **************** */
%token_type { union opam_token_u } /* terminals */

/* %default_type { struct opam_entry_s * } /\* non-terminal default *\/ */

%type package { struct opam_package_s * }
%type binding { struct opam_binding_s * }
%type deps { struct opam_deps_s * }
%type stringlist { UT_array * }

%syntax_error {
    log_error("**************** SYNTAX ERROR! ****************");
/* args passed:
   yyParser *yypParser, /\* The parser *\/
   int yymajor, /\* The major type of the error token *\/
   ParseTOKENTYPE yyminor /\* The minor type of the error token *\/
   ParseTOKENTYPE is union opam_token_u
 */


%ifdef YYDEBUG_EXIT_ON_ERROR
   exit(EXIT_FAILURE);
%else
     log_error("trying to recover...");
%endif
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
    log_debug(" bindings ct: %d",
              HASH_CNT(hh, opam_parse_state->pkg->entries));
#endif
}

%ifdef YYDEBUG_FILTER
package ::= filter . {
    log_debug("START filter");
}
%endif

%ifdef YYDEBUG_FPF
package ::= filtered_package_formulas . {
    log_debug("START filtered_package_formulas");
}
%endif

%ifdef YYDEBUG_FVF
package ::= fvf_expr . {
    log_debug("START fvf");
    /* printf("    bindings ct: %d\n", */
    /*        HASH_CNT(hh, opam_parse_state->pkg->entries)); */
}
%endif

 /****************************************************************/
bindings ::= binding . {
/* #if YYDEBUG */
/*     printf("ONE BINDING\n"); */
/* #endif */
}

bindings(Bindings_lhs) ::= bindings(Bindings_rhs) binding . {
#if YYDEBUG
    log_debug("BINDINGS ct: %d",
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
    log_debug("BINDING: %s: %s", Keyword.s, String.s);
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
    log_debug("BINDING stringlist %s, ct: %d",
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
    log_debug("BINDING stringlist val ct: %d",
           utarray_len((UT_array*)Binding->val));
#endif
}

/****************************************************************/
/* build: [ [ <term> { <filter> } ... ] { <filter> } ... ] */
binding(Binding) ::=
    BUILD COLON LBRACKET build_cmds RBRACKET . {
#if YYDEBUG
        log_debug("BINDING build");
#endif
    Binding = calloc(sizeof(struct opam_binding_s), 1);
    Binding->name = strndup("build", 7);
    Binding->t = BINDING_BUILD;
    /* Binding->val =  */
    HASH_ADD_KEYPTR(hh,
                    opam_parse_state->pkg->entries,
                    Binding->name, strlen(Binding->name), Binding);
}

build_cmds ::= . {
    log_debug("build_cmds null");
}

build_cmds ::= build_cmds build_cmd . {
    log_debug("build_cmds list");
}

build_cmd ::= LBRACKET build_terms RBRACKET . {
    log_debug("build_cmd");
}

build_cmd ::= LBRACKET build_terms RBRACKET term_filter . {
    log_debug("build_cmd filtered");
}

build_terms ::= . {
    log_debug("build_terms");
}

build_terms ::= build_terms TERM_STRING . {
    log_debug("build_terms TERM");
}

build_terms ::= build_terms TERM_VARIDENT . {
    log_debug("build_terms TERM");
}

term_filter ::= LBRACE FILTER RBRACE . {
    log_debug("term_filter");
}

/****************************************************************/
// depends: [ <filtered-package-formula> ... ]
binding(Binding) ::=
    DEPENDS COLON LBRACKET filtered_package_formulas RBRACKET . {
#if YYDEBUG
        log_debug("BINDING depends");
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
    log_debug("STRINGLIST single: %s", String.s);
#endif
    utarray_new(Stringlist, &ut_str_icd);
    utarray_push_back(Stringlist, &String.s);
}

stringlist(Stringlist_lhs) ::= stringlist(Stringlist) STRING(String) . {
#if YYDEBUG
    log_debug("STRINGLIST multiple, ct: %d; new: %s",
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

    filtered_package_formulas ::= . {
#if YYDEBUG
        log_debug("filtered_package_formulas: null");
#endif
    }

    filtered_package_formulas ::= filtered_package_formulas fpf . {
#if YYDEBUG
        log_debug("filtered_package_formulas leftrec");
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
        log_debug("fpf: pkgname fvf_expr");
#if YYDEBUG
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

    fvf ::= . {
#if YYDEBUG
        log_debug("fvf: null");
#endif
    }

        fvf ::= fvf logop_fvf . {
#if YYDEBUG
        log_debug("fvf: fvf logop_fvf");
#endif
    }

        logop_fvf ::= LOGOP fvf . {
#if YYDEBUG
        log_debug("fvf: fvf logop fvf");
#endif
    }

/*         fvf ::=  BANG fvf . { */
/* #if YYDEBUG */
/*         log_debug("fvf: bang fvf"); */
/* #endif */
/*     } */

/*         fvf ::= QMARK fvf . { */
/* #if YYDEBUG */
/*         log_debug("fvf: qmark fvf"); */
/* #endif */
/*     } */

// | "(" <filtered-version-formula> ")"
/*         fvf ::=  LPAREN fvf RPAREN . { */
/* #if YYDEBUG */
/*         log_debug("fvf: (fvf)"); */
/* #endif */
/*     } */

        // | <relop> <version>
/*         fvf_base ::= RELOP VERSION . { */
/* #if YYDEBUG */
/*         log_debug("fvf_base: relop version"); */
/* #endif */
/*     } */

       fvf ::= fvf_base . {
#if YYDEBUG
        log_debug("fvf: fvf_base");
#endif
       }

       fvf ::= filter_expr . [AMP] {
#if YYDEBUG
        log_debug("fvf: filter_expr");
#endif
       }

            fvf_base ::= VERSION . { // treated as <string>
#if YYDEBUG
        log_debug("fvf_base: version");
#endif
    }
        // | <filter>
/*         fvf_base ::= filter . { */
/* #if YYDEBUG */
/*             log_debug("fvf_base: filter"); */
/* #endif */
/*     } */

        // | <relop> <filter>
/*         fvf_base ::= RELOP filter . { */
/* #if YYDEBUG */
/*         log_debug("fvf_base: relop filter"); */
/* #endif */
/*     } */

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

   NB: <varident>, <string>, <int>, <bool> are lexed as FILTER tokens
    */

        filter_expr ::= filter . [LOGOP] {
#if YYDEBUG
        log_debug("filter_expr: filter");
#endif
    }

            filter_expr ::= filter LOGOP filter . {
#if YYDEBUG
        log_debug("filter_expr: filter_expr logop_filter");
#endif
    }

        filter ::= RELOP VERSION . {
#if YYDEBUG
        log_debug("filter: relop version");
#endif
    }

        filter ::= RELOP filter . {
#if YYDEBUG
        log_debug("filter: relop filter");
#endif
    }

/*         logop_filter ::= LOGOP filter . { */
/* #if YYDEBUG */
/*         log_debug("logop_filter: logop filter"); */
/* #endif */
/*     } */

    filter ::= LPAREN filter RPAREN . {
#if YYDEBUG
        log_debug("filter: (filter)");
#endif
    }

    filter ::= BANG filter . {
#if YYDEBUG
        log_debug("filter: BANG filter");
#endif
    }

    filter ::= QMARK filter . {
#if YYDEBUG
        log_debug("filter: QMARK filter");
#endif
    }

    filter ::= FILTER . { // varident|string|int|bool
#if YYDEBUG
        log_debug("filter: FILTER");
#endif
    }

    filter ::= VARIDENT . {
#if YYDEBUG
        log_debug("filter: VARIDENT");
#endif
    }
