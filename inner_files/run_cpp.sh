#!/usr/bin/bash

my_dir=$(pwd)/$(dirname "${BASH_SOURCE[0]}")
home_dir=${my_dir}/..
inner_files=${home_dir}/inner_files
outer_files=${home_dir}/outer_files
cpp_sources=${inner_files}/cpp
source "$outer_files/config.sh"


g++ -std=c++2a ${cpp_sources}/$1.cpp -o ${cpp_sources}/tmp/$1 && ${cpp_sources}/tmp/$1