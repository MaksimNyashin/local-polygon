#!/usr/bin/bash

my_dir="$(dirname "${BASH_SOURCE[0]}")"
home_dir=$my_dir/..
outer_files=${home_dir}/outer_files

source "$outer_files/config.sh"
source "$my_dir/utils.sh"

echo -e ${RED}\"make build\" is outdated. There is no need of using it${NC}

build_files=${my_dir}/../outer_files/cpp_build.txt
err=${my_dir}/build_err

while read line
do
    line=$(remove_trailing_spaces "$line")

    if [ ! "${line:0:2}" = "# " ]; then
        if [ -f "./${line}.cpp" ]; then
            if [ "$(buildFiles "${line}" "cpp")" = "CE" ]; then
                echo -e "${RED}Failed to compile ${line}.cpp${NC}"
                exit 1
            else
                echo "${line} is built successfiully"
            fi
        else
            echo "No ${line}.cpp found"
        fi
        echo '


' >> ${err}
    fi

done < ${build_files}
