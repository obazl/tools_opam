DEFAULT_PATH = "/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/usr/sbin"

#####################
def print_cwd(ctx):
    cmd = ["pwd"]
    res = ctx.execute(cmd, quiet = False)
    if res.return_code == 0:
        res = res.stdout.strip()
        return res
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

###############@######
def print_tree(ctx, dir=".", depth=1):
    cmd = ["tree"]
    res = ctx.execute(cmd, quiet = False)
    if res.return_code == 0:
        res = res.stdout.strip()
        return res
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

#####################
def file_exists(ctx, f, debug=0, verbosity=0):
    cmd = ["file", "-E", "-b", "{}".format(f)]
    res = ctx.execute(cmd, quiet = (verbosity < 1))
    if res.return_code == 0:
        return True
    elif res.return_code == 1:
        return False
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

#####################
def run_cmd(ctx, cmd, debug=0, verbosity=0):
    #print("RUNCMD: %s" % cmd)
    res = ctx.execute(cmd, quiet = (verbosity < 1))
    if res.return_code == 0:
        res = res.stdout.strip()
        return res
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

################
def is_pkg_installed(mctx, opambin, pkg,
                      OPAMROOT, ocaml_version):
    pkg_path = "{}/{}/lib/{}".format(
        OPAMROOT, ocaml_version, pkg)

    if file_exists(mctx, pkg_path):
        return True
    else:
        return False

###########################
def opam_install_pkg(rctx,
                     opam_path,
                     pkg,
                     switch,
                     switch_pfx,
                     sdk_bin,
                     root,
                     n, tot,
                     debug, verbosity):

    # print("installing, root = %s" % root)
    cmd = [opam_path,
           "install",
           pkg,
           "--switch", switch,
           "--root", "{}".format(root),
           "--yes"]

    if debug > 0: print("Installing pkg: %s" % cmd)
    rctx.report_progress("Installing pkg {p} ({i} of {tot})".format(p=pkg, i=n, tot=tot))
    res = rctx.execute(cmd,
                       environment = {
                           "PATH":  "{}:{}/bin:{}".format(
                               sdk_bin, switch_pfx, DEFAULT_PATH)
                       },
                       quiet = (verbosity < 1))
    if res.return_code == 0:
        if debug > 0: print("pkg installed: '%s'" % pkg)
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

