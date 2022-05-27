
##################################
def _throw_opam_cmd_error(cmd, r):
    print("OPAM cmd {cmd} rc    : {rc}".format(cmd=cmd, rc= r.return_code))
    print("OPAM cmd {cmd} stdout: {stdout}".format(cmd=cmd, stdout= r.stdout))
    print("OPAM cmd {cmd} stderr: {stderr}".format(cmd=cmd, stderr= r.stderr))
    fail("OPAM cmd failure.")

################################
def run_opam_cmd(repo_ctx, cmd):

    result = repo_ctx.execute(cmd)
    if result.return_code == 0:
        result = result.stdout.strip()
        # print("_opam_set_switch result ok: %s" % result)
    elif result.return_code == 5: # Not found
        fail("OPAM cmd {cmd} result: not found.".format(cmd = cmd))
    else:
        _throw_opam_cmd_error(cmd, result)

    return result

#############################################
def opam_get_current_switch_prefix(repo_ctx):
    cmd = ["opam", "var", "prefix"]
    return run_opam_cmd(repo_ctx, cmd)

#############################################
def opam_get_current_switch_libdir(repo_ctx):
    cmd = ["opam", "var", "lib"]
    return run_opam_cmd(repo_ctx, cmd)

