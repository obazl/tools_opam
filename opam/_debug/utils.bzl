DEBUG = False

def debug_report_progress(repo_ctx, msg):
    if repo_ctx.attr.debug:
        print(msg)
        repo_ctx.report_progress(msg)
        for i in range(50000000):
            x = 1
