load("@bazel_skylib//lib:collections.bzl", "collections")
load("opam_toolchain_xdg.bzl", "config_xdg_toolchain")
load("opam_toolchain_local.bzl", "config_local_toolchain")
load("opam_toolchain_global.bzl", "config_global_toolchain")
load("opam_dep.bzl", "opam_dep", "OBAZL_PKGS")
load("opam_repo.bzl", "opam_repo")
load("opam_ops.bzl",
     "opam_install_pkg",
     "print_cwd", "print_tree")

##############################
def _local_opam_repo_impl(repo_ctx):
    repo_ctx.file("dummy", content="")

local_opam_repo = repository_rule(
    implementation = _local_opam_repo_impl,
)

################################################################
def _build_config_tool(mctx, debug, verbosity):
    if debug > 1: print("_build_config_tool")
    bazel = mctx.which("bazel")
    # print("BAZEL: %s" % bazel)
    # print("CWD: %s" % print_cwd(mctx))

    # we have to turn this into a bazel repo
    # in order to run bazel in it:
    mctx.file("REPO.bazel", content = "")

    mctx.file(".bazelrc", content = """
common --registry=file:///Users/gar/.local/share/registry
common --registry=https://raw.githubusercontent.com/obazl/registry/main/
common --registry=https://bcr.bazel.build
# common --config=showpp
    """
              )

    # cmd = ["tree", "-aL", "1", "../../external"]
    # mctx.execute(cmd, quiet = False)

    mctx.file("MODULE.bazel",
              content = """
module(
    name = "config_tool",
    version = "0.0.0",
 )

bazel_dep(name = "tools_opam", repo_name="tools") # , version = "1.0.0")
local_path_override(
    module_name = "tools_opam",
    path = "../../external/tools_opam+")
bazel_dep(name = "rules_ocaml", version = "5.0.0")
local_path_override(
    module_name = "rules_ocaml",
    path = "../../external/rules_ocaml+")
               """
              )

    mctx.report_progress("Building @tools_opam//extensions/config")
    cmd = [bazel, "build", "@tools//extensions/config"]
    res = mctx.execute(cmd, quiet = (verbosity < 1))
    if res.return_code == 0:
        res = res.stdout.strip()
    else:
        print("cmd: %s" % cmd)
        print("rc: %s" % res.return_code)
        print("stdout: %s" % res.stdout)
        print("stderr: %s" % res.stderr)
        fail("cmd failure")

    # cmd = ["pwd"]
    # mctx.execute(cmd, quiet = False)
    # cmd = ["tree", "-a"]
    # mctx.execute(cmd, quiet = False)

    config_pkg_tool = mctx.path(".bazel/bin/external/tools_opam+/extensions/config/config")

    # this is the path on modextwd, we need the real path
    config_pkg_tool = config_pkg_tool.realpath

    return config_pkg_tool

##################################
def _throw_cmd_error(cmd, r):
    print("cmd: %s" % cmd)
    print("stdout: {stdout}".format(stdout= r.stdout))
    print("stderr: {stderr}".format(stderr= r.stderr))
    fail("cmd failure.")

# Set to false to see debug messages

##################################
def _ocamlfind_deps(mctx, ocamlfind, switch, pkg, version, debug, verbosity):
    if debug > 0: print("_ocamlfind_deps: %s" % pkg)

    cmd = [
        ocamlfind,
        "query",
        "-predicates",
        "ppx_driver",
        "-p-format",
        "-recursive",
        pkg
    ]
    res = mctx.execute(cmd, quiet = (verbosity < 1))

    if res.return_code == 0:
        deps = res.stdout.strip()
        deps = deps.splitlines()
        # print("ocamlfind query ok, deps: %s" % deps)
    elif res.return_code == 2:
        print("findlib pkg not found: " + pkg)
        return None
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
    # print("OPAM EXTENSION running")

    # get globals (version id etc). from root module
    opam_version  = None
    ocaml_version = "5.3.0"
    obazl_pfx     = "opam."
    toolchain     = None
    debug         = 0
    opam_verbosity    = 0
    verbosity     = 0
    root_module   = None
    direct_deps = []
    # subdeps = []

    for m in mctx.modules:
        if m.is_root:
            # print("ROOT mod: %s" % m.name)
            root_module = m
            for cfg in m.tags.deps:
                # env var overrides toolchain param
                ltc = mctx.getenv("OBAZL_USE_LOCAL_TC")
                if debug > 1: print("OBAZL LTC: %s" % ltc)
                if ltc:
                    toolchain = ltc
                    print("toolchain override: %s" % toolchain)
                else:
                    toolchain = cfg.toolchain
                opam_version = cfg.opam_version
                ocaml_version = cfg.ocaml_version
                direct_deps.extend(cfg.pkgs)
                # subdeps.extend(cfg.indirect_deps)
                debug      = cfg.debug
                opam_verbosity = cfg.opam_verbosity
                verbosity = cfg.verbosity
        else:
            for cfg in m.tags.deps:
                direct_deps.extend(cfg.pkgs)
        #         subdeps.extend(cfg.indirect_deps)
        #         if debug > 0:
        #             print("MOD: %s" % config.name)
        #             print("DEPS: %s" % config.direct_deps)
        #             print("SUBDEPS: %s" % config.indirect_deps)

    config_pkg_tool = _build_config_tool(mctx, debug, verbosity)
    if debug > 0: print("CONFIG TOOL: %s" % config_pkg_tool)

    # print_tree(mctx)
    # x = mctx.read("MODULE.bazel")
    # print(x)
    # fail("xxxxxxxxxxxxxxxxa")

    switch = None
    if toolchain == "local":
        (opam, opamroot, sdklib, switch,
         ocaml_version, deps) = config_local_toolchain(
            mctx,
            ocaml_version,
            direct_deps,
            debug, opam_verbosity, verbosity)
        if debug > 0: print("SDKLIB: %s" % sdklib)
    elif toolchain == "global":
        if debug > 0: print("TC GLOBAL")
        (opam, opamroot, sdklib, switch,
         ocaml_version, deps) = config_global_toolchain(
            mctx,
            ocaml_version,
            direct_deps,
            debug, opam_verbosity,
            verbosity)
        if debug > 0: print("SDKLIB: %s" % sdklib)
    elif toolchain == "xdg":
        if debug > 0: print("TC PRIVATE")
        (opam, opamroot, sdklib, switch,
         ocaml_version, deps) = config_xdg_toolchain(
             mctx,
             opam_version,
             ocaml_version,
             direct_deps,
             debug,
             opam_verbosity,
             verbosity)
    # else: # tc == "xdg_local"

    # if debug > 0: print(print_cwd(mctx))

    ## now create @opam
    # opam_repo(name = "opam",
    #           toolchain     = toolchain,
    #           # opam          = "//bin:opam",
    #           opam_version  = opam_version,
    #           # opam_root     = opamroot,
    #           ocaml_version = ocaml_version,
    #           pkgs          = deps,
    #           debug         = debug,
    #           verbosity     = verbosity)
    # print("registered repo @opam")

    opampath = str(opam)
    rootpath = str(opamroot)
    sdklib = str(sdklib)

    # always configure ocamlsdk & stublibs
    opam_dep(name="{}ocamlsdk".format(obazl_pfx),
             opam = opampath,
             switch = switch,
             root   = rootpath,
             sdklib = sdklib,
             ocaml_version = ocaml_version,
             obazl_pfx = obazl_pfx,
             config_tool = str(config_pkg_tool),
             debug = debug,
             verbosity = verbosity)
    opam_dep(name="{}stublibs".format(obazl_pfx),
             opam = opampath,
             switch = switch,
             root   = rootpath,
             sdklib = sdklib,
             ocaml_version = ocaml_version,
             obazl_pfx = obazl_pfx,
             config_tool = str(config_pkg_tool),
             debug = debug,
             verbosity = verbosity)

    ## configure all deps
    # for now, ignore versions
    # if verbosity > 0: print("\n\tRegistering repos for opam pkgs")
    for pkg in deps:
        ## FIXME: for ocaml >= 5, dynlink etc. not toplevel pkgs?
        if pkg in OBAZL_PKGS: # e.g. dynlink, str, unix
            pkg = pkg
        else:
            pkg = "{pfx}{pkg}".format(pfx=obazl_pfx, pkg=pkg)
        if verbosity > 0: print("\n  Registering repo: " + pkg)
        opam_dep(name=pkg,
                 install = False,
                 opam = opampath,
                 switch = switch,
                 root   = rootpath,
                 sdklib = sdklib,
                 ocaml_version = ocaml_version,
                 obazl_pfx = obazl_pfx,
                 config_tool = str(config_pkg_tool),
                 debug = debug,
                 verbosity = verbosity
                 )
        # if debug > 1: print("done")

    ## installing deps already installs subdeps
    ## so we create repo & config w/o installing opam pkg
    ## FIXME: this install is only for hermetic tc
    ## for local tc they're already installed
    # for pkg in newsubdeps:
    #     if pkg in OBAZL_PKGS:
    #         pkg = pkg
    #     else:
    #         pkg = "{pfx}{pkg}".format(pfx=obazl_pfx, pkg=pkg)
    #     if debug > 1: print("creating repo for: " + pkg)
    #     # if pkg == "stublibs": # special case
    #     #     install = True
    #     # else:
    #     install = False
    #     opam_dep(name=pkg,
    #              install = install,
    #              opam = opampath,
    #              switch = switch,
    #              root   = rootpath,
    #              sdklib = sdklib,
    #              ocaml_version = ocaml_version,
    #              obazl_pfx = obazl_pfx,
    #              config_tool = str(config_pkg_tool),
    #              debug = debug,
    #              verbosity = verbosity
    #              )
    #     if debug > 1: print("done")

    # opam_dep(name="{}ocamlfind".format(obazl_pfx),
    #          opam = opampath,
    #          switch = switch,
    #          root   = rootpath,
    #          ocaml_version = ocaml_version,
    #          obazl_pfx = obazl_pfx,
    #          config_tool = str(config_pkg_tool),
    #          debug = debug,
    #          verbosity = verbosity)
    # opam_dep(name="{}findlib".format(obazl_pfx),
    #          opam = opampath,
    #          switch = switch,
    #          root   = rootpath,
    #          ocaml_version = ocaml_version,
    #          obazl_pfx = obazl_pfx,
    #          config_tool = str(config_pkg_tool),
    #          debug = debug,
    #          verbosity = verbosity)

    rmdirects = ["opam.ocamlsdk"]
    for dep in direct_deps:
        rmdirects.append("{}{}".format(obazl_pfx, dep))
    return mctx.extension_metadata(
        root_module_direct_deps = rmdirects,
        root_module_direct_dev_deps = []
    )

##############################
opam = module_extension(
  implementation = _opam_ext_impl,
  tag_classes = {
      "deps": tag_class(
          attrs = {
              "toolchain": attr.string(
                  mandatory = False,
                  default = "xdg",
                  values  = ["local", "global", "xdg",
                             # future: "xdg_local"
                             ]
              ),
              "opam_version": attr.string(
                  mandatory = False,
                  default = "2.3.0"
              ),
              "ocaml_version": attr.string(
                  mandatory = False
              ),
    # or:
    # xdg_switch = {"id": "ocaml_version"}

              "pkgs": attr.string_dict(
                  mandatory = True,
                  allow_empty = False
              ),
              "dev_deps": attr.string_dict(
                  mandatory = False,
                  allow_empty = True
              ),
              "debug"     : attr.int(default = 0),
              "verbosity" : attr.int(default = 0),
              "opam_verbosity": attr.int(default = 0),
          }
      )
  }
)
