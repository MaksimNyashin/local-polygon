inner_files=./inner_files

build:
	@${inner_files}/build_all.sh

gen:
	@${inner_files}/run_gen.sh

run:
	@${inner_files}/run_solutions.sh

invoke:
	@${inner_files}/invokation/invoke.sh

freemaker:
	@${inner_files}/write_freemaker.sh > ./freemaker.txt

help:
	@less ${inner_files}/write_help.txt

clear_all:
	@${inner_files}/init_folder.sh

c_gen:
	@${inner_files}/run_cpp.sh run_gen

c_invoke:
	@${inner_files}/run_cpp.sh invoke

c_run:
	@(${inner_files}/run_cpp.sh run $(T))

c_cont:
	@${inner_files}/run_cpp.sh cont_builder

c_test_val:
	@${inner_files}/run_cpp.sh validator_check
