#!/usr/bin/bash

my_dir="$(dirname "${BASH_SOURCE[0]}")"
home_dir=${my_dir}/..
inner_files=${home_dir}/inner_files
outer_files=${home_dir}/outer_files

default_files=${home_dir}/default_files
solutions=${home_dir}/solutions
tests=${home_dir}/tests
checkers=${home_dir}/checkers

rm -f ${solutions}/*

for filename in ${default_files}/*
do
    filename_name=${filename##*/}
    if [ "${filename_name:0:4}" = "main" ]; then
        mv ${filename} ${solutions}/${filename_name}
    elif [ "${filename_name:0:7}" = "checker" ]; then
        mv ${filename} ${checkers}/${filename_name}
    else
        mv ${filename} ./${filename_name}
    fi
done

rm -rf ${default_files}
rm -f ${inner_files}/builds/*
rm -rf ${inner_files}/err
rm -f ${inner_files}/in.txt
rm -f ${inner_files}/out.txt
rm -f ${inner_files}/cpp/bin/*
rm -f ${tests}/tests/*.tst
rm -f ${tests}/tests/*.ans
rm -r ${tests}/validator/.gitkeep
rm -rf ${tests}/validator/*
rm -f ${tests}/*.txt
rm -f ${inner_files}/validator_logs.txt
rm -f ${home_dir}/freemaker.txt
rm -f ${inner_files}/invokation/out.txt
rm -f ${inner_files}/invokation/err
rm -f ${inner_files}/invokation/tmp/*
rm -rf ${home_dir}/.git
rm -f ${home_dir}/.gitignore
rm -rf ${home_dir}/cont


> ${solutions}/in.txt
echo "--group=example" > ${outer_files}/gen_strings.txt