load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository") # buildifier: disable=load
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")  # buildifier: disable=load

load("@bazel_skylib//rules:common_settings.bzl", "BuildSettingInfo")
load("@bazel_skylib//lib:collections.bzl", "collections")
load("@bazel_skylib//lib:types.bzl", "types")

load("@obazl_tools_bazel//tools/functions:strings.bzl", "tokenize")

load(":hermetic.bzl", "opam_repo_hermetic")

load(":ppx.bzl", "is_ppx_driver")

# 4.07.1 broken on XCode 12:
# https://discuss.ocaml.org/t/ocaml-4-07-1-fails-to-build-with-apple-xcode-12/6441/15
# OCAML_VERSION = "4.08.0"
# OCAMLBUILD_VERSION = "0.14.0"
# OCAMLFIND_VERSION = "1.8.1"
# COMPILER_NAME = "ocaml-base-compiler.%s" % OCAML_VERSION
# OPAM_ROOT_DIR = ".opam_root_dir"
# # Set to false to see debug messages
# DEBUG_QUIET = False

g_switch_name = ""

################################################################
def _config_opam_pkgs(repo_ctx):
    repo_ctx.report_progress("configuring OPAM pkgs...")

    ## FIXME: packages distibuted with the compiler can be hardcoded?
    ## e.g. compiler-libs.common
    ## but then we would have to keep a list for each compiler version...

    # print("Switch name: %s" % repo_ctx.attr.switch_name)
    # print("Switch compiler: %s" % repo_ctx.attr.switch_compiler)

    if "OBAZL_SWITCH" in repo_ctx.os.environ:
        print("OBAZL_SWITCH = %s" % repo_ctx.os.environ["OBAZL_SWITCH"])
        g_switch_name = repo_ctx.os.environ["OBAZL_SWITCH"]
        print("Using '{s}' from OBAZL_SWITCH env var.".format(s = g_switch_name))
        env_switch = True
    else:
        g_switch_name = repo_ctx.attr.switch_name  # + "-" + repo_ctx.attr.switch_version
        env_switch = False

    result = repo_ctx.execute(["opam", "switch", g_switch_name])
    if result.return_code == 5: # Not found
        if env_switch:
            repo_ctx.report_progress("SWITCH {s} from env var OBAZL_SWITCH not found.".format(s = g_switch_name))
            fail("\n\nERROR: OBAZL_SWITCH name '{s}' not found. To create a new switch, either do so from the command line or configure the switch in the opam config file (by convention, \"bzl/opam.bzl\").\n\n".format(s = g_switch_name))
        else:
            repo_ctx.report_progress("SWITCH {s} not found; creating".format(s = g_switch_name))
            print("SWITCH {s} not found; creating".format(s = g_switch_name))
            result = repo_ctx.execute(["opam", "switch", "create", g_switch_name, "--empty"])
            if result.return_code == 0:
                repo_ctx.report_progress("SWITCH {s} created. Installing {c} (may take a while)".format(
                    s = g_switch_name, c = "ocaml-base-compiler." + repo_ctx.attr.switch_compiler
                ))
            else:
                print("SWITCH CREATE ERROR: %s" % result.return_code)
                print("SWITCH CREATE STDERR: %s" % result.stderr)
                print("SWITCH CREATE STDOUT: %s" % result.stdout)
                return
            result = repo_ctx.execute(["opam", "install", "-y", "--switch=" + g_switch_name,
                                       "ocaml-base-compiler." + repo_ctx.attr.switch_compiler])
            if result.return_code == 0:
                repo_ctx.report_progress("SWITCH {s}: {c} installed.".format(
                    s = g_switch_name, c = "ocaml-base-compiler." + repo_ctx.attr.switch_compiler
                ))
            else:
                print("SWITCH CREATE ERROR: %s" % result.return_code)
                print("SWITCH CREATE STDERR: %s" % result.stderr)
                print("SWITCH CREATE STDOUT: %s" % result.stdout)
                return
    elif result.return_code != 0:
        print("SWITCH RC: %s" % result.return_code)
        print("SWITCH STDERR: %s" % result.stderr)
        print("SWITCH STDOUT: %s" % result.stdout)
        return

    # fetch and parse list of installed opam pkgs
    opam_pkg_list = repo_ctx.execute(["opam", "list"]).stdout
    # print("OPAM_PKG_LIST: %s" % opam_pkg_list)

    ## WARNING: because Bazel parallelizes actions, there is no
    ## guarantee that the following pin list action will occur before
    ## pinning actions below. So it does not necessarily tell us what
    ## was pinned before we started.
    # opam_pin_list = repo_ctx.execute(["opam", "pin", "list"]).stdout
    # print("OPAM_PIN_LIST: %s" % opam_pin_list)

    opam_pkg_list = opam_pkg_list.splitlines()
    opam_pkgs     = {}
    missing       = {}
    bad_version   = {}

    for pkg_desc in opam_pkg_list:
        if not pkg_desc.startswith("#"):
            [pkg, sep, rest] = pkg_desc.partition(" ")
            pkg = pkg.strip(" ")
            rest = rest.strip(" ")
            [version, sep, rest] = rest.partition(" ")
            version = version.strip(" ")
            opam_pkgs[pkg] = version

    opam_pkg_rules = []
    repo_ctx.report_progress("constructing OPAM pkg rules...")
    for [pkg, version] in repo_ctx.attr.opam_pkgs.items():
        # print("Pkg: {p} {v}".format(p=pkg, v=version))

        opam_version = opam_pkgs.get(pkg)
        if opam_version == None:
            print("Pkg {p} not found".format(p=pkg))
            missing[pkg] = version
        else:
            # print("opam_version: %s" % opam_version)
            if opam_version == version:
                ppx = is_ppx_driver(repo_ctx, pkg)
                opam_pkg_rules.append(
                    "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
                )
                # findlib_pkg_rules.append(
                #     "findlib_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
                # )
            else:
                bad_version[pkg] = version
                # fail("Bad version for pkg {p}. Wanted {v}, found installed: opam {ov}.".format(
                #     p=pkg, v=version, ov=opam_version))

    if len(missing) > 0:
        repo_ctx.report_progress("Missing packages: %s" % missing)
        print("Missing packages: %s" % missing)
        if repo_ctx.attr.install:
            for [pkg, version] in missing.items():
                repo_ctx.report_progress("Installing {p} {v}".format(p=pkg, v=version))
                print("installing {p} {v}".format(p=pkg, v=version))
                result = repo_ctx.execute(["opam", "install", "-y",
                                           pkg + "." + version])
                if result.return_code == 0:
                    repo_ctx.report_progress("Installed {p} {v}".format(p=pkg, v=version))
                    print("installed {p} {v}".format(p=pkg, v=version))
                    if repo_ctx.attr.pin:
                        result = repo_ctx.execute(["opam", "pin", pkg, version])
                        if result.return_code == 0:
                            repo_ctx.report_progress("Pinned {p} {v}".format(p=pkg, v=version))
                            print("pinned {p} {v}".format(p=pkg, v=version))
                            ppx = is_ppx_driver(repo_ctx, pkg)
                            opam_pkg_rules.append(
                                "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
                            )
                        else:
                            print("PIN ERROR RC: %s" % result.return_code)
                            print("PIN STDERR: %s" % result.stderr)
                            print("PIN STDOUT: %s" % result.stdout)
                            return
                else:
                    print("ERROR: OPAM INSTALL {p} RC: {rc}".format(p=pkg, rc=result.return_code))
                    print("STDERR: %s" % result.stderr)
                    print("STDOUT: %s" % result.stdout)
                    return

    if len(bad_version) > 0:
        repo_ctx.report_progress("Bad version packages: %s" % bad_version)
        print("Bad_Version packages: %s" % bad_version)
        if repo_ctx.attr.install:
            for [pkg, version] in bad_version.items():
                repo_ctx.report_progress("Removing {p}".format(p=pkg))
                print("removing {p}".format(p=pkg))
                result = repo_ctx.execute(["opam", "remove", "-y", pkg])
                if result.return_code == 0:
                    repo_ctx.report_progress("Removed {p}".format(p=pkg))
                    print("removed {p}".format(p=pkg))
                else:
                    print("Error in removal of {p}".format(p=pkg))
                    print("REMOVAL RC: %s" % result.return_code)
                    print("REMOVAL STDOUT: %s" % result.stdout)
                    print("REMOVAL STDERR: %s" % result.stderr)
                    fail("REMOVAL ERROR")

                repo_ctx.report_progress("Installing {p} {v}".format(p=pkg, v=version))
                print("installing {p} {v}".format(p=pkg, v=version))
                result = repo_ctx.execute(["opam", "install", "-y",
                                           pkg + "." + version])
                if result.return_code == 0:
                    repo_ctx.report_progress("Installed {p} {v}".format(p=pkg, v=version))
                    print("installed {p} {v}".format(p=pkg, v=version))
                    result = repo_ctx.execute(["opam", "pin", pkg, version])
                    if result.return_code == 0:
                        repo_ctx.report_progress("Pinned {p} {v}".format(p=pkg, v=version))
                        print("pinned {p} {v}".format(p=pkg, v=version))
                        ppx = is_ppx_driver(repo_ctx, pkg)
                        opam_pkg_rules.append(
                            "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
                        )
                    else:
                        print("PIN RC: %s" % result.return_code)
                        print("PIN STDERR: %s" % result.stderr)
                        print("PIN STDOUT: %s" % result.stdout)
                else:
                    print("RC: %s" % result.return_code)
                    print("STDERR: %s" % result.stderr)
                    print("STDOUT: %s" % result.stdout)

    opam_pkgs = "\n".join(opam_pkg_rules)
    return opam_pkgs

###################################
def _config_findlib_pkgs(repo_ctx):
    repo_ctx.report_progress("configuring FINDLIB pkgs...")

    ## FIXME: packages distibuted with the compiler can be hardcoded?
    ## e.g. compiler-libs.common
    ## but then we would have to keep a list for each compiler version...

    findlib_pkg_list = repo_ctx.execute(["ocamlfind", "list"]).stdout.splitlines()
    findlib_pkgs = {}
    for pkg_desc in findlib_pkg_list:
        [pkg, version] = pkg_desc.split("(version: ")
        pkg = pkg.strip(" ")
        version = version.strip(" ").rstrip(")")
        findlib_pkgs[pkg] = version

    findlib_pkg_rules = []
    repo_ctx.report_progress("constructing FINDLIB pkg rules...")
    ## FIXME: uniqify?
    # for [pkg, version] in repo_ctx.attr.findlib_pkgs.items():
    for pkg in repo_ctx.attr.findlib_pkgs:
        findlib_version = findlib_pkgs.get(pkg)
        # if findlib_version == version:
        #     ppx = is_ppx_driver(repo_ctx, pkg)
        findlib_pkg_rules.append(
            "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = "False" )
        )
            # findlib_pkg_rules.append(
            #     "findlib_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
            # )
        # else:
        #     opam_version = opam_pkgs.get(pkg)
        #     if opam_version == version:
        #         ppx = is_ppx_driver(repo_ctx, pkg)
        #         opam_pkg_rules.append(
        #             "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
        #         )
        #         findlib_pkg_rules.append(
        #             "findlib_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
        #         )
        # else:
        #     fail("Bad version for pkg {p}. Wanted {v}, found installed: opam {ov}, findlib {fv}.".format(
        #         p=pkg, v=version, fv=findlib_version))

    findlib_pkg_rules = "\n".join(findlib_pkg_rules)

    return findlib_pkg_rules

#########################
def _pin_paths(repo_ctx):
    repo_ctx.report_progress("pinning OPAM pkgs to paths...")
    print("_PIN_PATHS")

    pin = True
    pins = []
    pinned_pkg_rules = []
    print("PINNED_PKG_RULES: %s" % "\n".join(pinned_pkg_rules))
    fail("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")

    pinlist = repo_ctx.execute(["opam", "pin", "list"]).stdout.splitlines()

    # if len(repo_ctx.attr.pin_paths) > 0:
    rootpath = str(repo_ctx.path(Label("@//:WORKSPACE.bazel")))[:-16]
    print("ROOT DIR: %s" % rootpath)

    # x = tokenize("   foo bar     baz	buz   ")
    # print("X: %s" % x)
    # fail("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")

    pinned_paths = {}
    for pin in pinlist:
        tokens = tokenize(pin)
        print("TOKENS: %s" % tokens)
        [name, kind, spec] = tokens
        if kind == "version":
            pins.append(name)
        else:
            pinned_paths[name] = spec
    for [name, spec] in pinned_paths.items():
        print("PINNED PATH: {n} {s}".format(
            n=name.strip(),
            # k=kind.strip(),
            s=spec.strip()
        ))

     # kinds: git, git+file, rsync

    # TODO
    #  check the path. for now, i  to the user to make sure
    # t
    # e pinning is correct, using the opam command line.

    # TODO: install before pinning?

    for [pkg, path] in repo_ctx.attr.pin_paths.items():
    # for pkg in repo_ctx.attr.pin_paths:
        if pkg in pins:
            ppx = is_ppx_driver(repo_ctx, pkg)
            pinned_pkg_rules.append(
                "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = "False" )
            )
        else:
            pkg_path = rootpath + "/" + path
            repo_ctx.report_progress("Pinning {path} (may take a while)...".format(path = path))

            ## FIXME: add --switch
            pinout = repo_ctx.execute(["opam", "pin", "-v", "-y", "add", pkg_path])

            if pinout.return_code == 0:
                repo_ctx.report_progress("Pinned {path}.".format(path = path))
                ppx = is_ppx_driver(repo_ctx, pkg)
                pinned_pkg_rules.append(
                    "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = "False" )
                )
            else:
                print("ERROR opam pin rc: %s" % pinout.return_code)
                print("ERROR stdout: %s" % pinout.stdout)
                print("ERROR stderr: %s" % pinout.stderr)
                fail("OPAM pin add cmd failed")

    return "\n".join(pinned_pkg_rules)

    # if len(repo_ctx.attr.pin_versions) > 0:
    #     repo_ctx.report_progress("pinning OPAM pkgs to versions...")
    #     if len(pins) == 0:
    #         pins = repo_ctx.execute(["opam", "pin", "list", "-s"]).stdout.splitlines()
    #     for [pkg, version] in repo_ctx.attr.pin_versions.items():
    #         if not pkg in pins:
    #             repo_ctx.report_progress("Pinning {pkg} to {v} (may take a while)...".format(
    #                 pkg = pkg, v = version))
    #             pinout = repo_ctx.execute(["opam", "pin", "-v", "-y", "add", pkg, version])
    #             if pinout.return_code != 0:
    #                 print("ERROR opam pin rc: %s" % pinout.return_code)
    #                 print("ERROR stdout: %s" % pinout.stdout)
    #                 print("ERROR stderr: %s" % pinout.stderr)
    #                 fail("OPAM pin add cmd failed")

###########################################
def _opam_repo_localhost_findlib(repo_ctx):
    # print("opam/_bootstrap:opam.bzl _opam_repo_localhost_findlib(repo_ctx)")
    repo_ctx.report_progress("bootstrapping localhost_findlib OPAM repo...")

    opamroot = repo_ctx.execute(["opam", "var", "prefix"]).stdout.strip()
    # if verbose:
    #     print("opamroot: " + opamroot)

    opam_pkgs    = ""
    findlib_pkgs = ""
    pinned_paths = ""

    if len(repo_ctx.attr.pin_paths) > 0:
        pinned_paths = _pin_paths(repo_ctx)
    print("PINNED PATHS: %s" % pinned_paths)

    if len(repo_ctx.attr.opam_pkgs) > 0:
        opam_pkgs = _config_opam_pkgs(repo_ctx)

    print("OPAMPKGS: %s" % opam_pkgs)

    if len(repo_ctx.attr.findlib_pkgs) > 0:
        findlib_pkgs = _config_findlib_pkgs(repo_ctx)

    print("FINDLIB PKGS: %s" % findlib_pkgs)

    opam_pkgs = opam_pkgs + "\n" + findlib_pkgs + "\n" + pinned_paths
    # print("PKGS:\n%s" % opam_pkgs)

    # opambin = repo_ctx.which("opam") # "/usr/local/Cellar/opam/2.0.7/bin"
    # if "OPAM_SWITCH_PREFIX" in repo_ctx.os.environ:
    #     opampath = repo_ctx.os.environ["OPAM_SWITCH_PREFIX"] + "/bin"
    # else:
    #     fail("Env. var OPAM_SWITCH_PREFIX is unset; try running 'opam env'")
    repo_ctx.report_progress("OPAM: intalling BUILD.bazel files...")

    # repo_ctx.symlink(opambin, "opam") # the opam executable

    local_opam_root = "/Users/gar/.obazl/opam"
    # repo_ctx.symlink(local_opam_root, "opam")
    repo_ctx.symlink(local_opam_root + "/lib", "lib")

    repo_ctx.symlink(opamroot, "sdk")
    repo_ctx.symlink(opamroot + "/bin", "bin")

    repo_ctx.file("WORKSPACE", "", False)
    repo_ctx.template(
        "BUILD.bazel",
        Label("//opam/_templates:opam.BUILD.bazel"),
        executable = False,
    )
    # if len(findlib_pkgs) > 0:
    #     repo_ctx.template(
    #         "findlib/BUILD.bazel",
    #         Label("//opam/_templates:opam.findlib.BUILD.bazel"),
    #         executable = False,
    #         substitutions = { "{FINDLIB_PKGS}": findlib_pkgs }
    #     )
    if len(opam_pkgs) > 0:
        repo_ctx.template(
            "pkg/BUILD.bazel",
            Label("//opam/_templates:opam.pkg.BUILD.bazel"),
            executable = False,
            substitutions = { "{OPAM_PKGS}": opam_pkgs }
        )
    else:
        print("\n\n\t\tWARNING: you have not listed any OPAM package dependencies.  Deps of the form \"@opam//pkg:\" will fail.\n\n")

##############################
def _opam_repo_impl(repo_ctx):

    # x = repo_ctx.read("bzl/opam.bzl")
    # print("READ x: %s" % x)
    # if repo_ctx.attr.hermetic:
    #     opam_repo_hermetic(repo_ctx)
    # else:
        # bootstrap @opam with opam_findlib rules
    hermetics = _opam_repo_localhost_findlib(repo_ctx)


    # return { "foo": "bar" }

    # else:
    # bootstrap @opam with ocaml_import rules
    #     _opam_repo_localhost_imports(repo_ctx)  ## uses bazel rules with ocaml_import

#############################
_opam_repo = repository_rule(
    implementation = _opam_repo_impl,
    configure = True,
    local = True,
    attrs = dict(
        hermetic        = attr.bool( default = True ),
        verify          = attr.bool( default = True ),
        install         = attr.bool( default = True ),
        pin             = attr.bool( default = True ),

        switch_name     = attr.string(),
        switch_compiler = attr.string(),
        opam_pkgs = attr.string_dict(
            doc = "Dictionary of OPAM packages (name: version) to install.",
            # default = {"foo": "bar"}
        ),
        findlib_pkgs = attr.string_list(
            doc = "List of findlib packages to install.",
            # default = []
        ),
        # pins_install = attr.bool(default = True),
        pin_paths = attr.string_dict(
            doc = "Dictionariy of pkgs to pin (name: path)"
        ),
        # pin_versions = attr.string_dict(
        #     doc = "Dictionariy of pkgs to pin (name: path)"
        # ),
        # _switch = attr.string(default = "default")
    )
)

################################################################
def _opam_repo_hidden_impl(repo_ctx):
    repo_ctx.report_progress("Bootstrapping hidden _opam repo...")

    repo_ctx.file("WORKSPACE.bazel", "workspace = ( \"_opam\" )", False)

    opamroot = repo_ctx.execute(["opam", "var", "prefix"]).stdout.strip()
    # print("opamroot: " + opamroot)

    repo_ctx.symlink(opamroot + "/lib", "lib")

    repo_ctx.file(
        "BUILD.bazel",
        content = "exports_files(glob([\"lib/**/*\"]))",
        executable = False,
    )

####################################
_opam_repo_hidden = repository_rule(
    # exposes everything in opam via globbing, for use by ocaml_import rules in ~/.local/share/obazl/opam
    implementation = _opam_repo_hidden_impl,
    local = True,
    # attrs = dict(
    #     hermetic = attr.bool(
    #         default = True
    #     ),
    #     opam_pkgs = attr.string_dict(
    #         doc = "List of OPAM packages to install."
    #     )
    # )
)

################################################################
def opam_configure(
        opam = None,
        switch   = None,
        hermetic = False,
        verify   = True,
        install  = True,
        pin      = True,
        # pkgs = None
):
    """
OPAM Structure: 
    opam_version := string
    switches    := dict(name string, switch struct
Switch struct:
    compiler := version string
    packages := dict(name string, pkg spec)
Pkg spec:
    "pkg name": ["version_string"]
              | ["version_string", ["sublib_a", "sublib_b"]]
              | "path/to/pin"
# First form pins version, second pins version plus findlib subpackages, third pins path

# Example:

PACKAGES = {
    "base": ["v0.12.0"],
    "ocaml-compiler-libs": ["v0.11.0", ["compiler-libs.common"]],
    "ppx_expect": ["v0.12.0", ["ppx_expect.collector"]],
    "ppx_inline_test": ["v0.12.0", ["ppx_inline_test.runtime-lib"]],
    "ppxlib": ["0.8.1"],
    "stdio": ["v0.12.0"],
}

opam = struct(
    version = "2.0",
    switches = {
        "mina-0.1.0": struct(    # first entry is default
            compiler = "4.07.1",  # compiler version
            packages = PACKAGES
        ),
        "4.07.1": struct(
            name     = "4.07.1",
            compiler = "4.07.1",  # compiler version
            packages = PACKAGES
        )
    }
)
"""
    if opam == None:
        print("ERROR: opam arg required")
        return

    if switch == None:
        print("ERROR in workspace code: opam_configure fn missing required arg 'switch'.")
        return

    if hermetic:
        if not opam:
            fail("Hermetic builds require a list of OPAM deps.")

    if hasattr(opam, "switches"):
        if (not types.is_dict(opam.switches)):
                fail("opam.switches must be a dict")

        switch_struct = opam.switches[switch]
        if switch_struct == None:
            print("ERROR: switch {s} not defined in config file".format(s=switch))
            return
        switch_name = switch
        # if hasattr(switch, "name"):
        #     switch_name = switch.name
        # else:
        #     print("ERROR: opam switch must have name field")
        #     return
        # if hasattr(switch, "version"):
        #     switch_version = switch.version
        # else:
        #     print("ERROR: opam switch must have version field")
        #     return
        if hasattr(switch_struct, "compiler"):
            switch_compiler = switch_struct.compiler
        else:
            print("ERROR: opam switch must have compiler version field")
            return

    ## if local opam/obazl preconfigured, just use local_repository to point to it
    ## no need to bootstrap a repo in that case
    ## only use this to dynamically construct the ocaml_import rules
    ## not feasible until we can parse the META files in starlark or we have
    ## a fast tool we can call to do it
    # _opam_repo_hidden(name="_opam")

    opam_pkgs    = {}
    findlib_pkgs = []
    pin_paths = {}

    if switch_struct.packages:
        if (not types.is_dict(switch_struct.packages)):
            fail("switch.packages must be a dict")
        for [pkg, spec] in switch_struct.packages.items():
            # print("PKG: {p} SPEC: {s}".format(p=pkg, s=spec))
            if types.is_list(spec):
                if len(spec) == 0:
                    findlib_pkgs.append(pkg)
                elif len(spec) == 1:  # contains version string
                    opam_pkgs[pkg] =  spec[0]
                elif len(spec) == 2:  # contains tuple of sublibs
                    if types.is_list(spec[1]):
                        opam_pkgs[pkg] =  spec[0]
                        ## FIXME: verify second element is list of strings
                        findlib_pkgs.extend(spec[1])
                    else:
                        fail("switch.packages value entries 2nd element must be list of sublibs")
                else:
                    fail("switch.packages value must be a list of length zero, one or two")
            elif types.is_string(spec): # path/to/pin
                pin_paths[pkg] = spec
                print("PIN PATH: {p} {s}".format(p=pkg, s=pin_paths[pkg]))

            else:
                fail("switch.packages value entries must be list or string")

    # print("OPAM_PKGS: %s" % opam_pkgs)
    # print("FINDLIB_PKGS: %s" % findlib_pkgs)

    # pin_versions = {}
    # pins_install = False
    # if opam != None:
    #     if hasattr(opam, "pins"):
    #         if hasattr(opam.pins, "paths"):
    #             pin_paths = opam.pins.paths
    #         if hasattr(opam.pins, "versions"):
    #             pin_versions = opam.pins.versions
    #         if hasattr(opam.pins, "install"):
    #             pins_install = opam.pins.install

    maybe(
        git_repository,
        name = "obazl_tools_bazel",
        remote = "https://github.com/obazl/tools_bazel",
        branch = "main",
    )

    _opam_repo(name="opam",
               hermetic = hermetic,
               verify   = verify,
               install  = install,
               pin      = pin,
               switch_name = switch_name,
               switch_compiler = switch_compiler,
               opam_pkgs = opam_pkgs,
               findlib_pkgs = findlib_pkgs,
               pin_paths = pin_paths)

               # pins_install = pins_install,
               # pin_versions = pin_versions)
    # native.local_repository(name = "zopam", path = "/Users/gar/.obazl/opam")
