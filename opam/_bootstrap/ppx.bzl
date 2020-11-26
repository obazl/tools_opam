# Set up ppx stuff
def is_ppx_driver(repo_ctx, pkg):
    # 'ocamlfind printppx' prints the ppx preprocessor options as they would
    # occur in an OCaml compiler invocation for the packages listed in
    # the command. The output includes one "-ppx" option for each
    # preprocessor. The possible options have the same meaning as for
    # "ocamlfind ocamlc". The option "-predicates" adds assumed
    # predicates and "-ppxopt package,arg" adds "arg" to the ppx
    # invocation of package package.
    # This tells us which packages can serve as ppx exes (?)
    query_result = repo_ctx.execute(["ocamlfind", "printppx", pkg]).stdout.strip()
    # print("IS PPX DRIVER? {pkg} : {ppx}".format( pkg = pkg, ppx = len(query_result)))
    if len(query_result) == 0:
        return False
    else:
        return True

