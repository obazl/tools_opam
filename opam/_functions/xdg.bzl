
################################################################
def get_xdg_paths(repo_ctx):
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

