#!/usr/bin/bash

my_dir="$(dirname "${BASH_SOURCE[0]}")"
home_dir=${my_dir}/..
outer_files=${home_dir}/outer_files

source "$outer_files/config.sh"
source "$my_dir/utils.sh"
in=./solutions/in.txt
err=./solutions/err

> $err

if [ "${lang}" = "py" ]; then
    startTimer
    $py_version ./${main}.py < $in 2>> $err
    finishTimer "Time"
else
    if [ "$(buildFiles $main cpp)" = "CE" ]; then
        echo "Failed to compile $main"
        exit 1
    fi
    # echo $(buildLine "$main")
    startTimer
    $(buildLine "$main") < $in 2>> $err
    finishTimer "Time"
fi
