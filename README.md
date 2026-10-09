# Local Polygon

It is an offline tool for creating competitive programming problems.
It helps preparing tasks for [Polygon](https://polygon.codeforces.com) locally. For compatibility purposes, this tool uses [testlib.h](https://github.com/MikeMirzayanov/testlib) and its standard checkers.
It can be used in WSL and Unix-like operating systems.

## New folder initialization

Copy the whole folder and run `make clear_all`.

## Directories explanation (after initialization)

- Basic files that will be used in Polygon are located in the root directory:
    - `generator.cpp`
    - `legend.tex`
    - `legend.html` (only for Contester)
    - `makefile`
    - `testlib.h` (used to build everything)
    - `tutorial.tex`
    - `validator.cpp`

- **checkers**
  Some of the most useful standard checkers from [testlib](https://github.com/MikeMirzayanov/testlib/tree/master/checkers) and an empty template for a custom checker.

- **inner_files**
  This directory contains temporary files, binary files and script files that help creating tasks. It is recommended not to touch them during problem development.

- **outer_files**
  The files to configure the problem:
    - `config.sh` — the main config file.
    - `gen_strings.txt` — lines to generate tests including group annotation, generator name, and generator arguments.
    - `invoke_solutions.txt` — the description of files to be run during invocation with possible verdicts.
    - `cpp_build.txt` — deprecated.

- **solutions**
  A folder for solutions and for the text files that are needed for running custom tests.

- **tests**
  A folder for tests
  - `./tests/tests` contains generated tests and answers.
  - `./tests/tests.txt` stores all tests, the main correct solution output and also checker and validator messages.
  - `./tests/validator` a folder for validator tests
    - First line of the test must contain `OK` or `FL`, which shows if the test should return `OK` or `Fail`.
    - If the the test needs to contain a group number it should be written in the first line after verdict and `/`.
    - All lines, starting from the second are treated as test.
    - Example of validator test file:
      ```
      OK/1
      3
      1 2 3
      ```


## Makefile targets

- `make clear_all` — should be run once when initializing the folder for a new problem.
- `make c_run` — run the main correct solution on a custom test (stored in `./solutions/in.txt`).
- `make c_run T=<test>` — run the main correct solution on a generated test `<test>.tst` and compare it to `<test>.ans` using the checker from the config.
- `make c_gen` — generate tests according to `./outer_files/gen_string.txt` using the main correct solution, validating with `validator.cpp` and checking with the checker specified in the config.
- `make invoke` — run files specified in `./outer_files/invoke_solutions.txt` on all generated tests.
- `make freemaker` — generate Freemaker rows from `./outer_files/gen_string.txt`.
- `make c_cont` — create archive for Contester.
- `make c_test_val` — run validator tests.
- `make help` — read [a more detailed explanation of this tool](inner_files/write_help.txt).

<details>
TODO

- Fix ML handling.
- Add support for interactive tasks.
- Add ability to upload to polygon.
- Add checker tests
</details>