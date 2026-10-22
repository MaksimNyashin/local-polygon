#include "utils.h"
#include <set>

int main(int argc, char* argv[]) {
    TConfig config = NConfig::TConfig();
    TPaths paths = NPaths::TPaths();
    NPaths::clear_file(paths.gen_err);
    NPaths::clear_file(paths.gen_tests);

    const int tst = (argc == 2 ? std::stoi(argv[1]) : -1);

    // Compile Main Correct Solution
    fs::path built_main = NBuild::buildLine(paths, config.solution.main);
    if (!NBuild::buildFiles(paths, config, config.solution.main, config.solution.lang, true)) {
        std::cout << RED << "Failed to compile " << config.solution.main << ".cpp" << NC << std::endl;
        NBuild::write_build_errors(paths);
    }

    // Compile Validator
    fs::path built_validator = NBuild::buildLine(paths, "validator");
    if (!NBuild::buildFiles(paths, config, "validator", "cpp")) {
        std::cout << RED << "Failed to compile validator.cpp" << NC << std::endl;
        NBuild::write_build_errors(paths);
    }

    int test_num = config.task.first_test_num;
    int total_num = 0;
    int group_id = 0;
    int suc_num = 0;
    std::string group = "example";
    std::set<std::string> failed_generators;
    std::set<std::string> compiled_generators;

    NBuild::createFolder(paths.tests_tests);

    // Generate tests
    for (const std::string& line : get_gen_strings(paths)){

        // Set group name
        if (line.substr(0, 8) == "--group=") {
            ++group_id;
            group = line.substr(8);
            std::cout << "________" << group << "________" << std::endl;
            continue;
        }

        size_t space_pos = line.find(' ');
        std::string gen_name = (space_pos != std::string::npos) ? line.substr(0, space_pos) : line;
        std::string gen_args = (space_pos != std::string::npos) ? line.substr(space_pos + 1) : "";


        // Continue if only one test is generated
        if (tst != -1 && test_num != tst) {
            ++test_num;
            continue;
        }

        // Compile generator if it wasn't already compiled
        if (failed_generators.count(gen_name)) {
            std::cout << RED << gen_name << ".cpp didn't compile successfully" << NC << std::endl;
            ++test_num;
            ++total_num;
            continue;
        }
        if (!compiled_generators.count(gen_name)) {
            if (!NBuild::buildFiles(paths, config, gen_name, "cpp")) {
                std::cout << RED << "Failed to compile " << gen_name << ".cpp" << NC << std::endl;
                failed_generators.insert(gen_name);
                ++test_num;
                ++total_num;
                continue;
            } else {
                compiled_generators.insert(gen_name);
            }
        }

        // Run Generator
        std::cout << line << std::endl;
        std::cout << "Test " << test_num << ":" << std::endl;
        TTimer gen_timer = TTimer();
        system(
            std::format(
                "\"{}/{}\" {} > \"{}\" 2>> \"{}\"",
                paths.builds.string(),
                gen_name,
                gen_args,
                paths.gen_in.string(),
                paths.gen_err.string()
            ),
            "generator",
            paths.gen_err
        );
        gen_timer.finish("    gen", std::cout);
        std::ofstream tests(paths.gen_tests, std::ios::app);
        tests << "----------in#" << test_num << "----------\n";
        cat_file(paths.gen_in, tests);

        // Run Validator
        tests << "----------val#" << test_num << "---------\n";
        tests.close();
        int val_ret = run_validator(paths, built_validator, group, paths.gen_in, paths.gen_tests);

        // Check Validator return code
        tests.clear();
        tests.open(paths.gen_tests, std::ios::app);
        if (!val_ret) {
            tests << "----------out#" << test_num << "---------\n";
            TTimer sol_timer = TTimer();
            if (config.solution.lang == "py") {
                system(
                    std::format(
                        "{} \"{}\" < \"{}\" > \"{}\" 2>> \"{}\"",
                        config.build.py_version,
                        (paths.home_dir / (config.solution.main + ".py")).string(),
                        paths.gen_in.string(),
                        paths.gen_out.string(),
                        paths.gen_err.string()
                    ),
                    "main.py",
                    paths.gen_err
                );
            } else {
                system(
                    std::format(
                        "\"{}\" < \"{}\" > \"{}\" 2>> \"{}\"",
                        built_main.string(),
                        paths.gen_in.string(),
                        paths.gen_out.string(),
                        paths.gen_err.string()
                    ),
                    "main.cpp",
                    paths.gen_err
                );
            }
            sol_timer.finish("    time", std::cout);
            cat_file(paths.gen_out, tests);

            // Copy test to tests folder
            std::string file_test_num = std::format("{:02}", test_num);
            fs::copy_file(paths.gen_in, paths.tests_tests / (file_test_num + ".tst"), std::filesystem::copy_options::overwrite_existing);
            fs::copy_file(paths.gen_out, paths.tests_tests / (file_test_num + ".ans"), std::filesystem::copy_options::overwrite_existing);
            ++suc_num;
        } else {
            std::cout << "    Validator check is failed\n";
        }
        ++total_num;
        tests << "----------end#" << test_num << "---------\n\n\n";
        ++test_num;

        tests.close();
    }

    std::cout << "\n________Validator results________\n";
    std::cout << (suc_num < total_num ? RED : GREEN) << "Successfully generated/validated  " << suc_num << "/" << total_num << "  tests\n\n" << NC;

    NBuild::write_gen_errors(paths, false);

    return 0;
}