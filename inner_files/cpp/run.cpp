#include "utils.h"


int main(int argc, char* argv[]) {
    std::string in_file;
    if (argc == 1) {  // run on custom test
        in_file = "\"" + paths.sol_in.string() + "\"";
    } else {
        std::string test = argv[1];
        in_file = std::format(
            "\"{}.tst\" 1> \"{}\"",
            (paths.tests_tests / test).string(),
            paths.sol_out.string()
        );
    }

    const std::string built_solution = NBuild::build_main(false);
    TTimer timer = TTimer();
    system(
        std::format(
            "{} < {} 2> \"{}\"",
            built_solution,
            in_file,
            paths.sol_err.string()
        ),
        "main." + config.solution.lang,
        paths.sol_err
    );
    timer.finish("Time", std::cout);

    if (argc > 1) {
        const fs::path built_checker = NBuild::build_checker();
        run_checker(argv[1], built_checker, paths.sol_out.string());
    }
}