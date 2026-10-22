#include "utils.h"
#include <cassert>

enum class EValidatorVerdict {
    OK,
    FL
};

constexpr std::string verdict_to_str[2]{"OK", "Fail"};

int main(int argc, char* argv[]) {
    bool write_all = (argc == 2);

    // Compile validator
    const fs::path built_validator = NBuild::build_validator();

    std::vector<EValidatorVerdict> verdicts;
    // Iterate over tests
    size_t good = 0, total = 0;
    for (const auto& test_filename : fs::directory_iterator(paths.tests_validator_dir)) {
        if (test_filename.path().filename() == ".gitkeep") {
            continue;
        }
        // Get verdict
        const std::string filename = test_filename.path().filename().string();
        std::ifstream test(test_filename.path().string().c_str());
        std::string verdict_protostr;
        std::getline(test, verdict_protostr);
        if (verdict_protostr.back() == '\r') {
            verdict_protostr.pop_back();
        }
        assert(verdict_protostr.size() >= 2);
        std::string verdict_str = verdict_protostr.substr(0, 2);
        EValidatorVerdict expected;
        if (verdict_str == "OK") {
            expected = EValidatorVerdict::OK;
        } else if (verdict_str == "FL"){
            expected = EValidatorVerdict::FL;
        } else  {
            std::cerr << "Wrong verdict at file " << filename << ": \"" << verdict_str << "\"" << std::endl;
            exit(1);
        }

        // Get group
        std::string group = "";
        if (verdict_protostr.size() > 3 && verdict_protostr[2] == '/') {
            group = verdict_protostr.substr(3);
        }

        // Get test
        std::string text;
        std::string tmp;
        while (std::getline(test, tmp, '\r')) {
            text += tmp;
        }
        test.close();
        assert(text.back() == '\n');

        if (write_all) {
            verdicts.push_back(expected);
            std::cout << text << "===\n";
        } else {
            // Run validator
            int val_ret = std::system(
                std::format(
                    "\"{}\" --testset tests --group \"{}\" <<EOF 2> \"{}\"\n{}EOF",
                    built_validator.string(),
                    group,
                    paths.check_val_err.string(),
                    text
                ).c_str()
            );
            EValidatorVerdict result;
            if (val_ret == 0) {
                result = EValidatorVerdict::OK;
            } else if (val_ret == 768) {
                result = EValidatorVerdict::FL;
            } else {
                std::cerr << RED << "Error: " << NC << " validator_return = " << val_ret << std::endl;
                NBuild::write_errors(paths.check_val_err, filename, false);
                exit(val_ret);
            }

            // Output result
            std::cerr << filename << ":\t";
            if (result == expected) {
                std::cerr << GREEN;
                ++good;
            } else {
                std::cerr << RED;
            }
            ++total;
            std::cerr << verdict_to_str[(int)result] << NC << '\n';
            NBuild::write_errors(paths.check_val_err, filename, false);
            std::cerr << '\n';
        }
    }
    if (!write_all) {
        std::cerr << (good == total ? GREEN : RED) << "Tests passed: " << good << "/" << total << NC << std::endl;
    } else {
        for (auto v : verdicts) {
            if (v == EValidatorVerdict::OK) {
                std::cout << "VALID\n";
            } else {
                std::cout << "INVALID\n";
            }
        }
    }
    return 0;
}
