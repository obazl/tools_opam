#include "meta_packages.h"
#include "log.h"

static int indent = 2;
static int delta = 2;
static char *sp = " ";

#if INTERFACE
struct obzl_meta_package {
    char *name;
    char *directory;
    char *metafile;
    obzl_meta_entries *entries;          /* list of struct obzl_meta_entry */
};
#endif

EXPORT char *obzl_meta_package_name(obzl_meta_package *_pkg)
{
    return _pkg->name;
}

EXPORT char *obzl_meta_package_dir(obzl_meta_package *_pkg)
{
    return _pkg->directory;
}

EXPORT char *obzl_meta_package_src(obzl_meta_package *_pkg)
{
    return _pkg->metafile;
}

EXPORT obzl_meta_entries *obzl_meta_package_entries(obzl_meta_package *_pkg)
{
    return _pkg->entries;
}

EXPORT bool obzl_meta_package_has_archives(obzl_meta_package *_pkg)
{
    //FIXME: use a has_archives flag

    obzl_meta_entries *entries = _pkg->entries;
    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PROPERTY) {
            if (strncmp(e->property->name, "archive", 7) == 0) {
                return true;
            }
        }
    }
    return false;
}

EXPORT bool obzl_meta_package_has_plugins(obzl_meta_package *_pkg)
{
    //FIXME: use a has_plugins flag

    obzl_meta_entries *entries = _pkg->entries;
    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PROPERTY) {
            if (strncmp(e->property->name, "plugin", 7) == 0) {
                return true;
            }
        }
    }
    return false;
}

EXPORT bool obzl_meta_package_has_subpackages(obzl_meta_package *_pkg)
{
    //FIXME: use a has_subpackages flag
    log_debug("obzl_meta_package_has_subpackages");
    obzl_meta_entries *entries = _pkg->entries;
    obzl_meta_entry *e = NULL;

    for (int i = 0; i < obzl_meta_entries_count(entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        log_debug("entry type: %d", e->type);
        if (e->type == OMP_PROPERTY) {
            log_debug("Property entry: %s", e->property->name);
        } else {
            if (e->type == OMP_PACKAGE) {
                log_debug("Package entry: %s", e->package->name);
                return true;
            }
        }
    }
    return false;
}

EXPORT obzl_meta_property *obzl_meta_package_property(obzl_meta_package *_pkg, char *_name)
{
#if DEBUG_TRACE
    log_trace("obzl_meta_package_property('%s')", _name);
#endif
    /* utarray_find requires a sort; not worth the cost */
    obzl_meta_entry *e = NULL;
    for (int i = 0; i < obzl_meta_entries_count(_pkg->entries); i++) {
        e = obzl_meta_entries_nth(_pkg->entries, i);
        if (e->type == OMP_PROPERTY) {
            if (strncmp(e->property->name, _name, 256) == 0) {
                return e->property;
            }
        }
        /* log_debug("iteration %d", i); */
    }
    return NULL;
}

/* **************************************************************** */
#if DEBUG_TRACE
void dump_package(int indent, struct obzl_meta_package *pkg)
{
    log_debug("%*sdump_package:", indent, sp);
    log_debug("%*sname:      %s", delta+indent, sp, pkg->name);
    log_debug("%*sdirectory: %s", delta+indent, sp, pkg->directory);
    log_debug("%*smetafile:  %s", delta+indent, sp, pkg->metafile);
    dump_entries(delta+indent, pkg->entries);
}
#endif
