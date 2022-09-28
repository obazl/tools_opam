#include <assert.h>
#include <regex.h>

#include "s7.h"
#include "log.h"

#include "error_handler.h"

#include "utarray.h"
#if EXPORT_INTERFACE
#include "utstring.h"
#endif

s7_pointer old_err_port;
const char *errmsg = NULL;
s7_int gc_loc = -1;

#if EXPORT_INTERFACE
#define TO_STR(x) s7_object_to_c_string(s7, x)
#endif

#define ERRSEXP "(with-let (owlet) " \
    "(format #t \"file: ~A, line ~A\n\" error-file error-line))"

/* s7_pointer _s7_read_thunk_catcher(s7_scheme *s7, s7_pointer args) */
/* { */
/*     /\* log_info("s7_read_thunk_catcher args: %s", *\/ */
/*     /\*          TO_STR(args)); *\/ */
/*     /\* log_info("s7_read_thunk_catcher arg0: %s", TO_STR(s7_car(args))); *\/ */
/*     /\* log_info("s7_read_thunk_catcher arg1: %s", TO_STR(s7_cadr(args))); *\/ */
/*     /\* s7_show_stack(s7); *\/ */

/*     /\* if arg0 == 'read-error and *\/ */
/*     if (strstr(TO_STR(s7_cadr(args)), */
/*                "(\"unexpected close paren:") != NULL) { */
/*         /\* printf("XXXXXXXXXXXXXXXX\n"); *\/ */
/*         /\* if (strstr(errmsg, "BADDOT") != NULL) { *\/ */
/*         /\* log_info(RED "fixing baddot in %s" CRESET, *\/ */
/*         /\*          utstring_body(dunefile_name)); *\/ */
/*         s7_close_input_port(s7, g_dunefile_port); */
/*         s7_gc_unprotect_at(s7, dune_gc_loc); */

/*         /\* s7_show_stack(s7); *\/ */
/*         /\* clear out old error *\/ */
/*         /\* s7_flush_output_port(s7, s7_current_error_port(s7)); *\/ */
/*         /\* close_error_config(); *\/ */
/*         /\* error_config(); *\/ */
/*         /\* init_error_handlers(); *\/ */

/*         // FIXME: test case: 'include' after baddot */
/*         s7_pointer fixed = fix_baddot(dunefile_name); */
/*         /\* s7_pointer fixed = s7_eval_c_string(s7, "'(foob)"); *\/ */
/*         if (debug) log_debug(RED "FIXED:" CRESET " %s", */
/*                              TO_STR(fixed)); */
/*         /\* s7_show_stack(s7); *\/ */
/* #if defined(DEBUG_TRACE) */
/*         print_backtrace(s7); */
/* #endif */
/*         /\* close_error_config(); *\/ */
/*         /\* error_config(); *\/ */
/*         // FIXING baddot always re-reads entire dunefile */
/*         /\* stanzas = fixed; *\/ */
/*         /\* if (s7_is_null(s7,stanzas)) { *\/ */
/*         /\*     // fixed is a list of stanzas *\/ */
/*         /\*     stanzas = fixed; *\/ */
/*         /\* } else{ *\/ */
/*         /\*     stanzas = s7_append(s7, stanzas, fixed); *\/ */
/*         /\* } *\/ */
/*         /\* } *\/ */

/*         return fixed; */
/*     } else { */
/*         fprintf(stdout, RED "Error:" CRESET " %s\n", */
/*                 TO_STR(s7_cadr(args))); */
/*         fprintf(stdout, RED "[begin error context]\n"); */
/*         s7_eval_c_string(s7, ERRSEXP); */
/*         char *sexp = "(do ((e (outlet (owlet)) (outlet e))) " */
/*             "((eq? e (rootlet))) " */
/*             "(format () \"~{~A ~}~%\" e)) "; */
/*         s7_eval_c_string(s7, sexp); */
/*         s7_write(s7, */
/*                  /\* s7_make_string(s7, s7_car(args)), *\/ */
/*                  // s7_string(s7_car(args)), */
/*                  TO_STR(s7_car(args)), */
/*                  s7_current_error_port(s7)); */
/*         fprintf(stdout, "[end error context]" CRESET "\n"); */

/*         close_error_config(); */
/*         error_config(); */
/*         /\* init_error_handlers(); *\/ */
/*         /\* s7_quit(s7); *\/ */
/*         /\* exit(EXIT_FAILURE); *\/ */
/*     } */
/* } */

s7_pointer s7_read_thunk_catcher;

LOCAL s7_pointer _s7_error_handler(s7_scheme *s7, s7_pointer args)
{
   /* log_error("_s7_error_handler\n"); */
    /* log_info("err: %s", TO_STR(args)); */

    if (strstr(s7_string(s7_car(args)), "unexpected close paren:") != NULL) {
        if (debug)
            printf(RED "Error: BAD DOT" CRESET "\n");
        s7_write(s7, s7_make_string(s7, "BADDOT"),
                 s7_current_error_port(s7));

        /* fprintf(stdout, RED "[begin error context]\n"); */
        /* s7_eval_c_string(s7, ERRSEXP); */
        /* char *sexp = "(do ((e (outlet (owlet)) (outlet e))) " */
        /*     "((eq? e (rootlet))) " */
        /*     "(format () \"~{~A ~}~%\" e)) "; */
        /* s7_eval_c_string(s7, sexp); */
        /* s7_write(s7, */
        /*          s7_make_string(s7, s7_car(args)), */
        /*          // s7_string(s7_car(args)), */
        /*          s7_current_error_port(s7)); */
        /* fprintf(stdout, "[end error context]" CRESET "\n"); */
        /* exit(EXIT_FAILURE); */
        return s7_t(s7);
    } else {
        //TODO: write to error port
        fprintf(stdout, RED "Error:" CRESET " %s\n",
                s7_string(s7_car(args)));
        fprintf(stdout, RED "[begin error context]\n");
        s7_eval_c_string(s7, ERRSEXP);
        char *sexp = "(do ((e (outlet (owlet)) (outlet e))) "
            "((eq? e (rootlet))) "
            "(format () \"~{~A ~}~%\" e)) ";
        s7_eval_c_string(s7, sexp);
        s7_write(s7,
                 /* s7_make_string(s7, s7_car(args)), */
                 // s7_string(s7_car(args)),
                 s7_car(args),
                 s7_current_error_port(s7));
        fprintf(stdout, "[end error context]" CRESET "\n");

        /* (stacktrace) has no effect(?) */
        /* s7_eval_c_string(s7, "(stacktrace)"); */

        /* printf("EXIT ON ERROR? %s\n" TO_STR(s7_name_to_value(s7, *exit-on-error*))); */

        if (s7_name_to_value(s7, "*exit-on-error*") == s7_t(s7)) {
            fprintf(stdout, RED "exiting..." CRESET "\n");
            exit(EXIT_FAILURE);
        }

        /* s7_pointer eline = s7_eval_c_string(s7, "(with-let (owlet) error-line"); */
        /* fprintf(stderr, "file: %s, line: %s\n", TO_STR(efile), TO_STR(eline)); */

        /* fprintf(stderr, "%s\n", TO_STR(owlet)); */
        /* fprintf(stderr, "\n"); */
        /* fprintf(stderr, "%s\n", TO_STR(owlet)); */
        return(s7_f(s7));
    }
}

LOCAL s7_pointer _s7_read_error_handler(s7_scheme *s7, s7_pointer args)
{
    fprintf(stderr, RED "READ ERROR:" CRESET " %s\n",
            s7_string(s7_car(args)));
    s7_eval_c_string(s7, ERRSEXP);
    return(s7_f(s7));
}

void init_error_handlers(void)
{
    /* s7_read_thunk_catcher = s7_make_function(s7, "s7-read-thunk-catcher", */
    /*                                          _s7_read_thunk_catcher, */
    /*                                          2, 0, false, ""); */

   s7_define_function(s7, "error-handler",
                       _s7_error_handler, 1, 0, false,
                       "our error handler");

    s7_eval_c_string(s7, "(set! (hook-functions *error-hook*) \n\
                            (list (lambda (hook) \n\
                                    (error-handler \n\
                                      (apply format #f (hook 'data))) \n\
                                    (set! (hook 'result) 'our-error))))");

    /* read-error-hook evidently only catches problems with # names
       and \ escapes, not general read errors. */
    s7_define_function(s7, "read-error-handler",
                       _s7_read_error_handler, 1, 0, false,
                       "our read error handler");

    /* s7_eval_c_string(s7, "(set! (hook-functions *read-error-hook*) \n\ */
    /*                         (list (lambda (hook) \n\ */
    /*                                 (read-error-handler \n\ */
    /*                                   (apply format #f (hook 'data))) \n \ */
    /*                                 (set! (hook 'result) 'READ-error))))"); */


}

void error_config(void)
{
    /* if (trace) log_trace(BLU "error_config" CRESET); */

    old_err_port = s7_set_current_error_port(s7, s7_open_output_string(s7));
    if (old_err_port != s7_nil(s7)) {
        /* if (s7_is_output_port(s7, old_err_port)) { */
        /*     s7_flush_output_port(s7, old_err_port); */
        /* } */
        gc_loc = s7_gc_protect(s7, old_err_port);
    }
    s7_flush_output_port(s7, s7_current_error_port(s7));
}

void close_error_config(void) // s7_pointer err_port)
{
    s7_close_output_port(s7, s7_current_error_port(s7));

    s7_set_current_error_port(s7, old_err_port);
    if (gc_loc != -1)
        s7_gc_unprotect_at(s7, gc_loc);
}

/* char *dunefile_to_string(UT_string *dunefile_name) */
/* { */
/*     /\* FIXME: use malloc, this will rarely be called *\/ */
/*     /\* 16K should be enough for any dunefile *\/ */
/* #define BUFSZ 16384 */
/*     static char buffer[BUFSZ]; */
/*     memset(buffer, '\0', BUFSZ); */
/*     /\* FIXME: what about e.g. unicode in string literals? *\/ */

/*     FILE *inFp = fopen(utstring_body(dunefile_name), "r"); */
/*     fseek(inFp, 0, SEEK_END); */
/*     uint64_t fileSize = ftell(inFp); */
/*     /\* log_debug("filesize: %d", fileSize); *\/ */
/*     if (fileSize > BUFSZ) { */
/*         log_error("dune file size (%d) > BUFSZ (%d)\n", fileSize, BUFSZ); */
/*         exit(EXIT_FAILURE);     /\* FIXME: exit gracefully *\/ */
/*     } */
/*     rewind(inFp); */

/*     char *fixbuf = malloc(fileSize + 1); */
/*     memset(fixbuf, '\0', fileSize); */

/*     uint64_t outFileSizeCounter = fileSize; */

/*     /\* we fread() bytes from inFp in COPY_BUFFER_MAXSIZE increments, */
/*        until there is nothing left to fread() *\/ */
/*     int read_ct = 0; */
/*     do { */
/*         if (outFileSizeCounter > BUFSZ) { */
/*             /\* probably won't see a 16K dune file *\/ */
/*             read_ct = fread(buffer, 1, (size_t) BUFSZ, inFp); */
/*             if (read_ct != BUFSZ) { */
/*                 if (ferror(inFp) != 0) { */
/*                     log_error("fread error 1 for %s\n", */
/*                               utstring_body(dunefile_name)); */
/*                     exit(EXIT_FAILURE); //FIXME: exit gracefully */
/*                 } */
/*             } */
/*             /\* log_debug("writing"); *\/ */
/*             outFileSizeCounter -= BUFSZ; */
/*         } */
/*         else { */
/*             read_ct = fread(buffer, 1, (size_t) outFileSizeCounter, inFp); */
/*             if (read_ct != outFileSizeCounter) { */
/*                 if (ferror(inFp) != 0) { */
/*                     log_error("fread error 2 for %s\n", */
/*                               utstring_body(dunefile_name)); */
/*                     exit(EXIT_FAILURE); //FIXME: exit gracefully */
/*                 } else { */
/*                     if (feof(inFp) == 0) { */
/*                         log_error("fread error 3 for %s\n", */
/*                                   utstring_body(dunefile_name)); */
/*                         exit(EXIT_FAILURE); //FIXME: exit gracefully */
/*                     } */
/*                 } */
/*             } */
/*             outFileSizeCounter = 0ULL; */
/*         } */
/*     } while (outFileSizeCounter > 0); */
/*     /\* log_debug("readed %d bytes", read_ct); *\/ */
/*     fclose(inFp); */

/*     /\* printf(RED "READED:\n" CRESET " %s\n", buffer); *\/ */

/*     // FIXME: loop over the entire buffer */
/*     char *bptr = (char*)buffer; */
/*     char *fptr = (char*)fixbuf; */

/*     regex_t re; */
/*     int rc = regcomp(&re, "\\. *)", REG_EXTENDED); */
/*     assert(rc == 0); */

/*     regmatch_t matches[1]; */

/*     while (true) { */
/*         /\* printf(RED "bptr:\n" CRESET " %s\n", bptr); *\/ */
/*         /\* printf(RED "fixbuf:\n" CRESET " %s\n", fixbuf); *\/ */

/*         //FIXME: use regex.  When the need arises. */
/*         /\* rc = regexec(&re, bptr, *\/ */
/*         /\*              sizeof(matches)/sizeof(matches[0]), *\/ */
/*         /\*              (regmatch_t*)&matches,0); *\/ */
/*         /\* if (rc == 0) { *\/ */
/*         /\*     printf(MAG "regex match:" CRESET " %s\n", *\/ */
/*         /\*            bptr + matches[0].rm_so); *\/ */
/*         /\*     /\\* char *val = strndup(data+matches[1].rm_so, *\\/ *\/ */
/*         /\*     /\\*                     matches[1].rm_eo - matches[1].rm_so); *\\/ *\/ */
/*         /\* } else { *\/ */
/*         /\*     printf("regex NO match\n"); *\/ */
/*         /\* } *\/ */

/*         char *cursor = strstr((const char*) bptr, ".)"); */


/*         if (cursor == NULL) { */
/*             size_t ct = strlcpy(fptr, (const char*)bptr, strlen(bptr)); */
/*             break; */
/*         } else { */
/*             /\* log_debug("FOUND \".)\" at pos: %d", cursor - buffer); *\/ */
/*             size_t ct = strlcpy(fptr, (const char*)bptr, cursor - bptr); */
/*             if (ct >= BUFSZ) { */
/*                 // output string has been truncated */
/*             } */
/*             fptr = fptr + (cursor - bptr) - 1; */
/*             fptr[cursor - bptr] = '\0'; */
/*             ct = strlcat(fptr, " ./", BUFSZ); */
/*             fptr += 3; */

/*             bptr = bptr + (cursor - bptr) + 1; */

/*             /\* printf(GRN "bptr:\n" CRESET " %s\n", bptr); *\/ */

/*             if (ct >= BUFSZ) { */
/*                 // output string has been truncated */
/*             } */
/*             /\* log_debug("first seg: %s", fixbuf); *\/ */
/*             /\* log_debug("first seg len: %d", strlen((char*)fixbuf)); *\/ */
/*             /\* log_debug("cursor - buffer = %d", cursor - buffer); *\/ */
/*             /\* log_debug("second seg %s", buffer + 225); *\/ */
/*             /\* ct = strlcat((char*)fixbuf, buffer + (cursor - buffer) + 1, BUFSZ); *\/ */
/*             /\* if (ct >= BUFSZ) { *\/ */
/*             /\*     // output string has been truncated *\/ */
/*             /\* } *\/ */
/*             /\* log_debug("fixed: %s", (char*)fixbuf); *\/ */
/*         } */

/*     } */
/*     /\* log_debug("final:\n %s", (char*)fixbuf); *\/ */
/*     return fixbuf; */
/* } */

/* s7_pointer fix_baddot(UT_string *dunefile_name) */
/* { */
/*     //FIXME: this duplicates the code in load_dune:_read_dunefile */
/*     log_debug("fix_baddot"); */

/*     char *dunestring = dunefile_to_string(dunefile_name); */

/*     /\* now s7_read using string port *\/ */

/*     /\* first config err handling. clears out prev. error *\/ */
/*     close_error_config(); */
/*     error_config(); */
/*     /\* init_error_handling(); *\/ */

/*     /\* stanza accumulator *\/ */
/*     s7_pointer stanzas = s7_list(s7, 0); */

/*     s7_pointer sport = s7_open_input_string(s7, dunestring); */
/*     /\* s7_int baddot_gc_loc = s7_gc_protect(s7, sport); *\/ */

/*     errmsg = s7_get_output_string(s7, s7_current_error_port(s7)); */
/*     if (!s7_is_input_port(s7, sport)) { */
/*         errmsg = s7_get_output_string(s7, s7_current_error_port(s7)); */
/*         if ((errmsg) && (*errmsg)) { */
/*             log_error("[%s\n]", errmsg); */
/*             s7_shutdown(s7); */
/*             exit(EXIT_FAILURE); */
/*         } */
/*     } */
/*     if (debug) */
/*         log_debug("s7_open_input_string for error correction"); */

/*     /\* read all stanzas in dunefile *\/ */
/*     while(true) { */
/*         /\* log_debug("iter"); *\/ */
/*         s7_pointer stanza = s7_read(s7, sport); */
/*         /\* log_debug("stanza: %s", stanza); *\/ */
/*         errmsg = s7_get_output_string(s7, s7_current_error_port(s7)); */
/*         if ((errmsg) && (*errmsg)) { */
/*             if (debug) log_error("[%s\n]", errmsg); */
/*             s7_close_input_port(s7, sport); */
/*             s7_shutdown(s7); */
/*             exit(EXIT_FAILURE); */
/*             break; */
/*         } */
/*         if (stanza == s7_eof_object(s7)) break; */
/*         log_debug("stanza: %s", TO_STR(stanza)); */
/*         if (s7_is_null(s7,stanzas)) { */
/*             stanzas = s7_list(s7, 1, stanza); */
/*         } else{ */
/*             stanzas = s7_append(s7,stanzas, s7_list(s7, 1, stanza)); */
/*         } */
/*     } */
/*     s7_close_input_port(s7, sport); */
/*     /\* s7_gc_unprotect_at(s7, baddot_gc_loc); *\/ */
/*     close_error_config(); */

/*     /\* leave error config as-is *\/ */
/*     free(dunestring); */
/*     return stanzas; */
/* } */

LOCAL void s7_shutdown(s7_scheme *s7)
{
    close_error_config();
    s7_quit(s7);
}
