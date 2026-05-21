#!/usr/bin/bash

my_dir=$(pwd)/$(dirname "${BASH_SOURCE[0]}")
home_dir=${my_dir}/../..
inner_files=${home_dir}/inner_files
outer_files=${home_dir}/outer_files

solutions=solutions
tests=${home_dir}/tests/tests
checkers=${home_dir}/checkers
invoke_err=${my_dir}/err
result=${my_dir}/result.txt
invokation_tmp=${my_dir}/tmp

source "$outer_files/config.sh"
source "$inner_files/utils.sh"

createFolder "$invokation_tmp"

test_names=()
test_results=()

for filename in ${tests}/*.tst; do
    testname=${filename%\.*}
    if [ -f "${testname}.ans" ]; then
        test_results+=("| ${testname##*/} |")
        test_names+=("$testname")
    fi
done

checker="checkers/${checker}"
local_checker=$(buildLine "${checker}")
if [ "$(buildFiles "${checker}" "cpp")" = "CE" ]; then
    echo -e "${RED}Failed to compile ${checker}.cpp${NC}"
    exit 1
fi

upd_tl=$(bc <<< "scale=3; (${tl} + 1000)/1000")
real_tl=$(bc <<< "scale=3; (${tl} + 100)/1000")

> $invoke_err
> $result

run_py() {
    local filename=$1
    local ext=$2
    local test=$3
    local solutions=$4
    local out=$5
    local invoke_err=$6
    local tmp_file=$7
    local py_version=$8

    # echo "$@" >> ${invoke_err}
    if [[ "${ext}" = "py" ]]; then
        $py_version "${solutions}/${filename}.${ext}" < "${test}.tst" > "${out}" 2>> "${invoke_err}"
    else
        ${filename} < "${test}.tst" > "${out}" 2>> "${invoke_err}"
    fi
    echo $? >> ${tmp_file}
}

export -f run_py

timeout_run_py() {
    local out=$4
    local tmp_file=$5
    > ${tmp_file}
    timeout ${upd_tl} bash -c "run_py "${1}" "${2}" "${3}" "${solutions}" "${out}" "${invoke_err}" "${tmp_file}" "${py_version}" 2>> ${invoke_err}"
    return_code=$?
    # echo $(wc -l < ${tmp_file}) >> ${invoke_err}
    if [[ $(wc -l < ${tmp_file}) -eq 0 ]]; then
        echo -2 >> ${tmp_file}
    fi
    echo $return_code >> ${tmp_file}
}

solutions_names=("| result |")
solution_ind=0

time_spent_arr=("| time |")
result_arr=("| total |")
test_passed=("| passed |")
test_right=("| right |")
dashes=("+ ------")

run_tests() {
    local filename=$1
    local ext=$2
    local sol_ind=$3
    local verdict=$4
    local filename_name=$filename

    local answer=()

    if [ "$ext" = "cpp" ]; then
        filename=$(buildLine "${solutions}/${filename}")
    fi

    local rnd=${invokation_tmp}/$RANDOM
    local out=${rnd}.out
    local tmp_file=${rnd}_tmp

    local time_spent=0.0

    for test in ${test_names[@]}; do
        if [ ! "${verdict}" = "CE" ]; then
            verdict=OK

            local time_mes=`(time timeout_run_py "${filename}" "${ext}" "${test}" "${out}" "${tmp_file}") 2>&1`
            # echo $time_mes
            time_mes=$(fgrep 'real' <<< "$time_mes")
            time_mes=${time_mes:7}
            time_mes=${time_mes%\s*}

            if [ "${time_mes:0:2}" = ".." ]; then
                echo "Two dots ????? $time_mes"
                echo -e "$filename_name\t${test##*/}\t\t$time_mes\t$time_spent\t$return_code\t$tl_exit_code\t$time_mes0" >> ${invoke_err}
                time_mes=$upd_tl
            fi

            res=($(< "${tmp_file}"))
            return_code=${res[0]}
            # echo return_code=$return_code
            tl_exit_code=${res[1]}
            if [[ ${return_code} -eq 1 ]]; then
                verdict=RE
            fi

            # echo tl_code=$tl_exit_code
            # echo "$filename_name      $time_mes $time_spent" >> ${invoke_err}
            time_spent=$(bc <<< "if ($time_mes > $time_spent) $time_mes else $time_spent")

            # echo ${real_tl} ${time_mes}

            if [ "${verdict}" = "OK" ]; then
                if [[ ${tl_exit_code} -eq 124 ]]; then
                    # echo -e "${BLUE}time limit exceeded${NC}: program run more than ${tl}"
                    verdict=TL
                # elif [[ "${real_tl}" < "${time_mes}" ]]; then
                elif [[ $(bc <<< "$real_tl < $time_mes") = 1 ]]; then
                    verdict=TL
                else
                    checker_message=$(${local_checker} --testset tests --group 0 "${test}.tst" "${out}" "${test}.ans" 2>&1)
                    res=$?
                    # echo checker_message=$checker_message
                    # echo res=$res
                    if [[ $res -eq 1 ]]; then
                        verdict=WA
                        # comment="${checker_message:13}"
                    fi

                fi
            fi
        fi
        answer+=("${verdict}")
    done

    rm $tmp_file
    rm $out
    echo "$sol_ind $filename_name.$ext $time_spent ${answer[@]}" >> ${result}
    echo "$filename_name.$ext finihed testing"
}

possible_errors_arr=()

while read line; do
    line=$(remove_trailing_spaces "${line}")
    if [ "${line}" = "" ] || [ "${line:0:2}" = "# " ]; then
        continue;
    fi

    arr=($(tr ' ' '\n' <<< "${line}"))
    file_line=${arr[0]}
    possible_errors_arr+=("${arr[1]}")

    ext=${file_line##*\.}
    filename=${file_line%\.*}
    ((solution_ind++))

    solutions_names[$solution_ind]="${file_line} |"
    dashes+=("+ $(eval printf '_%.0s' {1..${#file_line}} | tr '_' '-')")

    compile_result=$(buildFiles "${solutions}/${filename}" "${ext}")
    if [[ "${compile_result}" = "CE" ]]; then
        verdict=CE
        comment="Solution failed to compile"
    fi

    run_tests "$filename" "$ext" "$solution_ind" "$verdict" &

done < ${outer_files}/invoke_solutions.txt

wait

result_ind=0
while read line; do
    line=$(remove_trailing_spaces "${line}")
    arr=($(tr ' ' '\n' <<< "${line}"))
    possible_errors=($(tr '/' '\n' <<< "${possible_errors_arr[$result_ind]}"))

    test_ind=0
    cnt_ok=0
    cnt_success=0
    time_spent=${arr[2]}

    for verdict in ${arr[@]:3}; do
        test_color=""
        if [[ "${verdict}" = "OK" ]]; then
            # echo -e verdict=${GREEN}$verdict${NC}
            test_color=$GREEN
            ((cnt_ok++))
            ((cnt_success++))
        elif [[ "${possible_errors[@]}" =~ "${verdict}" ]]; then
            test_color=$BLUE
            ((cnt_success++))
            # echo -e verdict=${BLUE}$verdict${NC}
        else
            test_color=$RED
            # echo -e verdict=${RED}$verdict${NC}
            # echo $comment
        fi

        test_results[$test_ind]+=" ${test_color}${verdict}${NC} |"
        ((test_ind++))
    done
    time_spent_arr+=("$time_spent |")

    if [[ $cnt_success = $test_ind ]]; then
        result_arr+=("${GREEN}Pass${NC} |")
    else
        result_arr+=("${RED}Fail${NC} |")
    fi
    test_passed+=("${cnt_ok}/${test_ind} |")
    test_right+=("${cnt_success}/${test_ind} |")
    ((result_ind++))

done <<< $(sort -n $result)


for (( i=0; i < ${#test_results[*]}; i++ )); do
    test_results[$i]+='\n'
done

dashes+=(" +")

echo -e "${dashes[@]}
${solutions_names[@]}
${dashes[@]}
${test_results[@]}${dashes[@]}
${solutions_names[@]}
${dashes[@]}
${result_arr[@]}
${time_spent_arr[@]}
${test_right[@]}
${test_passed[@]}
${dashes[@]}" | column -t
