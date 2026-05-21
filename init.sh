#!/usr/bin/bash

my_dir="$(dirname "$0")"
read -p "dirname: " dir_name
dir_name=../$dir_name

rm -rf $dir_name
cp -r $my_dir $dir_name
cd $dir_name
make clear_all
