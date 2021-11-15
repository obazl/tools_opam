def build_opam_bootstrapper(repo_ctx):

    bootstrap_opam = "https://github.com/obazl/bootstrap_opam/archive/refs/heads/main.zip"
    # bootstrap_opam = "https://github.com/obazl/bootstrap_ocaml/archive/aa85ef74ab038fbb1eaafe768814d68193b886ef.zip"

    repo_ctx.download_and_extract(
        bootstrap_opam,
        "bootstrap_opam",
        stripPrefix = "bootstrap_opam-main"
    )

    # if "PATH" in repo_ctx.os.environ:
    #     path = repo_ctx.os.environ["PATH"]
    # if "BINDIR" in repo_ctx.os.environ:
    #     print("BINDIR: %s" % repo_ctx.os.environ["BINDIR"])

    # if "OBAZL_BOOTSTRAP" in repo_ctx.os.environ:
    #     path = str(here) + "/" + repo_ctx.os.environ["OBAZL_BOOTSTRAP"] + ":" + path

    # print("OBAZL path: %s" % path)

    # fail("test")

    # cmd_env["PATH"] = path
    # print("BOOTSTRAPPER: %s" % repo_ctx.attr.bootstrapper)
    # print("BOOTSTRAPPER: %s" % repo_ctx.attr.bootstrapper[DefaultInfo])
    # f = repo_ctx.file(repo_ctx.attr.bootstrapper)
    # print("BOOTSTRAPPER: %s" % f)
    # for x in repo_ctx.file.bootstrapper[DefaultInfo].files.to_list():
    #     print("F: %s" % x)

    ## FIXME: cache bootstrap result

    if "HOME" in repo_ctx.os.environ:
        home = repo_ctx.os.environ["HOME"]
    else:
        fail("HOME not in env.")

    local_bin = home + "/.local/bin"

    cmd = ["make", "-C", "./bootstrap_opam/src", "opam_bootstrap"]

    xr = repo_ctx.execute(cmd) ## , environment=cmd_env)
    if xr.return_code == 0:
        print("make bootstrap_opam stdout: %s" % xr.stdout)
    else:
        print("make bootstrap_opam result: %s" % xr.stdout)
        print("make bootstrap_opam rc: {rc} stderr: {stderr}".format(rc=xr.return_code, stderr=xr.stderr));
        fail("Comand failed: make -C bootstrap_opam")

    cmd = ["cp", "-v", "./bootstrap_opam/src/opam_bootstrap", local_bin]
    xr = repo_ctx.execute(cmd) ## , environment=cmd_env)
    if xr.return_code == 0:
        print("cp src/opam_bootstrap ~/.local/bin succeeded; stdout: %s" % xr.stdout)
    else:
        print("cp src/opam_bootstrap result: %s" % xr.stdout)
        print("cp src/opam_bootstrap rc: {rc} stderr: {stderr}".format(rc=xr.return_code, stderr=xr.stderr));
        fail("Comand failed: %s" % cmd)

################################################################
def build_re2c(repo_ctx):

    # Have we already built re2c? FIXME: check version
    repo_ctx.report_progress("Checking for cached re2c...")
    repo_ctx.file(
        "re2c.sh",
        content = "\n".join([
            "#!/bin/bash",
            "if [[ -x ${HOME}/.local/bin/re2c ]]; then",
            "    VERNUM=`${HOME}/.local/bin/re2c --vernum`",
            "    if [[ $VERNUM -eq \"020003\" ]]; then",
            "        echo \"re2c already built\"",
            "        exit 0",
            "    else",
            "        echo ${VERNUM}",
            "        exit -2",
            "    fi",
            "fi",
            "exit -1",
        ])
    )

    cmd = ["./re2c.sh"]
    xr = repo_ctx.execute(cmd)
    if xr.return_code == 0:
        print("re2c already built")
        return
    elif xr.return_code == 254:
        print("Found $HOME/.local/bin/re2c, vernum {VERNUM} - expected 02003".format(VERNUM = xr.stdout))
    else:
        print("ls re2c rc: {rc}".format(rc=xr.return_code))
        print("ls re2c stderr: {stderr}".format(stderr=xr.stderr))
        print("ls re2c stdout: %s" % xr.stdout)
        print("re2c not found; building")

    repo_ctx.report_progress("... not found.")

    # fail("test")

    repo_ctx.download_and_extract(
        "https://github.com/skvadrik/re2c/archive/2.0.3.zip",
        "re2c",
        stripPrefix = "re2c-2.0.3",
        sha256 = "8f74163d02b4ce371d69876af1610177b45055b387656d0fb22c3eab131ccbf9",
    )

    repo_ctx.file(
        "re2c.sh",
        content = "\n".join([
            "#!/bin/bash",
            "if [[ -x ${HOME}/.local/bin/re2c ]]; then",
            "    echo \"re2c already built\"",
            "    exit 0",
            "fi",
            "echo \"Buiding re2c...\"",
            "cd re2c",
            "autoreconf -i -W all",
            "echo \".configure...\"",
            "./configure",
            "echo \".make...\"",
            "make",
            "mkdir -p ${HOME}/.local/bin",
            "cp re2c ${HOME}/.local/bin",
            "cd -"
        ])
    )

    repo_ctx.report_progress("Building re2c (may take a while)...")

    cmd = ["./re2c.sh"]
    xr = repo_ctx.execute(cmd)
    if xr.return_code == 0:
        print("re2c autoreconf: %s" % xr.stdout)
    else:
        print("re2c autreconf rc: {rc}".format(rc=xr.return_code))
        print("re2c autoreconf stderr: {stderr}".format(stderr=xr.stderr))
        print("re2c autoreconf stdout: %s" % xr.stdout)
        fail("Comand failed: %s" % cmd)

    repo_ctx.delete("re2c.sh")

################################################################
def build_tools(repo_ctx):

    build_re2c(repo_ctx)

    build_opam_bootstrapper(repo_ctx)

################################################################
def get_xdg(repo_ctx):
    if "HOME" in repo_ctx.os.environ:
        home = repo_ctx.os.environ["HOME"]
        # print("HOME: %s" % home)
    else:
        fail("HOME not in env.")

    if "XDG_CACHE_HOME" in repo_ctx.os.environ:
        xdg_cache_home = repo_ctx.os.environ["XDG_CACHE_HOME"]
    else:
        xdg_cache_home = home + "/.cache"

    if "XDG_CONFIG_HOME" in repo_ctx.os.environ:
        xdg_config_home = repo_ctx.os.environ["XDG_CONFIG_HOME"]
    else:
        xdg_config_home = home + "/.config"

    if "XDG_DATA_HOME" in repo_ctx.os.environ:
        xdg_data_home = repo_ctx.os.environ["XDG_DATA_HOME"]
    else:
        xdg_data_home = home + "/.local/share"

    return home, repo_ctx.path(xdg_cache_home), repo_ctx.path(xdg_config_home), repo_ctx.path(xdg_data_home)

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

    repo_ctx.file(
        "BUILD.bazel",
        content = """
print('Hello from repo @opam!')
genrule(
    name = "test",
    outs = ["hello.txt"],
    cmd = "echo \\"Hello world\\" > $@"
)
        """,
        executable=False
    )

    install_opam_repository(ctx)

#####################
opam_repository = repository_rule(
    implementation = impl_opam_repository,
    environ = [
        "XDG_CACHE_HOME",
    ],
    configure = True,
    local = True,
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
