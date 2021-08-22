#!/bin/bash

# will exit with 1 if there are compiler warnings in $1
# will exit with 0 if there are no compiler warnings $1

target=$1

function main() {
    if [ $# -ne 1 ]; then
        usage
        exit 0
    fi

    if [ $1 == "-h" ]; then
        usage
        exit 0
    fi

    if [ $1 == "--help" ]; then
        usage
        exit 0
    fi

    if [ -z $1 ]; then
        echo "No input file specified"
        exit 0
    fi

    if [ ! -f $1 ]; then
        echo "$1 is not a valid file"
        exit 0
    fi

    reg="^(/([a-zA-Z0-9\-\_\.])+)+:[0-9]+:[0-9]+: warning: "

    if cat $1 | grep -q -P "$reg"; then
        echo "Has warnings. Exit 1."
        exit 1
    else
        echo "Has no warnings. Exit 0."
        exit 0
    fi
}

function usage() {
    echo "hasCompilerWarnings.sh: Check a compiler log for warnings"
    echo ""
    echo "usage: hasCompilerWarnings.sh /path/to/build.log"
    echo ""
    echo "Exits with 1 if there are warnings."
    echo "Exits with 0 if there are no warnings or if there are issues with"
    echo "the specified input file."
    echo ""
}

main $@
