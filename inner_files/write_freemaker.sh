#!/usr/bin/bash

my_dir="$(dirname "${BASH_SOURCE[0]}")"
home_dir=${my_dir}/..
inner_files=${home_dir}/inner_files
outer_files=${home_dir}/outer_files

gen_strings=${outer_files}/gen_strings.txt

source "$inner_files/utils.sh"

while read line
do
    line=$(remove_trailing_spaces "$line")

    if [ "${line}" = "" ]; then
        echo ''
        continue
    fi

    if [ "${line:0:8}" = "--group=" ]; then
        continue
    fi

    echo "${line} > $"

done < ${gen_strings}