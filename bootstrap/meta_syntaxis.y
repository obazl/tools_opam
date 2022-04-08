%include {
#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

#if defined(YYDEBUG)
static int indent = 2;
static int delta = 2;
static char *sp = " ";
#endif

#if INTERFACE
#ifndef YYMALLOCARGTYPE
#define YYMALLOCARGTYPE size_t
#endif
#endif
}

%extra_argument { struct obzl_meta_package *the_root_pkg }

%token PACKAGE VERSION DESCRIPTION DIRECTORY VNAME FLAGS WORD WORDS.
%token  REQUIRES PPX_RUNTIME_DEPS .
%token WARNING ERROR.
%token DQ EQ PLUSEQ LPAREN RPAREN.

/* PLUGIN. */
/* %token PWORD.                   /\* P = PROP | PRED *\/ */
 /* COMMENT */
/* %token ARCHIVE */
/* PRED NPRED. */
/* EXISTS_IF PPX PPXOPT LINKOPTS */
/* %token VALTOK.  /\* DEPLIST. *\/ */
 /* DQSTR */

/* flags */
/* BYTE NATIVE TOPLOOP CREATE_TOPLOOP MT MT_POSIX MT_VM GPROF AUTOLINK PREPROCESSOR SYNTAX. */

/* %token LIBRARY_KIND PPX_RUNTIME_DEPS CUSTOM_PPX PPX_DRIVER. */

/* **************** */
%token_type { union meta_token* }

/* %type archive { UT_array* } // { struct obzl_meta_property* } */
/* %destructor archive { */
/*     log_trace("freeing archive"); */
/*     /\* FIXME: free components *\/ */
/*     /\* free($$); *\/ */
/* } */

/* %type plugin { UT_array* } // { struct obzl_meta_property* } */
/* %destructor plugin { */
/*     log_trace("freeing plugin"); */
/*     /\* FIXME: free components *\/ */
/*     /\* free($$); *\/ */
/* } */

%type property { struct obzl_meta_property* }
%destructor property {
    log_trace("freeing property");
    /* free($$); */
}

%type opcode { enum obzl_meta_opcode_e }

%type entry { struct obzl_meta_entry* }
%destructor entry {
    log_trace("freeing entry");
    free($$);
}

%type entries { obzl_meta_entries* } // UT_array* }
%destructor entries {
    log_trace("freeing entries");
    utarray_free($$->list);
}

%type flag { struct obzl_meta_flag* }
%destructor flag {
    log_trace("freeing flag %s", $$->s);
    free($$);
}

%type flags { UT_array* }
%destructor flags {
    log_trace("freeing flags");
    utarray_free($$);
}

/* %type properties { struct obzl_meta_property* } */
%type properties { UT_array* }  /* array of obzl_meta_property */
%destructor properties {
    log_trace("freeing properties");
    /* free($$); */
}

%type package { struct obzl_meta_package* }
%destructor package {
    log_trace("freeing package");
    /* free($$); */
}

%type packages { UT_array* }
%destructor packages {
    log_trace("freeing packages");
    /* free($$); */
}

/* %type words { UT_array* } */
%type words { obzl_meta_values* }
%destructor words {
    log_trace("freeing words");
    utarray_free($$->list);
}


%syntax_error {
    log_trace("**************** Syntax error! ****************");
    exit(EXIT_FAILURE);
}

/* %parse_accept { */
/*     log_trace("Parsing complete"); */
/* } */

/*
We have two kinds of properties, simple and compound. Simple: foo =
"bar"; compound: foo(bar) = "baz".

The lexer passes simple props as a single token; lhs is token type, rhs is token string. So the parser just needs to convert, and we have one rule per simple prop token, e.g. for VERSION (ie 'version = "1.2.1"')

simple_prop(PROPERTY) ::= VERSION(V) . { ...construct struct obzl_meta_property for the version property... }

Compound props must be parsed with a more complex rule, e.g.
property(PROPERTY) ::= REQUIRES(R) opcode(OPCODE) VALTOK(V). {... construct obzl_meta_property for 'requires' prop...}

 */

/* **************************************************************** */
package(PKG) ::= entries(ENTRIES) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>package ::= entries");
    /* log_trace("  package (PKG)"); */
    /* log_trace("  entries (ENTRIES)"); */
    /* dump_entries(0, ENTRIES); */
#endif
    /* PKG = (struct obzl_meta_package*)calloc(sizeof(struct obzl_meta_package), 1); */
    /* PKG->entries = ENTRIES; */
    the_root_pkg->entries = ENTRIES;
    /* *the_root_pkg = PKG; */
    PKG = the_root_pkg;
#if YYDEBUG
    log_trace("DUMPING Pkg: %s", PKG->name);
    dump_package(0, PKG);
    log_trace("DUMPED Pkg: PKG");
#endif
}

entries(ENTRIES) ::= entry(ENTRY) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entries ::= entry");
    log_trace("%*sentries lhs (ENTRIES)", indent, sp);
    log_trace("%*sentry (ENTRY)", indent, sp);
    dump_entry(indent, ENTRY);
#endif
    /* lhs ENTRIES: lemon only allocates the type (a ptr); we must allocate/init the struct */
    ENTRIES = (obzl_meta_entries*)calloc(sizeof(struct obzl_meta_entries), 1);
    utarray_new(ENTRIES->list, &entry_icd);
    utarray_push_back(ENTRIES->list, ENTRY);
#if YYDEBUG
    dump_entries(indent, ENTRIES);
#endif
}

/* **************************************************************** */
entries(ENTRIES) ::= entries(PREVENTRIES) entry(ENTRY) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entries ::= entries entry");
    log_trace("%*sentries lhs(ENTRIES)", indent, sp); //, obzl_meta_entries_count(ENTRIES));
    log_trace("%*sentries rhs(PREVENTRIES)", indent, sp); //, obzl_meta_entries_count(PREVENTRIES));
    dump_entries(delta+indent, PREVENTRIES);
    log_trace("%*sentry (ENTRY)", indent, sp); //, obzl_meta_entries_count(PREVENTRIES));
    dump_entry(delta+indent, ENTRY);
#endif

    normalize_entries(PREVENTRIES, ENTRY);
    ENTRIES = PREVENTRIES;
    /* log_trace("after normalization"); */
#if YYDEBUG
    /* log_trace("//entries ::= entries entry DONE"); */
    /* dump_entries(delta+indent, ENTRIES); */
#endif
}

/* **************************************************************** */
/*     SIMPLE PROPS     */
entry(ENTRY) ::= VNAME(VAR) opcode(OPCODE) WORD(W) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= VNAME opcode WORD");
    log_trace("  entry (ENTRY)");
    log_trace("  var (VAR): %s", VAR->s);
    log_trace("  opcode (OPCODE): %d", OPCODE);
    /* if (W) */
    /*     log_trace("  WORD (W): %s", W->s); */
    /* else */
    /*     log_trace("  WORD (W): <empty>", W->s); */
#endif
    ENTRY = handle_simple_prop(VAR, OPCODE, W);
    /* dump_entry(indent, ENTRY); */
}

entry(ENTRY) ::= VNAME(P) opcode(OPCODE) WORDS(WS) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= VNAME opcode WORDS");
    log_trace("\tentry (ENTRY)");
    log_trace("\tpword (P): %s", P->s);
    log_trace("\topcode (OPCODE): %d", OPCODE);
    log_trace("\tWORD (W): %s", WS->s);
#endif
    ENTRY = handle_simple_prop(P, OPCODE, WS);
    /* dump_entry(delta, ENTRY); */
}

/* **************************************************************** */
/* ****************
PRIMITIVE PROPS
**************** */
/* entry(ENTRY) ::= COMMENT(C) . { */
/* #if YYDEBUG */
/*     log_trace("\n"); */
/*     log_trace(">>entry ::= COMMENT"); */
/*     log_trace("  entry (ENTRY)"); */
/*     log_trace("  COMMENT (C): %s", C->s); */
/* #endif */
/*     ENTRY = handle_primitive_prop(DESCRIPTION, D); */
/* #if YYDEBUG */
/*     log_trace("dumping new description :"); */
/*     dump_entry(delta, ENTRY); */
/* #endif */
/* } */

/* entry(ENTRY) ::= DESCRIPTION(D) . { */
/* #if YYDEBUG */
/*     log_trace("\n"); */
/*     log_trace(">>entry ::= DESCRIPTION"); */
/*     log_trace("  entry (ENTRY)"); */
/*     log_trace("  DESCRIPTION (D): %s", D->s); */
/* #endif */
/*     ENTRY = handle_primitive_prop(DESCRIPTION, D); */
/* #if YYDEBUG */
/*     log_trace("dumping new description :"); */
/*     dump_entry(delta, ENTRY); */
/* #endif */
/* } */

entry(ENTRY) ::= DIRECTORY(D) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= DIRECTORY");
#endif
    ENTRY = handle_primitive_prop(DIRECTORY, D);
#if YYDEBUG
    /* dump_entry(0, ENTRY); */
#endif
}

/* entry(ENTRY) ::= ERROR(V) . { */
/* #if YYDEBUG */
/*     log_trace("\n"); */
/*     log_trace(">>entry ::= ERROR"); */
/* #endif */
/*     ENTRY = handle_primitive_prop(ERROR, V); */
/*     /\* dump_entry(0, ENTRY); *\/ */
/* } */

/* entry(ENTRY) ::= VERSION(V) . { */
/* #if YYDEBUG */
/*     log_trace("\n"); */
/*     log_trace(">>entry ::= VERSION"); */
/*     log_trace("  entry (ENTRY)"); */
/*     log_trace("  VERSION (V): %s", V->s); */
/* #endif */
/*     ENTRY= handle_primitive_prop(VERSION, V); */
/* #if YYDEBUG */
/*     log_trace("dumping new version:"); */
/*     dump_entry(indent, ENTRY); */
/* #endif */
/* } */

/* entry(ENTRY) ::= WARNING(V) . { */
/*     log_trace("\n"); */
/*     log_trace(">>entry ::= WARNING"); */
/*     ENTRY = handle_primitive_prop(WARNING, V); */
/*     /\* dump_entry(0, ENTRY); *\/ */
/* } */

/* **************************************************************** */
entry(ENTRY) ::= PACKAGE(PKG) LPAREN entries(ENTRIES) RPAREN. {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= PACKAGE LPAREN entries RPAREN");
    log_trace("  entry lhs(ENTRY): %p", ENTRY);
    log_trace("  entries rhs(ENTRIES)");
    log_trace("  the_root_pkg->name: %s", the_root_pkg->name);
    log_trace("  the_root_pkg->path: %s", the_root_pkg->path);
    log_trace("  the_root_pkg->directory: %s", the_root_pkg->directory);
    log_trace("  the_root_pkg->metafile: %s", the_root_pkg->metafile);
    /* entries: utarray of struct obzl_meta_entry */
    dump_entries(indent, ENTRIES);
#endif
    /* ENTRY = handle_primitive_prop(PACKAGE, PKG); */
    struct obzl_meta_package *new_package = (struct obzl_meta_package*)calloc(sizeof(struct obzl_meta_package), 1);
    new_package->name = PKG->s;
    /* directory will be filled in during post-processing, from parent and directory prop. */
    /* new_package->directory = the_root_pkg->directory; */
    new_package->path = the_root_pkg->path; // directory;
    new_package->metafile = the_root_pkg->metafile; // THE_METAFILE;
    new_package->entries = ENTRIES;
    /* dump_package(indent, new_package); */

    ENTRY = (struct obzl_meta_entry*)calloc(sizeof(struct obzl_meta_entry), 1);
    ENTRY->type = OMP_PACKAGE;
    ENTRY->package = new_package;
    /* log_trace("xxxxxxxxxxxxxxxx"); */
    /* dump_entry(indent, ENTRY); */
}

/* ************************************************************ */
/*     COMPOUND PROPS     */
entry(ENTRY) ::= VNAME(VAR) FLAGS(Flags) opcode(OPCODE) words(WS) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= VNAME FLAGS opcode words");
    log_trace("\tentry (ENTRY)");
    log_trace("\tvarname (VAR): %s", VAR->s);
    log_trace("\tflags (FLAGS): %s", Flags->s);
    /* dump_flags(FLAGS); */
    log_trace("\topcode (OPCODE): %d", OPCODE);
    log_trace("\twords (WS)"); //: %s", W->s);
#endif

    /* new_prop->name = strdup(VAR->s); */
    struct obzl_meta_property *new_prop= obzl_meta_property_new(strdup(VAR->s));
    /* struct obzl_meta_property *new_prop= (struct obzl_meta_property*)malloc(sizeof(struct obzl_meta_property)); */
    /* utarray_new(new_prop->settings->list, &obzl_meta_setting_icd); */

    struct obzl_meta_setting *new_setting = obzl_meta_setting_new(Flags->s, OPCODE, WS);
    utarray_push_back(new_prop->settings->list, new_setting);

    /* new_prop->setting = (struct obzl_meta_setting*)malloc(sizeof(struct obzl_meta_setting)); */
    /* new_prop->setting->flags = obzl_meta_flags_new_tokenized(FLAGS->s); */
    /* new_prop->setting->opcode = OPCODE; */
    /* new_prop->setting->values = WS; */

    struct obzl_meta_entry *new_entry= (struct obzl_meta_entry*)malloc(sizeof(struct obzl_meta_entry));
    new_entry->type = OMP_PROPERTY;
    new_entry->property = new_prop;

    ENTRY = new_entry;
    /* log_trace("output:"); */
    /* dump_entry(0, ENTRY); */
    /* dump_values(WS); */
    /* ENTRY = handle_compound_prop(P, FLAGS, OPCODE, WS); */
    /* dump_entry(0, ENTRY); */
}

/* **************************************************************** */
/* special case: REQUIRES */
/*  words in value string for requires must be validated. we do this here instead of in the lexer. */
entry(ENTRY) ::= REQUIRES(P) opcode(OPCODE) words(WS) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= REQUIRES opcode words");
    log_trace("%*sentry (ENTRY)", indent, sp);
    log_trace("%*sopcode (OPCODE): %d", indent, sp, OPCODE);
    log_trace("%*swords (WS)", indent, sp);
#endif

    /* validate_requires_args(WS); */

    /* /\* ENTRY = handle_primitive_prop(P, OPCODE, WS); *\/ */
    /* struct obzl_meta_property *new_prop= (struct obzl_meta_property*)malloc(sizeof(struct obzl_meta_property)); */
    /* /\* new_prop->name = strdup(token_names[token_type]); // "version"); *\/ */
    /* new_prop->name = strdup("requires"); */
    /* /\* new_prop->setting = (struct obzl_meta_setting*)malloc(sizeof(struct obzl_meta_setting)); *\/ */
    /* /\* new_prop->setting->flags = NULL; *\/ */
    /* /\* new_prop->setting->opcode = OPCODE; *\/ */
    /* /\* new_prop->setting->values  = WS; *\/ */
    /* utarray_new(new_prop->settings->list, &obzl_meta_setting_icd); */
    /* struct obzl_meta_setting *new_setting = obzl_meta_setting_new(NULL, OPCODE, WS); */
    /* utarray_push_back(new_prop->settings->list, new_setting); */

    struct obzl_meta_property *new_prop= obzl_meta_property_new(strdup("requires"));

    struct obzl_meta_setting *new_setting = obzl_meta_setting_new(NULL, OPCODE, WS);
    utarray_push_back(new_prop->settings->list, new_setting);

    struct obzl_meta_entry *new_entry= (struct obzl_meta_entry*)malloc(sizeof(struct obzl_meta_entry));
    new_entry->type = OMP_PROPERTY;
    new_entry->property = new_prop;

    ENTRY = new_entry;
    /* dump_entry(delta+indent, ENTRY); */
}

entry(ENTRY) ::= REQUIRES FLAGS(FLAGS) opcode(OPCODE) words(WS) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= REQUIRES LPAREN flag RPAREN opcode words");
    log_trace("%*sentry (ENTRY)", indent, sp);
    log_trace("%*sflags (FLAGS): %s", indent, sp, FLAGS->s);
    /* dump_flags(FLAGS); */
    log_trace("%*sopcode (OPCODE): %d", indent, sp, OPCODE);
    log_trace("%*swords (WS)", indent, sp);
    /* dump_values(WS); */
#endif
    /* ENTRY = handle_requires_prop(FLAGS, OPCODE, WS); */

    /* validate_requires_args(WS); */

    /* struct obzl_meta_property *new_prop= (struct obzl_meta_property*)malloc(sizeof(struct obzl_meta_property)); */
    /* new_prop->name = strdup("requires"); */
    struct obzl_meta_property *new_prop= obzl_meta_property_new(strdup("requires"));

    struct obzl_meta_setting *new_setting = obzl_meta_setting_new(FLAGS->s, OPCODE, WS);
    utarray_push_back(new_prop->settings->list, new_setting);

    /* utarray_new(new_prop->settings->list, &obzl_meta_setting_icd); */
    /* struct obzl_meta_setting *new_setting = obzl_meta_setting_new(FLAGS->s, OPCODE, WS); */
    /* utarray_push_back(new_prop->settings->list, new_setting); */

    /* new_prop->setting = (struct obzl_meta_setting*)malloc(sizeof(struct obzl_meta_setting)); */
    /* new_prop->setting->flags = obzl_meta_flags_new_tokenized(FLAGS->s); */
    /* new_prop->setting->opcode = OPCODE; */
    /* new_prop->setting->values = WS; */

    struct obzl_meta_entry *new_entry= (struct obzl_meta_entry*)malloc(sizeof(struct obzl_meta_entry));
    new_entry->type = OMP_PROPERTY;
    new_entry->property = new_prop;

    ENTRY = new_entry;
    /* log_trace("output:"); */
    /* dump_entry(0, ENTRY); */
}


/* **************************************************************** */
/* special case: PPX_RUNTIME_DEPS */
/*  words in value string for requires must be validated. we do this here instead of in the lexer. */
entry(ENTRY) ::= PPX_RUNTIME_DEPS opcode(OPCODE) words(WS) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= PPX_RUNTIME_DEPS opcode words");
    log_trace("%*sentry (ENTRY)", indent, sp);
    log_trace("%*sopcode (OPCODE): %d", indent, sp, OPCODE);
    log_trace("%*swords (WS)", indent, sp);
#endif

    struct obzl_meta_property *new_prop= obzl_meta_property_new(strdup("ppx_runtime_deps"));

    struct obzl_meta_setting *new_setting = obzl_meta_setting_new(NULL, OPCODE, WS);
    utarray_push_back(new_prop->settings->list, new_setting);

    struct obzl_meta_entry *new_entry= (struct obzl_meta_entry*)malloc(sizeof(struct obzl_meta_entry));
    new_entry->type = OMP_PROPERTY;
    new_entry->property = new_prop;

    ENTRY = new_entry;
    /* dump_entry(delta+indent, ENTRY); */
}

entry(ENTRY) ::= PPX_RUNTIME_DEPS FLAGS(FLAGS) opcode(OPCODE) words(WS) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>entry ::= PPX_RUNTIME_DEPS LPAREN flag RPAREN opcode words");
    log_trace("%*sentry (ENTRY)", indent, sp);
    log_trace("%*sflags (FLAGS): %s", indent, sp, FLAGS->s);
    /* dump_flags(FLAGS); */
    log_trace("%*sopcode (OPCODE): %d", indent, sp, OPCODE);
    log_trace("%*swords (WS)", indent, sp);
    /* dump_values(WS); */
#endif
    struct obzl_meta_property *new_prop= obzl_meta_property_new(strdup("ppx_runtime_deps"));

    struct obzl_meta_setting *new_setting = obzl_meta_setting_new(FLAGS->s, OPCODE, WS);
    utarray_push_back(new_prop->settings->list, new_setting);

    struct obzl_meta_entry *new_entry= (struct obzl_meta_entry*)malloc(sizeof(struct obzl_meta_entry));
    new_entry->type = OMP_PROPERTY;
    new_entry->property = new_prop;

    ENTRY = new_entry;
    /* log_trace("output:"); */
    /* dump_entry(0, ENTRY); */
}


/* ****************************************************************
SPECIAL CASE: ARCHIVE

"The variable "archive" specifies the list of archive files. These files should be given either as (1) plain names without any directory information; they are only searched in the package directory. (2) Or they have the form "+path" in which case the files are looked up relative to the standard library. (3) Or they have the form "@name/file" in which case the files are looked up in the package directory of another package. (4) Or they are given as absolute paths."

/* ****************** */
words(WORDS) ::= WORD(W) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>words ::= WORD");
    log_trace("\twords lhs(WORDS)");
    if (W->s == NULL)
        log_trace("\tWORD (W): NULL");
    else
        log_trace("\tWORD (W): %s", W->s);
#endif
    /* UT_array *new_values = obzl_meta_values_new_copy(W->s); */
    obzl_meta_values *new_values = obzl_meta_values_new(W->s);
    /* dump_values(indent, new_values); */
    WORDS = new_values;
}

words(A) ::= WORDS(WS) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>words ::= WORDS");
    log_trace("\twords lhs(A)");
    log_trace("\tWORDS (WS): %s", WS->s);
#endif
    obzl_meta_values *new_values = obzl_meta_values_new_tokenized(WS->s);
    /* dump_values(indent, new_values); */
    A = new_values;
    /* utarray_new(new_values, &ut_str_icd); */
    /* utarray_push_back(new_values, &(VALTOK->s)); */
    /* new_prop->setting->values = new_values; */
}

/* **************************************************************** */
/* valtok(A) ::= VALTOK(B) . { */
/*     log_trace(">>valtok(A) ::= VALTOK(B)"); */
/*     log_trace("\tvaltok: %s", B->s); */
/*     A = B; */
/* } */

opcode(A) ::= EQ(B) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>opcode ::= EQ");
    log_trace("\topcode lhs(A)");
    log_trace("\tEQ (B)");
#endif
    A = OP_SET;
}

opcode(A) ::= PLUSEQ(B) . {
#if YYDEBUG
    log_trace("\n");
    log_trace(">>opcode(A) ::= PLUSEQ(B)");
#endif
    A = OP_UPDATE;
}

