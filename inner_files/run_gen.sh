#!/usr/bin/bash

my_dir="$(dirname "${BASH_SOURCE[0]}")"
home_dir=${my_dir}/..
inner_files=${home_dir}/inner_files
outer_files=${home_dir}/outer_files

test_dir=${home_dir}/tests
tests=${test_dir}/tests.txt
in=${inner_files}/in.txt
out=${inner_files}/out.txt
err=${inner_files}/_err
builds=${inner_files}/builds
gen_strings=${outer_files}/gen_strings.txt
validator_logs=${inner_files}/validator_logs.txt
tests_tests=${test_dir}/tests

source "$outer_files/config.sh"
source "$inner_files/utils.sh"

echo -e ${YELLOW}\"make gen\" is becoming outdated. Try using \"make c_gen\"${NC}

createFolder $tests_tests

> $tests
> $err


built_main=$(buildLine "${main}")
if [ "$(buildFiles "${main}" "${lang}")" = "CE" ]; then
    echo -e "${RED}Failed to compile ${main}.cpp${NC}"
    exit 1
fi

built_validator=$(buildLine "validator")
if [ "$(buildFiles validator cpp)" = "CE" ]; then
    echo -e "${RED}Failed to compile validator.cpp${NC}"
    exit 1
fi

group=group
total_num=0
suc_num=0
group_id=0

compiled_generators=()
failed_generators=()
test_num=${first_test_num}

while read line
do
    line=$(remove_trailing_spaces "$line")

    if [ "${line}" = "" ] || [ "${line:0:2}" = "# " ]; then
        continue
    fi

    if [ "${line:0:8}" = "--group=" ]; then
        ((group_id++))
        # group="${group_id}${line:8}"
        group="${line:8}"
        echo "________${group}________"
        continue
    fi

    gen_name=${line%% *}

    if [[ "${failed_generators[@]}" =~ "${gen_name}" ]]; then
        echo -e "${RED}${gen_name}.cpp didn't compile succesfully${NC}"
        ((test_num++))
        ((total_num++))
        continue
    fi

    if [[ ! "${compiled_generators[@]}" =~ "${gen_name}" ]]; then
        if [ "$(buildFiles "${gen_name}" "cpp")" = "CE" ]; then
            echo -e "${RED}Failed to compile ${gen_name}.cpp${NC}"
            failed_generators+=("$gen_name")
            ((test_num++))
            ((total_num++))
            continue
        else
            compiled_generators+=("$gen_name")
        fi
    fi

    echo $line

    echo "Test ${test_num}:"
    startTimer
    ${builds}/${line} > $in 2>> $err
    finishTimer "    gen"
    echo "----------in#$test_num----------" >> $tests
    cat $in >> $tests

    echo "----------val#$test_num---------" >> $tests
    ${builds}/validator --testset tests --group "$group" --testOverviewLogFileName "$validator_logs" --testCaseFileName "$in" < "$in" 2>> "$tests"

    if [[ $? -eq 0 ]]; then
        echo "----------out#$test_num---------" >> $tests
        startTimer
        if [ "${lang}" = "py" ]; then
            $py_version ./${main}.py < $in > $out 2>> $err
        else
            $built_main < $in > $out 2>> $err
        fi
        finishTimer "    time"
        cat $out >> $tests

        file_test_num=$(printf "%02d\n" $test_num)
        cp $in ${tests_tests}/$file_test_num.tst
        cp $out ${tests_tests}/$file_test_num.ans
        ((suc_num++))
    else
        echo '    Validator check is failed' >> $tests
    fi
    ((total_num++))

    echo "----------end#$test_num---------" >> $tests
    echo '' >> $tests

    ((test_num++))

done < ${gen_strings}

if [ -f ${validator_logs} ]; then
    echo '
________Validator results________'
    echo "Successfully generated/validated  ${suc_num}/${total_num}  tests
"
    # cat ${validator_logs}
fi
