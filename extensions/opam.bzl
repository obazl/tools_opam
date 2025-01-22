load("@bazel_skylib//lib:collections.bzl", "collections")

##################################
def _throw_cmd_error(cmd, r):
    # print("cmd {cmd} rc    : {rc}".format(cmd=cmd, rc= r.return_code))
    print("stdout: {stdout}".format(cmd=cmd, stdout= r.stdout))
    print("stderr: {stderr}".format(cmd=cmd, stderr= r.stderr))
    fail("cmd failure.")

# Set to false to see debug messages
DEBUG_QUIET = True

##################################
def _get_deps(mctx, ocamlfind, pkg, version):
    # ocamlfind query -predicates ppx_driver -p-format -recursive $D
    # print("get deps for: " + pkg)
    cmd = [
        ocamlfind,
        "query",
        "-predicates",
        "-ppx_driver",
        "-p-format",
        "-recursive",
        pkg
    ]
    deps = mctx.execute(cmd, quiet = DEBUG_QUIET)

    if deps.return_code == 0:
        deps = deps.stdout.strip()
        deps = deps.splitlines()
        # print("ocamlfind query ok, deps: %s" % deps)
    # elif deps.return_code == 2:
    #     print("ocamlfind query stdout: %s" % deps.stdout)
    #     print("ocamlfind query stderr: %s" % deps.stderr)
    #     fail("findlib pkg not found: " + pkg)
    else:
        _throw_cmd_error(cmd, deps)

    return deps

##############################
def _opam_dep_repo_impl(repo_ctx):
    ## repo_cts.name == tools_opam++opam+opam.ounit2
    # print("REPO " + repo_ctx.name)
    repo = repo_ctx.name.removeprefix("tools_opam++opam+")
    repo_pkg = repo.removeprefix("opam.")

    # print("REPO {} TOOL: {}".format(repo_pkg, repo_ctx.attr.tool))
    # if repo_pkg == "ocamlsdk":
    # print("OPAM pkg: " + repo_pkg)

    # cmd = ["opam", "var", "bin"]
    # cmd = ["PWD"]
    # switch_bin = repo_ctx.execute(cmd)
    # if switch_bin.return_code == 0:
    #     switch_bin = switch_bin.stdout.strip()
    #     # print("opam prefix: %s" % switch_bin)
    # elif switch_bin.return_code == 5: # Not found
    #     fail("OPAM cmd {cmd} switch_bin: not found.".format(cmd = cmd))
    # else:
    #     _throw_cmd_error(cmd, switch_bin)

    # print("pwd: " + switch_bin)
    # ocamlfind = switch_bin + "/ocamlfind"

    # repo = repo_ctx.name.removeprefix("+_repo_rules+")
    # repo_pkg = repo.removeprefix("opam.")

    # print("opam pfx: " + switch_pfx)
    # check: installed?
    # if repo_pkg not in ["ocamlsdk",
    #                 "compiler-libs",
    #                 "dynlink",
    #                 "ffi",
    #                 "ocamldoc",
    #                 "profiling",
    #                 "runtime_events",
    #                 "stdlib",
    #                 "str",
    #                 "threads",
    #                 "unix",
    #                 "findlib",
    #                 "stublibs"]:
    #     cmd = ["opam", "var", repo_pkg + ":installed"]
    #     opam_installed = repo_ctx.execute(cmd)
    #     if opam_installed.return_code == 0:
    #         opam_installed = opam_installed.stdout.strip()
    #         if opam_installed == "false":
    #             fail("Requested pkg %s is not installed" % repo_pkg)

    # check version
    if False: # repo_ctx.attr.version:
        # print("PKG %s %s" %
        #       (repo, repo_ctx.attr.version))

        cmd = ["opam", "var", repo_pkg + ":version"]
        opam_version = repo_ctx.execute(cmd)
        if opam_version.return_code == 0:
            opam_version = opam_version.stdout.strip()
            # print("%s installed version: %s" %
            #   (repo, opam_version))
            if opam_version != repo_ctx.attr.version:
                fail("Requested version of %s is %s, but installed version is %s" %
                     (repo_pkg, repo_ctx.attr.version, opam_version))
        elif opam_version.return_code == 5: # Not found
            fail("OPAM cmd {cmd} opam_version: not found.".format(cmd = cmd))
        else:
            fail("OPAM dependency not found: %s.%s" %
              (repo_pkg, repo_ctx.attr.version))

        # opam_path = switch_pfx + "/.opam-switch/packages/" + repo + "." + repo_ctx.attr.version
        # print("path: " + opam_path)
        # pkg_path = repo_ctx.path(opam_path)
        # if pkg_path.is_dir:
        #     print("Found: " + opam_path)
        # else:
        #     print("Not found: " + opam_path)
        #     fail("OPAM dependency not found: %s.%s" %
        #       (repo, repo_ctx.attr.version))

    ## Running repo_ctx.execute cmd creates the repo
    ## so the ensuing symlink would fail with 'already exists'
    ## so we delete it before creating the symlink:

    repo_ctx.delete(".")

    # cmd = ["pwd"]
    # result = repo_ctx.execute(cmd)
    # if result.return_code == 0:
    #     result = result.stdout.strip()
    #     print("PWD1: %s" % result)
    # else:
    #     _throw_cmd_error(cmd, result)


    # tgt =   "{pfx}/../.config/obazl/lib/{repo}/".format(

    # tgt = "{pfx}/share/obazl/repository/lib/{repo}/".format(
    #     pfx=repo_ctx.attr.switch_pfx,
    #     repo=repo)
    # print("SYMLINKING: " + tgt)
    # repo_ctx.symlink(tgt, repo_ctx.name)

    # print("BUILD_WORKSPACE_DIRECTORY: %s" %
    #       repo_ctx.getenv("BUILD_WORKSPACE_DIRECTORY", "NO BWSD"))
    # print("RUNFILES_DIR: %s" %
    #       repo_ctx.getenv("RUNFILES_DIR", "NO RFD"))
    # print("RUNFILES_MANIFEST_FILE: %s" %
    #       repo_ctx.getenv("RUNFILES_MANIFEST_FILE", "NO RFMF"))

    # cmd = ["ls", "-l", "../tools_opam+/templates/"]
    # result = repo_ctx.execute(cmd)
    # if result.return_code == 0:
    #     result = result.stdout.strip()
    #     # print("tools_opam+: %s" % result)
    # else:
    #     _throw_cmd_error(cmd, result)


    cmd = [repo_ctx.attr.tool,
           "-ddt",
           "--pkg", repo_pkg,
           "--switch", repo_ctx.attr.switch_id]
    _pkg_deps = repo_ctx.execute(cmd)
    if _pkg_deps.return_code == 0:
        _pkg_deps = _pkg_deps.stdout.strip()
        # print("pkg {} deps: {}".format(repo_pkg, _pkg_deps))
    else:
        print("cmd {cmd} rc    : {rc}".format(
            cmd=cmd,
            rc= _pkg_deps.return_code))
        _throw_cmd_error(cmd, _pkg_deps)

## for use with use_repo_rule
opam_dep = repository_rule(
    implementation = _opam_dep_repo_impl,
    attrs = {
        "switch_id": attr.string(mandatory = True),
        "switch_pfx": attr.string(mandatory = True),
        "switch_lib": attr.string(mandatory = True),
        "version": attr.string(
            mandatory = False,
        ),
        "tool": attr.string()
    },
)

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
    # for each module in the depgraph that calls this extension

    # print("bwsd: " + mctx.getenv("BUILD_WORKSPACE_DIRECTORY",
    #                              ""))

    # for m in mctx.modules:
    #     print(m.name)
    #     print("is root? %s" % m.is_root)
    #     if m.is_root:
    #         print("root tags")
    #         for cfg in m.tags.dep:
    #             print(cfg)

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

    # cmd = ["ls", "-l", "../../execroot/_main/_opam"]
    cmd = ["realpath", "../../execroot/_main/_opam"]
    result = mctx.execute(cmd)
    if result.return_code == 0:
        cmd = ["readlink", "-f", "../../execroot/_main"]
        result = mctx.execute(cmd)
        if result.return_code == 0:
            local_opam = result.stdout.strip()
        else:
            _throw_cmd_error(cmd, result)
    else:
        local_opam = None


    cmd = ["opam", "var", "prefix"]
    if local_opam:
        cmd.extend(["--switch", local_opam])
           # "/Users/gar/obazl/demos_obazl/rules_ocaml"]
    switch_pfx = mctx.execute(cmd)
    if switch_pfx.return_code == 0:
        switch_pfx = switch_pfx.stdout.strip()
    else:
        _throw_cmd_error(cmd, switch_pfx)

    # print("opam pfx: " + switch_pfx)

    cmd = ["opam", "var", "bin"]
    if local_opam:
        cmd.extend(["--switch", local_opam])
    # print("cmd: %s" % cmd)
    switch_bin = mctx.execute(cmd)
    if switch_bin.return_code == 0:
        switch_bin = switch_bin.stdout.strip()
    else:
        _throw_cmd_error(cmd, switch_bin)

    # print("opam bin: " + switch_bin)
    ocamlfind = switch_bin + "/ocamlfind"
    ## FIXME: verify ocamlfind installed
    ## else install it.

    cmd = ["opam", "var", "lib"]
    if local_opam:
        cmd.extend(["--switch", local_opam])
    # print("cmd: %s" % cmd)
    switch_lib = mctx.execute(cmd)
    if switch_lib.return_code == 0:
        switch_lib = switch_lib.stdout.strip()
    else:
        _throw_cmd_error(cmd, switch_lib)

    # deps = set()
    deps = []
    for mod in mctx.modules:
        # create repo for each dep
        for config in mod.tags.dep:
            config_pkg_tool = mctx.which(config._tool.name)
            # print("TOOL: %s" % config_pkg_tool)
            # if mod.is_root:
            #     print("_loc: %s" %
            #           config._loc.workspace_root)
            #     cmd = ["ls", "-l"]
            #     result = mctx.execute(cmd)
            #     if result.return_code == 0:
            #         result = result.stdout.strip()
            #         print("loc ls: %s" % result)
            #     else:
            #         _throw_cmd_error(cmd, result)

            for k, v in config.pkgs.items():
                # print("CONFIG.name: " + k)
                if k not in ["findlib",
                             "ocamlsdk",
                             "stublibs",
                             "threads"]:
                    # deps.update(_get_deps(mctx, k, v))
                    deps.extend(_get_deps(mctx, ocamlfind, k, v))
                    # print("DEPS %s" % deps)
                else:
                    # print("ADDING DEP " + k)
                    opam_dep(name="{}".format(k),
                             switch_id = local_opam,
                             switch_pfx=switch_pfx,
                             switch_lib=switch_lib,
                             tool = str(config_pkg_tool),
                             version = v)

    deps = collections.uniq(deps)
    newdeps = []
    for dep in deps:
        (before, sep, after) = dep.partition(".")
        # deps.remove(dep)
        # deps.add(before)
        newdeps.append(before)
    newdeps = collections.uniq(newdeps)
    newdeps = sorted(newdeps)
    print("ALL DEPS: %s" % newdeps)
    # for now, ignore versions
    for pkg in newdeps:
        # print("creating repo for: " + pkg)
        opam_dep(name="opam.{}".format(pkg),
                 switch_id = local_opam,
                 switch_pfx=switch_pfx,
                 switch_lib=switch_lib,
                 tool = str(config_pkg_tool))
                 #          version = v)
    return mctx.extension_metadata(
        root_module_direct_deps = "all",
        root_module_direct_dev_deps = []
    )

##############################
opam = module_extension(
  implementation = _opam_ext_impl,
  tag_classes = {"dep": tag_class(
      attrs = {
          "pkgs": attr.string_dict(
              mandatory = True,
              allow_empty = False
          ),
          "_tool": attr.label(
              default = "@@//bin:obazl_config_opam_pkg")
          # "name": attr.string(),
          # "version": attr.string(),
      }
  )}
)

