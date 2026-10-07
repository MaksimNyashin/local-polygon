#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

// const std::string RED = "\033[0;31m";
const std::string RED = "\033[0;91m";
const std::string GREEN = "\033[0;32m";
const std::string BLUE = "\033[0;34m";
const std::string YELLOW = "\033[0;33m";
const std::string NC = "\033[0m"; // No Color



namespace NConfig {

int read_int(const std::string& str) {
    char* res = std::getenv(str.c_str());
    if (res != nullptr) {
        return std::atoi(res);
    } else {
        return -1;
    }
}

std::string read_string(const std::string& str) {
    char* res = std::getenv(str.c_str());
    if (res != nullptr) {
        return res;
    } else {
        return "";
    }
}

struct TConfigSolution {
    std::string main;
    std::string lang;

    TConfigSolution()
        : main(read_string("main"))
        , lang(read_string("lang"))
    {}
};

struct TConfigTask {
    std::string task_name;
    std::string checker;
    int tl;
    int ml;
    int first_test_num;
    bool is_interactive;

    TConfigTask()
        : task_name(read_string("task_name"))
        , checker(read_string("checker"))
        , first_test_num(read_int("first_test_num"))
        , tl(read_int("tl"))
        , ml(read_int("ml"))
        , is_interactive(read_int("is_interactive"))
    {}
};

struct TConfigBuild {
    std::string cpp_options;
    std::string cpp_version;
    std::string py_version;
    int thread_num;

    TConfigBuild()
        : cpp_options(read_string("cpp_options"))
        , cpp_version(read_string("cpp_version"))
        , py_version(read_string("py_version"))
        , thread_num(read_int("thread_num"))
    {}
};

struct TConfig {
    TConfigSolution solution;
    TConfigTask task;
    TConfigBuild build;

    std::string contest;
    std::string contest_let;

    TConfig()
        : solution()
        , task()
        , build()
        , contest(read_string("contest"))
        , contest_let(read_string("contest_let"))
    {}
};

} // NConfig

using TConfig = NConfig::TConfig;



void cat_file(const fs::path& path, std::ostream& os) {
    std::ifstream tmp(path);
    std::string line;
    while (std::getline(tmp, line)) {
        os << line << '\n';
    }
    tmp.close();
}


namespace NPaths {

void clear_file(const fs::path& path) {
    if (!fs::exists(path)) {
        return;
    }
    std::ofstream file(path, std::ios::trunc);
    if (file.is_open()) {
        file.close();
    }
}

struct TPaths {
    fs::path my_dir;
    fs::path home_dir;
    fs::path checkers;
    fs::path inner_files;
    fs::path outer_files;
    fs::path solutions;
    fs::path test_dir;
    fs::path cont_dir;
    fs::path checkers_cont;

    fs::path build_err;
    fs::path builds;
    fs::path gen_err;
    fs::path gen_in;
    fs::path gen_out;
    fs::path inv;
    fs::path validator_logs;

    fs::path gen_strings;
    fs::path inv_solutions;

    fs::path gen_tests;
    fs::path tests_tests;

    fs::path inv_tmp;

    fs::path sol_in;
    fs::path sol_out;
    fs::path sol_err;

    fs::path cont_tests;

    TPaths()
        : my_dir(fs::path(__FILE__).parent_path())
        , home_dir(my_dir.parent_path().parent_path())
        , checkers(home_dir / "checkers")
        , inner_files(my_dir.parent_path())
        , outer_files(home_dir / "outer_files")
        , solutions(home_dir / "solutions")
        , test_dir(home_dir / "tests")
        , cont_dir(home_dir / "cont" / "to_arch")
        , checkers_cont(checkers / "cont" / "contlib.h")

        , build_err(inner_files / "build_err.txt")
        , builds(inner_files / "builds")
        , gen_err(inner_files / "_err")
        , gen_in(inner_files / "in.txt")
        , gen_out(inner_files / "out.txt")
        , inv(inner_files / "invokation")
        , validator_logs(inner_files / "validator_logs.txt")

        , gen_strings(outer_files / "gen_strings.txt")
        , inv_solutions(outer_files / "invoke_solutions.txt")

        , gen_tests(test_dir / "tests.txt")
        , tests_tests(test_dir / "tests")

        , inv_tmp(inv / "tmp")

        , sol_in(solutions / "in.txt")
        , sol_out(solutions/ "out.txt")
        , sol_err(solutions / "err.txt")

        , cont_tests(cont_dir / "tests")
    {
        clear_file(build_err);
    }
};

} // NPaths

using TPaths = NPaths::TPaths;



namespace NBuild {

fs::path buildLine(const TPaths& paths, std::string name) {
    for (char& c : name) {
        if (c == '/' || c == '\\') {
            c = '_';
        }
    }
    return paths.builds / name;
}

fs::file_time_type builtTime(const fs::path& path) {
    using namespace std::chrono_literals;
    fs::file_time_type res;
    if (fs::exists(path)) {
        res = fs::last_write_time(path);
    } else {
        auto def_date = std::chrono::sys_days{1970y / std::chrono::January / 1d};
        res = std::chrono::clock_cast<std::chrono::file_clock>(def_date);
    }
    // auto sctp = std::chrono::clock_cast<std::chrono::system_clock>(res);
    // std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
    // std::cout << path.string() << ' ' << std::ctime(&cftime) << std::endl;
    return res;
}

inline bool needsUpdate(const TPaths& paths, const std::string& name) {
    return builtTime(buildLine(paths, name)) < builtTime(paths.home_dir / (name + ".cpp"));
}

inline void createFolder(const fs::path& path) {
    fs::create_directories(path);
}

bool buildFiles(const TPaths& paths, const TConfig& config, const std::string& file_path, const std::string& lang) {
    if (lang == "cpp") {
        if (needsUpdate(paths, file_path)) {
            createFolder(paths.builds);
            if (
                std::system(
                    std::format(
                        "g++ {} {} -o {} {}.cpp 2>> {}",
                        config.build.cpp_version,
                        config.build.cpp_options,
                        buildLine(paths, file_path).string(),
                        (paths.home_dir / file_path).string(),
                        (paths.build_err).string()
                    ).c_str()
                )
            ) {
                std::cout << RED << file_path << "." << lang << ": CE" << NC << std::endl;
                return false;
            } else {
                std::cout << BLUE << file_path << "." << lang << " is built successfully" << NC << std::endl;
            }
        // } else {
            // std::cout << "The " << file_path << " binary is already up-to-date" << std::endl;
        }
    }
    return true;
}

inline void write_errors(const fs::path& err_file, const std::string& filename, const int first_line, bool ex) {
    if (!fs::is_empty(err_file)) {
        std::cout << std::string(first_line, '-') << filename << std::string(first_line, '-') << RED << '\n';
        cat_file(err_file, std::cout);
        std::cout << NC << std::string(31, '-') << std::endl;
        if (ex) {
            exit(1);
        }
    }
}

inline void write_build_errors(const TPaths& paths, bool ex = true) {
    write_errors(paths.build_err, "build_err", 11, ex);
}

inline void write_gen_errors(const TPaths& paths, bool ex = true) {
    write_errors(paths.gen_err, "gen_err", 12, ex);
}

} // NBuild



std::string remove_trailing_spaces(const std::string& s) {
    std::size_t cnt = 0;
    while (cnt < s.size() && std::isspace(s[cnt])) {
        ++cnt;
    }
    std::string res =  s.substr(cnt);
    if (!res.empty() && res.back() == '\r') {
        res.pop_back();
    }
    return res;
}





class TTimer {
public:
    TTimer()
    : start(std::chrono::high_resolution_clock::now())
    {}

    void finish(const std::string& out_line, std::ostream& os) {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        os << out_line << ": " << diff.count() * 1000 << "ms" << std::endl;
    }

    double stop() {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        return diff.count();
    }
private:
    std::chrono::high_resolution_clock::time_point start;
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

EVerdict run_checker(const TPaths& paths, const std::string& test, const fs::path& checker, const fs::path& out_file) {
    int checker_status = std::system(
        std::format(
            "{} --testset tests --group 0 \"{}.tst\" \"{}\" \"{}.ans\"",
            checker.string(),
            (paths.tests_tests / test).string(),
            out_file.string(),
            (paths.tests_tests / test).string()
        ).c_str()
    );
    if (checker_status == 256) {
        return EVerdict::WA;
    } else if (checker_status == 512) {
        return EVerdict::PE;
    }
    return EVerdict::OK;
}
