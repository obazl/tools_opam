/* types etc. for parsed opam files */

#if EXPORT_INTERFACE
#include "uthash.h"

enum phrase_type_e {
    BINDING_BOOL,
    BINDING_BUILD,
    BINDING_DEPENDS,
    BINDING_INT,
    BINDING_STRING,
    BINDING_STRINGLIST,
};

struct opam_binding_s {
    char *name;                    /* key */
    // WARNING: if we use 'int t', then the enum above will not be
    // included by makeheaders.
    phrase_type_e t;
    void *val;
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct opam_package_s {
    char *fname;
    struct opam_binding_s *entries; /* uthash table */
}
#endif
