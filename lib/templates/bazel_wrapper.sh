#!/usr/bin/env bash

# https://github.com/bazelbuild/bazel/blob/80f7e529eafcaa6f7395b9bfbbe8cac07bc41406/scripts/packages/bazel.sh#L34
# In addition, if an executable called "tools/bazel" is found in the current
# workspace, this script will not directly execute Bazel, but instead store
# the path to the real Bazel executable in the environment variable BAZEL_REAL
# and then execute the "tools/bazel" wrapper script. The location of the wrapper
# script relative to the workspace can be changed with the $BAZEL_WRAPPER
# environment variable.

# with bazelisk (https://github.com/bazelbuild/bazelisk):
# Bazel installers typically provide Bazel's shell wrapper script as the bazel on the PATH.

# When installed this way, Bazel checks the .bazelversion file itself,
# but the failure when it mismatches with the actual version of Bazel
# can be quite confusing to developers. You may find yourself having
# to explain the difference between Bazel and Bazelisk (especially
# when you upgrade the pinned version). To avoid this, you can add a
# check in your tools/bazel wrapper. Since Bazelisk is careful to
# avoid calling itself in a loop, it always calls the wrapper with the
# environment variable BAZELISK_SKIP_WRAPPER set to `true'. You can
# check for the presence of that variable, and when not found, report
# a useful error to your users about how to install Bazelisk.

# if [[ "$BAZELISK_SKIP_WRAPPER" = "true" ]]
# then
#     echo "running BAZELISK"
# else
#     echo "NOT running BAZELISK"
# fi

# echo "args: $@"

SWITCH=`opam switch show`
while getopts "vs:" o; do
    echo "OPT: ${o}"
    case "${o}" in
        s)
            SWITCH=${OPTARG}
            ;;
        v)
            VERBOSE=1
            ;;
        *)
            # echo "Unrecognized option: ${o}"
            # exit
            ;;
    esac
done
shift $((OPTIND-1))

# echo "SWITCH: ${SWITCH}"

CMD=$1
shift 1

# echo "CMD: ${CMD}"

if [[ $CMD = "help" ]]
then
    ${BAZEL_REAL} ${CMD} $@
    exit
fi

reg1="${HOME}/.opam/${SWITCH}/share/obazl"

if [[ -e $reg1 ]]
then
    reg=$reg1
else
    reg2="${HOME}/.local/share/obazl/opam/${SWITCH}"
    if [[ -e $reg2 ]]
    then
        reg=$reg2
    else
        # local switch
        # FIXME: if -s passed, compare it
        reg3="`pwd`/_opam/share/obazl"
        if [[ -e $reg3 ]]
        then
            reg=$reg3
        else
            echo "Switch: ${SWITCH}"
            echo "Registry for switch not found. Tried:"
            echo "     ${reg1}"
            echo "     ${reg2}"
            echo "     ${reg3}"
            echo "Did you forget to run 'coswitch'?"
            exit
        fi
    fi
fi

## WARNING: we need to always pass --registry,
## even for non-build cmds like mod or query;
## otherwise bazel_deps will go unresolved
set -- "--registry=file://${reg}" "$@"

if [[ -e $VERBOSE ]]
then
    set -x
fi

${BAZEL_REAL} ${CMD} $@
