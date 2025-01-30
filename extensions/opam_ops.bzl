def opam_install_pkg(ctx, pkg, switch, debug, verbosity):
    opam = ctx.which("opam")
    cmd = [opam,
           "install",
           pkg,
           "--switch", switch,
           "--yes"]

    ctx.report_progress("Installing pkg %s" % pkg)
    res = ctx.execute(cmd, quiet = (verbosity < 1))
    if res.return_code == 0:
        if debug > 0:
            print("pkg installed: %s" % pkg)
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

##########################################
def opam_create_local_switch(ctx, opambin,
                             ocaml_version,
                             debug, verbosity):

    cmd = ["readlink", "-f", "../../execroot/_main/MODULE.bazel"]
    res = ctx.execute(cmd)
    if res.return_code == 0:
        proj_module = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("stdout: {stdout}".format(stdout= res.stdout))
        print("stderr: {stderr}".format(stderr= res.stderr))
        fail("cmd failure.")

    proj_module_path = ctx.path(proj_module)
    proj_root = proj_module_path.dirname
    if debug > 0: print("PROJ ROOT: %s" % proj_root)

    switch_id = proj_root

    cmd = [opambin, "switch",
           "create", switch_id, ocaml_version,
           ]
    # if root:
    #     cmd.extend(["--root={}".format(root)])
    if verbosity > 2:
        cmd.extend(["--verbose"])
    if debug > 0:
        print("Creating switch:\n%s" % cmd)
    ctx.report_progress("Creating local switch for compiler {version}, at {id}".format(
        id=switch_id, version=ocaml_version))
    res = ctx.execute(cmd, quiet = (verbosity < 1))
    if res.return_code != 0:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure: %s" % cmd)

    return switch_id
