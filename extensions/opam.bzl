load("@bazel_skylib//lib:collections.bzl", "collections")
load("opam_download_toolchain.bzl",
     "download_and_config_toolchain")
load("opam_deps.bzl", "opam_dep", "OBAZL_PKGS")

##############################
def _local_opam_repo_impl(repo_ctx):
    repo_ctx.file("dummy", content="")

local_opam_repo = repository_rule(
    implementation = _local_opam_repo_impl,
)

###################################################
def config_local_toolchain(mctx, debug, verbosity):
    if debug > 0: print("config_local_toolchain")

    opam = mctx.which("opam")
    if debug > 0: print("OPAM: %s" % opam)

    cmd = ["realpath", "../../execroot/_main/_opam"]
    res = mctx.execute(cmd)
    if res.return_code == 0:
        cmd = ["readlink", "-f", "../../execroot/_main"]
        result = mctx.execute(cmd)
        if res.return_code == 0:
            switch = result.stdout.strip()
        else:
            print("cmd: %s" % cmd)
            print("stdout: {stdout}".format(stdout= res.stdout))
            print("stderr: {stderr}".format(stderr= res.stderr))
            fail("cmd failure.")
    else:
        cmd = [opam, "var", "switch"]
        res = mctx.execute(cmd)
        if res.return_code == 0:
            switch = res.stdout.strip()
        else:
            print("cmd: %s" % cmd)
            print("stdout: {stdout}".format(stdout= res.stdout))
            print("stderr: {stderr}".format(stderr= res.stderr))
            fail("cmd failure.")

    if debug > 1:
        print("switch: %s" % switch)

    cmd = [opam, "var", "bin", "--switch", switch]
    res = mctx.execute(cmd)
    if res.return_code == 0:
        switch_bin = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("stdout: {stdout}".format(stdout= res.stdout))
        print("stderr: {stderr}".format(stderr= res.stderr))
        fail("cmd failure.")

    ocamlfind = mctx.path(switch_bin + "/ocamlfind")
    if not ocamlfind.exists:
        mctx.report_progress("Installing ocamlfind")
        cmd = [opam, "install", "ocamlfind", "--switch", switch]
        res = mctx.execute(cmd)
        if res.return_code != 0:
            print("cmd: %s" % cmd)
            print("stdout: {stdout}".format(stdout= res.stdout))
            print("stderr: {stderr}".format(stderr= res.stderr))
            fail("cmd failure.")

    if ocamlfind.exists:
        if debug > 1: print("ocamlfind: %s" % ocamlfind)
    else:
        fail("ocamlfind installation failed.")

    # downloaded sdk creates @opam and needs a use_repo for it
    # to prevent warning "Imported but not created" we need
    # to create it (as a dummy repo).
    local_opam_repo(name="opam")

    return (opam, switch, ocamlfind)

##################################
def _throw_cmd_error(cmd, r):
    print("cmd: %s" % cmd)
    print("stdout: {stdout}".format(stdout= r.stdout))
    print("stderr: {stderr}".format(stderr= r.stderr))
    fail("cmd failure.")

# Set to false to see debug messages

##################################
def _ocamlfind_deps(mctx, ocamlfind, pkg, version, debug, verbosity):
    if debug > 0: print("_ocamlfind_deps: %s" % pkg)

    

    cmd = [
        ocamlfind,
        "query",
        "-predicates",
        "-ppx_driver",
        "-p-format",
        "-recursive",
        pkg
    ]
    res = mctx.execute(cmd, quiet = (verbosity < 1))

    if res.return_code == 0:
        deps = res.stdout.strip()
        deps = deps.splitlines()
        # print("ocamlfind query ok, deps: %s" % deps)
    # elif deps.return_code == 2:
    #     print("ocamlfind query stdout: %s" % deps.stdout)
    #     print("ocamlfind query stderr: %s" % deps.stderr)
    #     fail("findlib pkg not found: " + pkg)
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

    return deps

################################################################
## extension
##  use:
##   bazel_dep(name = "tools_opam", version = "...")
##   opam = use_extension("@tools_opam//extensions:opam.bzl")
##   opam.dep(name = "ounit2", version = "...")
##   use_repo(opam, "opam.ounit2")
##
## here 'opam.dep' is a "tag function" that calls a repo
## rule to create repo @opam.ounit2

#### EXTENSION IMPL ####
def _opam_ext_impl(mctx):
    # print("OPAM EXTENSION")

    # get version ids etc. from root module
    opam_version = None
    ocaml_version = None
    local_tc = False
    debug = 0
    verbosity = 0
    root_module = None
    for m in mctx.modules:
        if m.is_root:
            root_module = m
            for cfg in m.tags.deps:
                local_tc = cfg.local_toolchain
                opam_version = cfg.opam_version
                ocaml_version = cfg.ocaml_version
                debug  = cfg.debug
                verbosity = cfg.verbosity
    if debug > 1:
        print("local tc? %s" % local_tc)

    if local_tc:
        (opam, switch, ocamlfind) = config_local_toolchain(mctx, debug, verbosity)
        xopam = None # str(opam)
    else:
        download_and_config_toolchain(mctx,
                                      opam_version,
                                      ocaml_version,
                                      debug,
                                      verbosity)
        xopam = "@opam//bin:opam"

    # cmd = ["pwd"]
    # result = mctx.execute(cmd)
    # if result.return_code == 0:
    #     result = result.stdout.strip()
    #     print("PWD: %s" % result)
    # else:
    #     _throw_cmd_error(cmd, result)

    # cmd = ["ls", "-l", ".."]
    # result = mctx.execute(cmd)
    # if result.return_code == 0:
    #     result = result.stdout.strip()
    #     print(result)
    # else:
    #     _throw_cmd_error(cmd, result)

    # deps = set()
    deps = []
    subdeps = []
    for mod in mctx.modules:
        # create repo for each dep
        for config in mod.tags.deps:
            config_pkg_tool = mctx.which(config._tool.name)
            for pkg, val in config.direct_deps.items():
                if debug > 1: print("pkg name: " + pkg)
                deps.append(pkg)
                if local_tc:
                    # always derive subdeps, ensures completeness
                    if pkg not in OBAZL_PKGS:
                        subdeps.extend(_ocamlfind_deps(mctx, ocamlfind, pkg, val, debug, verbosity))
            subdeps.extend(config.indirect_deps.keys())

    deps = collections.uniq(deps)
    newdeps = []
    for dep in deps:
        (before, sep, after) = dep.partition(".")
        # deps.remove(dep)
        # deps.add(before)
        newdeps.append(before)
    newdeps = collections.uniq(newdeps)
    newdeps = sorted(newdeps)
    if debug > 1: print("ALL DEPS: %s" % newdeps)

    if debug > 1: print("ALL SUBDEPS: %s" % subdeps)
    subdeps = collections.uniq(subdeps)
    newsubdeps = []
    for dep in subdeps:
        (before, sep, after) = dep.partition(".")
        # deps.remove(dep)
        # deps.add(before)
        if before not in OBAZL_PKGS:
            if before not in newdeps:
                newsubdeps.append(before)
    newsubdeps = collections.uniq(newsubdeps)
    newsubdeps = sorted(newsubdeps)
    if debug > 1: print("ALL NEWSUBDEPS: %s" % newsubdeps)

    # for now, ignore versions
    for pkg in newdeps:
        if pkg in OBAZL_PKGS:
            pkg = pkg
        else:
            pkg = "opam.{}".format(pkg)
        if debug > 1: print("creating repo for: " + pkg)
        opam_dep(name=pkg,
                 xopam = xopam,
                 ocaml_version = ocaml_version,
                 # switch_id = switch,
                 # switch_pfx=switch_pfx,
                 # switch_lib=switch_lib,
                 tool = str(config_pkg_tool),
                 debug = debug,
                 verbosity = verbosity
                 )
        if debug > 1: print("done")

    ## installing deps already installs subdeps
    ## so we create repo & config w/o installing opam pkg
    for pkg in newsubdeps:
        if pkg in OBAZL_PKGS:
            pkg = pkg
        else:
            pkg = "opam.{}".format(pkg)
        if debug > 1: print("creating repo for: " + pkg)
        # if pkg == "stublibs": # special case
        #     install = True
        # else:
        install = False
        opam_dep(name=pkg,
                 install = install,
                 xopam = xopam,
                 ocaml_version = ocaml_version,
                 # switch_id = switch,
                 # switch_pfx=switch_pfx,
                 # switch_lib=switch_lib,
                 tool = str(config_pkg_tool),
                 debug = debug,
                 verbosity = verbosity
                 )
        if debug > 1: print("done")

    # always configure ocamlsdk & stublibs
    opam_dep(name="ocamlsdk",
             xopam = xopam,
             ocaml_version = ocaml_version,
             # switch_id = switch,
             # switch_pfx=switch_pfx,
             # switch_lib=switch_lib,
                 tool = str(config_pkg_tool),
             debug = debug,
             verbosity = verbosity)
    opam_dep(name="opam.stublibs",
             xopam = xopam,
             ocaml_version = ocaml_version,
             # switch_id = switch,
             # switch_pfx=switch_pfx,
             # switch_lib=switch_lib,
                 tool = str(config_pkg_tool),
             debug = debug,
             verbosity = verbosity)

    return mctx.extension_metadata(
        root_module_direct_deps = "all",
        root_module_direct_dev_deps = []
    )

##############################
opam = module_extension(
  implementation = _opam_ext_impl,
  tag_classes = {
      "deps": tag_class(
          attrs = {
              "opam_version": attr.string(
                  default = "2.3.0"
              ),
              "ocaml_version": attr.string(
                  default = "5.3.0"
              ),
              "direct_deps": attr.string_dict(
                  mandatory = True,
                  allow_empty = False
              ),
              "indirect_deps": attr.string_dict(
                  mandatory = True,
                  allow_empty = False
              ),
              "local_toolchain": attr.bool(
                  default = False
              ),
              "_tool": attr.label(
                  default = "@@//bin:obazl_config_opam_pkg"),
              "debug": attr.int(default=0),
              "verbosity": attr.int(default=0)
          }
      )
  }
)

