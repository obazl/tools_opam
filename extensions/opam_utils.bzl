def get_sdk_lib(ctx, opambin, switch_pfx, debug):
    if debug > 0: print("\nSWITCH PFX: %s" % switch_pfx)
    cfgpath = str(switch_pfx) + "/.opam-switch/config/ocaml-system.config"
    # print("\nCFGPATH %s" % cfgpath)
    sysconfig = ctx.path(cfgpath)
    if sysconfig.exists:
        if debug > 0: print("\nFOUND ocaml-system.config")
        cmd = [opambin, "exec", "ocamlc", "--", "-where"]
        res = ctx.execute(cmd)
        if res.return_code == 0:
            SDKLIB = res.stdout.strip().removesuffix("/ocaml")
        else:
            print("cmd: %s" % cmd)
            print("stdout: {stdout}".format(stdout= res.stdout))
            print("stderr: {stderr}".format(stderr= res.stderr))
            fail("cmd failure.")
    else:
        if debug > 0: print("\nNOT FOUND ocaml-system.config")
        SDKLIB = str(switch_pfx) + "/lib"

    if debug > 0: print("\nSDKLIB: %s" % SDKLIB)
    return SDKLIB
