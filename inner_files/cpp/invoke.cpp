#include "utils.h"
#include <algorithm>
#include <iomanip>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

void write_columns(const std::vector<std::vector<std::string>>& refList) {
    std::vector<std::size_t> col_wid;
    for (const auto& i : refList) {
        std::size_t sz = i.size();
        if (col_wid.size() < sz) {
            col_wid.resize(sz);
        }
        for (std::size_t j = 0; j < sz; ++j) {
            std::size_t line_size = i[j].size();
            if (i[j].find('\033') != std::string::npos) {
                line_size -= 11;
            }
            col_wid[j] = std::max(col_wid[j], line_size);
        }
    }

    auto dashes = [&col_wid]() {
        for (const auto& i : col_wid) {
            std::cout << "+" << std::string(i + 2, '-');
        }
        std::cout << "+\n";
    };

    std::cout << std::setfill(' ') << std::left;
    for (const auto& i : refList) {
        if (i.empty()) {
            dashes();
        } else {
            for (std::size_t j = 0; j < i.size(); ++j) {
                std::size_t wid = col_wid[j] + 1;
                if (i[j].find('\033') != std::string::npos) {
                    wid += 11;
                }
                std::cout << "| " << std::setw(wid) << i[j];
            }
            std::cout << "|\n";
        }
    }
}


struct TRunTest{
    std::string filename;
    std::string ext;
    std::string test;
    int file_ind;
    int test_ind;
};

enum class EVerdict {
    OK,
    WA,
    PE,
    TL,
    ML,
    RE,
    CE
};

const std::string verdicts[7] = {"OK", "WA", "PE", "TL", "ML", "RE", "CE"};

using TTestResult = std::map<std::pair<int ,int>, std::pair<EVerdict, double>>;
std::mutex tests_mutex;
std::mutex write_mutex;

void run_thread(
    const int ind,
    const TConfig& config,
    const TPaths& paths,
    const fs::path& checker,
    std::queue<TRunTest>& tests_queue,
    TTestResult& test_results
) {
    TRunTest test;
    fs::path out_file = paths.inv_tmp / (std::to_string(ind) + ".out");
    fs::path err_file = paths.inv_tmp / (std::to_string(ind) + ".err");
    double upd_tl = double(config.solution.tl + 1000) / 1000;
    double real_tl = double(config.solution.tl + 1000) / 1000;

    NPaths::clear_file(err_file);

    while (true) {
        // Read next test
        {
            std::lock_guard<std::mutex> lock(tests_mutex);
            if (tests_queue.empty()) {
                break;
            }
            test = tests_queue.front();
            tests_queue.pop();
        }

        // Run solution on the test
        TTimer timer = TTimer();
        int status;
        if (test.ext == "cpp") {
            status = std::system(
                std::format(
                    "ulimit -v {}; timeout {}s {} < {} 1> {} 2>>{}",
                    config.solution.ml * 1024,
                    upd_tl,
                    NBuild::buildLine(paths, "solutions/" + test.filename).string(),
                    (paths.tests_tests / (test.test + ".tst")).string(),
                    out_file.string(),
                    err_file.string()
                ).c_str()
            );
        } else if (test.ext == "py") {
            status = std::system(
                std::format(
                    "ulimit -v {}; timeout {}s {} {}.py < {} 1> {} 2>>{}",
                    config.solution.ml * 1024,
                    upd_tl,
                    config.build.py_version,
                    (paths.solutions / test.filename).string(),
                    (paths.tests_tests / (test.test + ".tst")).string(),
                    out_file.string(),
                    err_file.string()
                ).c_str()
            );
        }
        double tl = timer.stop();

        // Set verdicts
        EVerdict verdict = EVerdict::OK;
        if (status == 137) {
            verdict = EVerdict::ML;
        } else if (status == 124) {
            verdict = EVerdict::TL;
        } else if (tl > real_tl) {
            verdict = EVerdict::TL;
        } else if (status != 0) {
            verdict = EVerdict::RE;
        } else {  // Run Checker for OK, WA or PE
            int checker_status = std::system(
                std::format(
                    "{} --testset tests --group 0 \"{}.tst\" \"{}\" \"{}.ans\"",
                    checker.string(),
                    (paths.tests_tests / test.test).string(),
                    out_file.string(),
                    (paths.tests_tests / test.test).string()
                ).c_str()
            );
            if (checker_status == 256) {
                verdict = EVerdict::WA;
            } else if (checker_status == 512) {
                verdict = EVerdict::PE;
            }
        }

        // Save test result
        {
            std::lock_guard<std::mutex> lock(write_mutex);
            test_results[{test.file_ind, test.test_ind}] = {verdict, tl};
        }
    }
}


int main() {
    TConfig config = NConfig::read_config();
    TPaths paths = NPaths::get_paths();

    NBuild::createFolder(paths.inv_tmp);

    std::vector<std::vector<std::string>> result(3);

    // Read names of all tests
    std::vector<fs::path> test_names;
    for (const auto& test_filename : fs::directory_iterator(paths.tests_tests)) {
        if (test_filename.path().extension() == ".tst") {
            std::string testname = test_filename.path().stem().string();
            fs::path ans_filename = test_filename;
            ans_filename.replace_extension(".ans");
            if (fs::exists(ans_filename)) {
                result.push_back({testname});
                ans_filename.replace_extension();
                test_names.push_back(ans_filename);
            }
        }
    }

    // Compile Checker
    std::string checker = "checkers/" + config.solution.checker;
    fs::path built_checker = NBuild::buildLine(paths, checker);
    if (!NBuild::buildFiles(paths, config, checker, "cpp")) {
        std::cout << RED << "Failed to compile " << checker <<".cpp" << NC << std::endl;
        exit(1);
    }

    NPaths::clear_file(paths.inv_err);

    std::map<std::string, EVerdict> verdicts_map;
    for (int i = 0; i < 7; ++i) {
        verdicts_map[verdicts[i]] = EVerdict(i);
    }


    // Read solution names and possible verdicts
    std::vector<std::tuple<std::string, std::string, int>> solutions;
    {
        std::ifstream fi(paths.inv_solutions);
        std::string solution;
        while (std::getline(fi, solution)) {
            solution = remove_trailing_spaces(solution);
            if (solution.size() == 0 || (solution[0] == '#' && solution[1] == ' ')) {
                continue;
            }
            std::stringstream ss(solution);
            std::string sol_name;
            std::getline(ss, sol_name, '.');
            std::string sol_ext;
            std::getline(ss, sol_ext, ' ');
            int mask = 0;
            std::string verdict;
            while (std::getline(ss, verdict, '/')) {
                auto f = verdicts_map.find(verdict);
                if (f != verdicts_map.end()) {
                    mask |= (1 << (int)(f->second));
                }
            }
            solutions.emplace_back(sol_name, sol_ext, mask);
        }
    }

    TTestResult test_result;
    // Fill tests queue
    std::queue<TRunTest> tests_queue;
    for (std::size_t sol_ind = 0; sol_ind < solutions.size(); ++ sol_ind) {
        if (!NBuild::buildFiles(
            paths,
            config,
            "solutions/" + std::get<0>(solutions[sol_ind]),
            std::get<1>(solutions[sol_ind])
        )) {
            for (std::size_t test_ind = 3; test_ind < result.size(); ++test_ind) {
                test_result[{sol_ind, test_ind}] = {EVerdict::CE, 0};
            }
        }
        for (std::size_t test_ind = 3; test_ind < result.size(); ++test_ind) {
            tests_queue.emplace(
                std::get<0>(solutions[sol_ind]),
                std::get<1>(solutions[sol_ind]),
                result[test_ind][0],
                sol_ind,
                test_ind
            );
        }
    }

    // Create testing threads
    std::vector<std::thread> threads;
    for (std::size_t thread_ind = 0; thread_ind < config.build.thread_num; ++ thread_ind) {
        threads.emplace_back(
            run_thread,
            thread_ind,
            std::cref(config),
            std::cref(paths),
            std::cref(built_checker),
            std::ref(tests_queue),
            std::ref(test_result)
        );
    }

    // Join testing threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Initialize output columns
    std::size_t st_ind = result.size();
    result.insert(result.end(), {{}, {"total"}, {"time"}, {"passed"}, {"right"}, {}});
    std::vector<std::string>& solutions_names = result[1];
    solutions_names.push_back("result");
    std::vector<std::string>& result_arr = result[st_ind + 1];
    std::vector<std::string>& time_spent_arr = result[st_ind + 2];
    std::vector<std::string>& test_right = result[st_ind + 3];
    std::vector<std::string>& test_passed = result[st_ind + 4];

    // Fill output columns
    for (std::size_t sol_ind = 0; sol_ind < solutions.size(); ++sol_ind) {
        const auto& [sol, ext, mask] = solutions[sol_ind];
        solutions_names.push_back(sol + "." + ext);
        std::size_t right_tests = 0, passed_tests = 0, total_tests = 0;
        double max_tl = 0;

        for (std::size_t test_ind = 3; test_ind < st_ind; ++test_ind) {
            auto [verdict, tl] = test_result[{sol_ind, test_ind}];
            std::string col;
            if (verdict == EVerdict::OK) {
                col = GREEN;
                ++passed_tests;
            } else if ((mask >> (int)verdict) & 1) {
                col = BLUE;
                ++right_tests;
            } else {
                col = RED;
            }
            ++total_tests;
            result[test_ind].push_back(col + verdicts[std::size_t(verdict)] + NC);
            max_tl = std::max(max_tl, tl);
        }
        right_tests += passed_tests;
        if (total_tests == passed_tests) {
            result_arr.push_back(GREEN + "AC" + NC);
        } else if (total_tests == right_tests) {
            result_arr.push_back(BLUE + "Pass" + NC);
        } else {
            result_arr.push_back(RED + "Fail" + NC);
        }
        std::string tl_col = GREEN;
        if (max_tl * 500 > (double)config.solution.tl) {
            tl_col = RED;
        } else if (max_tl * 1000 > (double)config.solution.tl) {
            tl_col = YELLOW;
        } else if (max_tl * 2000 > (double)config.solution.tl) {
            tl_col = BLUE;
        }
        time_spent_arr.push_back(std::format("{}{:.3f}{}", tl_col, max_tl, NC));
        test_right.push_back(std::format("{}/{}", right_tests, total_tests));
        test_passed.push_back(std::format("{}/{}", passed_tests, total_tests));
    }


    for (int thread_ind = 0; thread_ind < config.build.thread_num; ++thread_ind) {
        fs::path err_file = paths.inv_tmp / (std::to_string(thread_ind) + ".err");
        std::cout << "\n-------------err#" << thread_ind << "-------------\n";
        if (fs::is_empty(err_file)) {
            std::cout << BLUE << "empty" << '\n';
        } else {
            std::cout << RED;
            cat_file(err_file, std::cout);
        }
        std::cout << NC;
    }
    // std::cout << "\n-------------------------------\n";

    write_columns(result);

    return 0;
}
