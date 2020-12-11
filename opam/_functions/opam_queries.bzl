load(":ppx.bzl", "is_ppx_driver")

############################
def opam_predicate(repo_ctx, pkg, predicate):
    "Tests OPAM pkg to see if its value for predicate (i.e. 'var') is 'true'."
    pred = pkg + ":" + predicate
    is_predicated = repo_ctx.execute(["opam", "config", "var", pred])
    if is_predicated.return_code == 0:
        if is_predicated.stdout.strip() == "true":
            # print("PKG '{pkg}' satisfies predicate '{pred}'".format(
            #     pkg=pkg, pred=predicate
            # ))
            return True
        elif is_predicated.stdout.strip() == "false":
            # print("PKG '{pkg}' does NOT satisfy predicate '{pred}'".format(
            #     pkg=pkg, pred=predicate
            # ))
            return False
        elif is_predicated.stdout.strip() == "not found":
            print("PKG {pkg} predicate '{pred}' NOT FOUND".format(
                pkg=pkg, pred=predicate
            ))
            return False
        else:
            print("Unexpected result on cmd 'opam config var {pred}".format(pred=pred))
            print("cmd RC: {rc}".format(rc = is_predicated.return_code))
            print("cmd STDOUT: {msg}".format(msg = is_predicated.stdout))
            print("cmd STDERR: {msg}".format(msg = is_predicated.stderr))
            return False
    else:
        print("ERROR on cmd 'opam config var {pred}".format(pred=pred))
        print("ERROR RC: {rc}".format(rc = is_predicated.return_code))
        print("ERROR STDOUT: {msg}".format(msg = is_predicated.stdout))
        print("ERROR STDERR: {msg}".format(msg = is_predicated.stderr))

############################
def opam_property(repo_ctx, pkg, property, value):
    "Tests OPAM pkg to see if its value for property (i.e. 'var') matches param 'value'"
    prop = pkg + ":" + property
    has_property = repo_ctx.execute(["opam", "config", "var", prop])
    if has_property.return_code == 0:
        if has_property.stdout.strip() == "not found":
            print("ERROR: PKG '{pkg}' property '{prop}' NOT FOUND'".format(pkg=pkg, prop=property))
            fail("ERROR: PKG '{pkg}' property '{prop}' NOT FOUND'".format(pkg=pkg, prop=property))
            return False
        elif has_property.stdout.strip() == value:
            # print("PKG '{pkg}' property '{prop}' matches {v}".format(
            #     pkg=pkg, prop=property, v=value
            # ))
            return True
        else:
            print("PKG '{pkg}' property '{prop}' mismatch: wanted '{v}', found '{pv}'".format(
                pkg=pkg, prop=property, v=value, pv=has_property.stdout.strip()
            ))
            return False
    else:
        print("ERROR on cmd 'opam config var {prop}".format(prop=prop))
        print("ERROR RC: {rc}".format(rc = has_property.return_code))
        print("ERROR STDOUT: {msg}".format(msg = has_property.stdout))
        print("ERROR STDERR: {msg}".format(msg = has_property.stderr))

# def opam_match_pinned_path(repo_ctx, pkg, path):

###########################################
def _get_pinned_path(rootpath, kind, path):
    if kind == "git":
        print("PROCESSING GIT PATH %s" % path)
        if path.startswith("git"):
            return path[4:]  # remove prefix "git+"
        else:
            print("ERROR Bad pin data: kind does not match path: {k} {p}".format(k=kind, p=path))
    elif kind == "rsync":
        print("PROCESSING RSYNC PATH %s" % path)
    else:
        print("UNKNOWN PIN KIND: %s" % kind)
        fail("ERROR: UNKNOWN PIN KIND '%s'" % kind)

##############################################################################
def _opam_pkg_for_pin_path(repo_ctx, rootpath, pinned_paths, name, pkg, spec):

    cfg_name = repo_ctx.execute(["opam", "config", "var", pkg + ":name"])
    if cfg_name.return_code == 0:
        print("MATCHED CFG_NAME: %s" % cfg_name.stdout.strip())
        cfg_version = repo_ctx.execute(["opam", "config", "var", pkg + ":version"])
        if cfg_version.return_code == 0:
            if cfg_version.stdout.strip() == spec[0]:
                print("MATCHED CFG_VERSION: %s" % cfg_version.stdout.strip())
                # now match path
                installed_path = _get_pinned_path(rootpath, pinned_paths[name][0], pinned_paths[name][1])
                print("INSTALLED_PATH: %s" % installed_path)
                if installed_path == spec[1]:
                    path_match = True
                    print("MATCHED PINNED PATH: {p}".format(
                        p = spec[1]
                        # k = pinned_paths[name][0], p = pinned_paths[name][1]
                    ))
                else:
                    path_match = False
                    print("MISMATCH PINNED PATH: {p} : {installed}".format(
                        p = spec[1], installed = installed_path
                    ))
            else:
                print("MISMATCHED CFG_VERSION: {v} != {cv}".format(
                    v = spec[0], cv = cfg_version.stdout.strip()
                ))
                version_mismatch = True
    else:
        ## THIS SHOULD NEVER HAPPEN
        print("MISMATCHED CFG_NAME: {n} : {cn}".format(n=pkg, cn=cfg_name.stdout.strip()))
        fail("ERROR mismatched cfg name should not happen")

    if path_match:
        # already correctly pinned
        is_ppx = is_ppx_driver(repo_ctx, pkg)
        result = "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = is_ppx )
    else:
        pkg_path = rootpath + "/" + spec[1] # path
        repo_ctx.report_progress("Pinning {p}.{version} to {path} (may take a while)...".format(
            p = pkg,
            version = spec[0],
            path = spec[1]
        ))

        fail("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
        ## FIXME: add --switch
        # pinout = repo_ctx.execute(["opam", "pin", "-v", "-y",
        #                            "add", name, pkg_path])

        # if pinout.return_code == 0:
        #     repo_ctx.report_progress("Pinned {path}.".format(path = spec[1]))
        #     is_ppx = is_ppx_driver(repo_ctx, pkg)
        #     result = "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = is_ppx )
        # else:
        #     print("ERROR opam pin rc: %s" % pinout.return_code)
        #     print("ERROR stdout: %s" % pinout.stdout)
        #     print("ERROR stderr: %s" % pinout.stderr)
        #     fail("OPAM pin add cmd failed")

    # return result
