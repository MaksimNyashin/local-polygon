#!/usr/bin/bash
my_dir="$(dirname "${BASH_SOURCE[0]}")"
builds=${my_dir}/builds
err=${my_dir}/build_err

> ${err}

remove_trailing_spaces() {
    local line=$1
    echo "${line%"${line##*[![:space:]]}"}"
}

buildLine() {
    echo "${builds}/"$(echo "${1}" | tr '/' '_')
}

builtTime() {
    if [ -f "${1}" ]; then
        stat -c %y $1
    else
        echo ""
    fi
}

startTimer() {
    START=$(date +%s.%N)
}

finishTimer() {
    END=$(date +%s.%N)
    DIFF=$(echo "($END - $START) * 1000" | bc)
    echo "$1: ${DIFF}ms"
}

needsUpdate() {
    [[ $(builtTime "$(buildLine $1)") < $(builtTime "./${1}.cpp") ]]
}

createFolder() {
    if [ ! -d "$1" ]; then
        mkdir -p "$1"
    fi
}

buildFiles() {
    if [ "${2}" = "cpp" ]; then
        if needsUpdate $1; then
            createFolder "$builds"
            g++ ${cpp_version} ${cpp_options} -o $(buildLine $1) ./${1}.cpp 2>> ${err} && echo "${1} is built suucessfully" || echo "CE"
        else
            echo "The ${1} binary is already up-to-date"
        fi
    fi
}

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color
