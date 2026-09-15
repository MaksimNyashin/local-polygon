#!/usr/bin/bash

my_dir=$(pwd)/$(dirname "${BASH_SOURCE[0]}")
home_dir=${my_dir}/..
inner_files=${my_dir}
outer_files=${home_dir}/outer_files
cpp_sources=${inner_files}/cpp
source "$outer_files/config.sh"

mkdir -p ${cpp_sources}/bin

src_file=${cpp_sources}/$1.cpp
utils_file=${cpp_sources}/utils.h
o_file=${cpp_sources}/bin/$1

if [[ $(stat -c %y $o_file) < $(stat -c %y $src_file) || $(stat -c %y $o_file) < $(stat -c %y $utils_file) ]]; then
    g++ -std=c++2a $src_file -o $o_file && $o_file $2
else
    $o_file $2
fi