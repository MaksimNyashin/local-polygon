#include "utils.h"


int main(int argc, char* argv[]) {
    TConfig config = NConfig::TConfig();
    TPaths paths = NPaths::TPaths();

    std::string in_file;
    if (argc == 1) {  // run on custom test
        in_file = paths.sol_in.string();
    } else {
        std::string test = argv[1];
        in_file = std::format(
            "{}.tst 1> {}",
            (paths.tests_tests / test).string(),
            paths.sol_out.string()
        );
    }

    if (config.solution.lang == "py") {
        TTimer timer = TTimer();
        std::system(
            std::format(
                "{} ./{}.py < {} 2>> {}",
                config.build.py_version,
                config.solution.main,
                in_file,
                paths.sol_err.string()
            ).c_str()
        );
        timer.finish("Time", std::cout);
    } else if (config.solution.lang == "cpp") {
        if (!NBuild::buildFiles(paths, config, config.solution.main, "cpp")) {
            std::cout << RED << "Failed to compile " << config.solution.main << ".cpp" << NC << std::endl;
            NBuild::write_build_errors(paths);
        }
        fs::path built_solution = NBuild::buildLine(paths, config.solution.main);
        TTimer timer = TTimer();
        std::system(
            std::format(
                "{} < {} 2>> {}",
                built_solution.string(),
                in_file,
                paths.sol_err.string()
            ).c_str()
        );
        timer.finish("Time", std::cout);
    }

    if (argc > 1) {
        std::string checker = "checkers/" + config.task.checker;
        fs::path built_checker = NBuild::buildLine(paths, checker);
        if (!NBuild::buildFiles(paths, config, checker, "cpp")) {
            std::cout << RED << "Failed to compile " << checker <<".cpp" << NC << std::endl;
            NBuild::write_build_errors(paths);
        }
        run_checker(paths, argv[1], built_checker, paths.sol_out.string());
    }
}