load("@bazel_skylib//lib:collections.bzl", "collections")

load(":hermetic.bzl", "opam_repo_hermetic")

load(":ppx.bzl", "is_ppx_driver")

# load("//implementation:common.bzl",
#     "OCAML_VERSION")

# 4.07.1 broken on XCode 12:
# https://discuss.ocaml.org/t/ocaml-4-07-1-fails-to-build-with-apple-xcode-12/6441/15
OCAML_VERSION = "4.08.0"
OCAMLBUILD_VERSION = "0.14.0"
OCAMLFIND_VERSION = "1.8.1"
COMPILER_NAME = "ocaml-base-compiler.%s" % OCAML_VERSION
OPAM_ROOT_DIR = ".opam_root_dir"
# Set to false to see debug messages
DEBUG_QUIET = False

# # The path to the root opam directory within external
# OPAMROOT = "OPAMROOT"

################################################################
def _opam_repo_localhost_findlib(repo_ctx):
    print("opam/_bootstrap:opam.bzl _opam_repo_localhost_findlib(repo_ctx)")
    repo_ctx.report_progress("Bootstrapping localhost_findlib OPAM...")

    # print("CURRENT SYSTEM: %s" % repo_ctx.os.name)
    env = repo_ctx.os.environ
    # for item in env.items():
    #     print("ENV ENTRY: %s" % str(item))

    print("ROOT WS DIRECTORY: %s" % str(repo_ctx.path(Label("@//:WORKSPACE.bazel")))[:-16])

    #### opam pinning
    pin = True
    if len(repo_ctx.attr.pins):
        rootpath = str(repo_ctx.path(Label("@//:WORKSPACE.bazel")))[:-16]
        # print("ROOT DIR: %s" % rootpath)

        pins = repo_ctx.execute(["opam", "pin", "list", "-s"]).stdout.splitlines()
        # print("INSTALLED PINS: %s" % pins)
        for [pkg, path] in repo_ctx.attr.pins.items():
            if not pkg in pins:
                # print("ALREADY PINNED: %s" % pkg)
            # else:
                pkg_path = rootpath + "/" + path
                print("PINNING {pkg} to {path}".format(pkg = pkg, path = path))
                repo_ctx.report_progress("Pinning {path} (may take a while)...".format(pkg = pkg, path = path))
                pinout = repo_ctx.execute(["opam", "pin", "-v", "-y", "add", pkg_path])
                if pinout.return_code != 0:
                    print("ERROR opam pin rc: %s" % pinout.return_code)
                    print("ERROR stdout: %s" % pinout.stdout)
                    print("ERROR stderr: %s" % pinout.stderr)

    opamroot = repo_ctx.execute(["opam", "var", "prefix"]).stdout.strip()
    # if verbose:
    #     print("opamroot: " + opamroot)

    packages = []

    ## we need to run both opam and ocamlfind list
    ## opam includes pinned local pkgs, ocamlfind does not
    ## ocamlfind includes subpackages like ppx_derivine.eq, opam does  not
    ## then we use collections.uniq to dedup.
    pkgs = repo_ctx.execute(["opam", "list"]).stdout.splitlines()
    for pkg in pkgs:
        if not pkg.startswith("#"):
            packages.append(pkg.split(" ")[0])
    pkgs = repo_ctx.execute(["ocamlfind", "list"]).stdout.splitlines()
    for pkg in pkgs:
        packages.append(pkg.split(" ")[0])
    pkgs = collections.uniq(pkgs)

    opam_pkgs = []
    findlib_pkgs = []
    repo_ctx.report_progress("constructing OPAM pkg rules...")
    # for pkg in collections.uniq(packages):
        ## WARNING: this is slow
        # print("PKG rule for %s" % p)
    for [pkg, version] in repo_ctx.attr.pkgs.items():
        # print("PKG: %s" % pkg)
        ppx = is_ppx_driver(repo_ctx, pkg)
        # print("PPX: %s" % ppx)
        opam_pkgs.append(
            "opam_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
        )
        findlib_pkgs.append(
            "findlib_pkg(name = \"{pkg}\", ppx_driver={ppx})".format( pkg = pkg, ppx = ppx )
        )
    # print("ocamlfind pkgs:")
    # for p in opam_pkgs:
    #   print(p)
    ocamlfind_pkgs = "\n".join(opam_pkgs)
    findlib_pkgs = "\n".join(findlib_pkgs)

    opambin = repo_ctx.which("opam") # "/usr/local/Cellar/opam/2.0.7/bin"
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
        Label("//opam/_templates:BUILD.opam"),
        executable = False,
        # substitutions = { "{opam_pkgs}": ocamlfind_pkgs }
    )
    repo_ctx.template(
        "pkg/BUILD.bazel",
        Label("//opam/_templates:BUILD.opam.pkg"),
        executable = False,
        substitutions = { # "{has_pkg_values}": has_pkgs,
                          "{OPAM_PKGS}": ocamlfind_pkgs }
    )
    repo_ctx.template(
        "findlib/BUILD.bazel",
        Label("//opam/_templates:BUILD.opam.findlib"),
        executable = False,
        substitutions = { # "{has_pkg_values}": has_pkgs,
                          "{FINDLIB_PKGS}": findlib_pkgs }
    )

##############################
def _opam_repo_impl(repo_ctx):
    # repo_ctx.report_progress("Bootstrapping OPAM... hermetic? {}".format(repo_ctx.attr.hermetic))

    if repo_ctx.attr.hermetic:
        opam_repo_hermetic(repo_ctx)
    else:
        # bootstrap @opam with opam_findlib rules
        _opam_repo_localhost_findlib(repo_ctx)
    # else:
    # bootstrap @opam with ocaml_import rules
    #     _opam_repo_localhost_imports(repo_ctx)  ## uses bazel rules with ocaml_import

#############################
_opam_repo = repository_rule(
    implementation = _opam_repo_impl,
    configure = True,
    local = True,
    attrs = dict(
        hermetic = attr.bool(
            default = True
        ),
        pkgs = attr.string_dict(
            doc = "Dictionary of OPAM packages (name: version) to install."
        ),
        pins = attr.string_dict(
            doc = "Dictionariy of pkgs to pin (name: path)"
        )
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
def opam_configure(hermetic = False,
                   opam = None,
                   switch = "4.07.1",
                   pkgs = None):
    if hermetic:
        if not opam:
            fail("Hermetic builds require a list of OPAM deps.")

    if opam:
        if hasattr(opam, "pins"):
            pins = opam.pins
        else:
            pins = {}

    ## if local opam/obazl preconfigured, just use local_repository to point to it
    ## no need to bootstrap a repo in that case
    ## only use this to dynamically construct the ocaml_import rules
    ## not feasible until we can parse the META files in starlark or we have
    ## a fast tool we can call to do it
    # _opam_repo_hidden(name="_opam")

    _opam_repo(name="opam", hermetic = hermetic,
               pkgs = opam.packages if opam else {},
               pins = pins)
    # native.local_repository(name = "zopam", path = "/Users/gar/.obazl/opam")
