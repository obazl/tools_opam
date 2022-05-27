## opam_repository

#  1. fetch ocaml compiler, within @opam
#  2. fetch opam-repository
#  3. for each pkg:
#     a. find and parse opam file in opam-repository, extracting:
#        1. download url for src pkg
#        2. build recipe
#     b. apply repo rule 'new_opam_pkg_repo' with info from opam file
#        0. creates repo for pkg, e.g. @yojson
#        1. fetch and download sources from url obtained from opam file
#        2. convert opam build recipe to starlark string
#        3. use repo.ctx.file to write ws and build files

# NB: using Bazel routines puts stuff into Bazel's cache:
# repo_ctx.download_and_extract puts the unzipped archive onto the
# cache, and repo_ctx.file puts the ws and build files into the cache.

# IOW, we can create the ws and build files directly, by the tool we
# run with repo_ctx.execute. But Bazel would not aware of them. So our
# tool must create the content, but then return it to our repo rule,
# which will pass it as input to repo_ctx.file. Or the tool could
# write the build file to an intermediate file, and the the repo rule
# would read it using repo_ctx.read and pass the content to
# repo_ctx.file.  This is how e.g. http_archive works.

# we can use the same code http_archive uses:
# load(
#     "@bazel_tools//tools/build_defs/repo:utils.bzl",
#     "workspace_and_buildfile")
#

# http_archive gets input for repo_ctx.file via rule attributes. But we
# won't know what that input is until we download and analyse opam
# files, so we obtain it by calling repo_ctx.execute on a tool that
# returns the desired content.

# Note that our current bootstrapper writes build files directly,
# without using repo_ctx.file. It seems to work, but then the question
# is whether it regenerates those files when it does not need to.

##################################
def _opam_create_switch(repo_ctx):
    print("_opam_create_switch")

    cmd = [
        "opam", "switch", "create", "ocaml-base-compiler.4.12.0",
        # "--root=./root",
        "--yes", "--quiet"
    ]

    repo_ctx.report_progress("Creating switch.".format(v = repo_ctx.attr.ocaml))
    result = repo_ctx.execute(cmd)

    if result.return_code == 0:
        msg = result.stdout.strip()
        print("CMD stdout: %s" % msg)
        repo_ctx.report_progress("Created switch")
    else:
        print("OPAM cmd {cmd} rc    : {rc}".format(cmd=cmd, rc= result.return_code))
        print("OPAM cmd {cmd} stdout: {stdout}".format(cmd=cmd, stdout= result.stdout))
        print("OPAM cmd {cmd} stderr: {stderr}".format(cmd=cmd, stderr= result.stderr))
        fail("OPAM cmd failure.")

        # "local", "file:///{base}/opam-repository".format(base=repobase),
        # "obazl",
        # "-k", "local",
        # "--compiler=ocaml-base-compiler.{version}".format(
        #     version = repo_ctx.attr.ocaml),

#############################
def _opam_set_repo(repo_ctx):
        ### now set the repo
    cmd = [
        "opam", "repository",
        "--root=./root",
        "set-url", "default", "default=../opam-repository",
        "--yes", "--quiet"
    ]

    repo_ctx.report_progress("Set URL.")
    result = repo_ctx.execute(cmd)

    if result.return_code == 0:
        msg = result.stdout.strip()
        print("Set repo url: %s" % msg)
        repo_ctx.report_progress("set repo url")
    else:
        print("OPAM cmd {cmd} rc    : {rc}".format(cmd=cmd, rc= result.return_code))
        print("OPAM cmd {cmd} stdout: {stdout}".format(cmd=cmd, stdout= result.stdout))
        print("OPAM cmd {cmd} stderr: {stderr}".format(cmd=cmd, stderr= result.stderr))
        fail("OPAM cmd failure.")

#########################
def _opam_init_root(repo_ctx):
    print("_opam_init_root")
    # repobase = repo_ctx.path("opam_repository")

    ## initialize opam root
    cmd = [
        "opam", "init",
        # "../opam-repository",
        # "--kind=local",
        "--root=opam-repository/root",
        "--bare", "--no-setup", "--no-opamrc",
        "--yes", "--quiet"
    ]
    print("OPAM INIT cmd: %s" % cmd)

    repo_ctx.report_progress("Initializing OPAM with compiler version {v}. This downloads the OPAM repository, then downloads and builds the OCaml compiler, so it will probably take several minutes at least; please be patient.".format(v = repo_ctx.attr.ocaml))
    result = repo_ctx.execute(cmd)

    if result.return_code == 0:
        msg = result.stdout.strip()
        print("CMD stdout: %s" % msg)
        repo_ctx.report_progress("Initialized OPAM")
    else:
        print("OPAM cmd {cmd} rc    : {rc}".format(cmd=cmd, rc= result.return_code))
        print("OPAM cmd {cmd} stdout: {stdout}".format(cmd=cmd, stdout= result.stdout))
        print("OPAM cmd {cmd} stderr: {stderr}".format(cmd=cmd, stderr= result.stderr))
        fail("OPAM cmd failure.")

##################################
def _download_opam_repo(repo_ctx):
    print("downloading opam repository")

    result = repo_ctx.download_and_extract(
        url = "https://github.com/ocaml/opam-repository/archive/f569100d0045c2959ab9af0a04fef6f04f882e5e.zip",
        output = "opam-repository",
        stripPrefix = "opam-repository-f569100d0045c2959ab9af0a04fef6f04f882e5e",
        sha256 = "3824de87034d7e553c18df1fe2a577912f71009ee06a6c2e8b50612839e5e9fc",
    )

    print("writing opam-repository/WORKSPACE.bazel")
    repo_ctx.file(
        "opam-repository/WORKSPACE.bazel",
        content = """workspace( name = \"opam-repository\" )
        """,
        executable=False
    )
    print("done writing")

#######################################
def _install_opam_repository(repo_ctx):

    _download_opam_repo(repo_ctx)

    _opam_init_root(repo_ctx)
    # _opam_set_repo(repo_ctx)
    _opam_create_switch(repo_ctx)

###############################
def impl_opam_repository(repo_ctx):

    debug = True ## False

    if debug:
        print("OPAM_REPOSITORY TARGET: %s" % repo_ctx.name)
        print("opam deps: %s" % repo_ctx.attr.packages)
        print("custom build_files: %s" % repo_ctx.attr.build_files)

    repo_ctx.file(
        "WORKSPACE.bazel",
        content = """workspace( name = \"opam\" )
        """,
        executable=False
    )

    root = repo_ctx.path(".").realpath
    repo_ctx.file(
        "BUILD.bazel",
        content = """
print('Hello from repo @opam!')
genrule(
    name = "test",
    outs = ["hello.txt"],
    cmd = "echo \\"Hello world\\" > $@"
)

genrule(
    name = "list",
    outs = ["opam-list.txt"],
    cmd = "\\n".join([
"echo HELLO",
"echo ROOT: {root}",
"opam list --root={root}/opam-repository/root --base 2>&1 | tee $@"
        ])
)
        """.format(root=root),
        executable=False
    )

    _install_opam_repository(repo_ctx)

#####################
opam_repository = repository_rule(
    implementation = impl_opam_repository,
    environ = [
        "XDG_CACHE_HOME",
    ],
    # configure = True,
    # local = True,
    doc = """Configures OPAM installation""",
    attrs = dict(
        opam = attr.string(
            doc = "OPAM version"
        ),

        ocaml = attr.string(
            doc = "OCaml version"
        ),

        packages = attr.string_dict(
            doc = """Dictionary of OPAM package:version dependencies.

            """,
        ),

        build_files  = attr.label_keyed_string_dict(
            doc = """Dictionary mapping custom build files to OPAM package names.

            """,
        ),

        # download = attr.bool(
        #     default = False,
        # ),

        # hermetic = attr.bool(
        #     default = False,
        # ),

        # bootstrapper = attr.label(
        #     executable = True,
        #     cfg = "host",
        #     default = "@tools_obazl//bootstrap:opam_bootstrap",
        #     allow_single_file = True
        # ),
    ),
)
