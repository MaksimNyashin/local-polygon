#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace NConfig {

struct TConfigSolution {
    std::string main;
    std::string lang;
    int first_test_num;
    std::string checker;
    int tl;
    bool is_interactive;
};

struct TConfigBuild {
    std::string cpp_options;
    std::string cpp_version;
    std::string py_version;
};

struct TConfig {
    TConfigSolution solution;
    TConfigBuild build;

    int is_cont;
};

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

TConfig read_config() {
    TConfig result;
    result.solution.main = read_string("main");
    result.solution.lang = read_string("lang");
    result.solution.first_test_num = read_int("first_test_num");
    result.solution.checker = read_string("checker");
    result.solution.tl = read_int("tl");
    result.solution.is_interactive = read_int("is_interactive");

    result.build.cpp_options = read_string("cpp_options");
    result.build.cpp_version = read_string("cpp_version");
    result.build.py_version = read_string("py_version");

    result.is_cont = read_int("is_cont");
    return result;
};

} // NConfig

using TConfig = NConfig::TConfig;



namespace NPaths {

struct TPaths {
    fs::path my_dir;
    fs::path home_dir;
    fs::path inner_files;
    fs::path outer_files;
    fs::path test_dir;

    fs::path build_err;
    fs::path builds;
    fs::path gen_err;
    fs::path gen_in;
    fs::path gen_out;
    fs::path validator_logs;

    fs::path gen_strings;

    fs::path gen_tests;
    fs::path tests_tests;
};

void clear_file(const fs::path& path) {
    std::ofstream file(path, std::ios::trunc);
    if (file.is_open()) {
        file.close();
    }
}

struct TPaths get_paths() {
    TPaths result;
    result.my_dir = fs::path(__FILE__).parent_path();
    result.home_dir = result.my_dir.parent_path().parent_path();
    result.inner_files = result.home_dir / "inner_files";
    result.outer_files = result.home_dir / "outer_files";
    result.test_dir = result.home_dir / "tests";

    result.build_err = result.inner_files / "build_err.txt";
    result.builds = result.inner_files / "builds";
    result.gen_err = result.inner_files / "_err";
    result.gen_in = result.inner_files / "in.txt";
    result.gen_out = result.inner_files / "out.txt";
    result.validator_logs = result.inner_files / "validator_logs.txt";

    result.gen_strings = result.outer_files / "gen_strings.txt";

    result.gen_tests = result.test_dir / "tests.txt";
    result.tests_tests = result.test_dir / "tests";

    clear_file(result.build_err);
    clear_file(result.gen_err);
    clear_file(result.gen_tests);

    return result;
}

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
    fs::file_time_type res;
    if (fs::exists(path)) {
        res = fs::last_write_time(path);
    } else {
        res = fs::file_time_type();
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
                !std::system(
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
                std::cout << file_path << " is built successfully" << std::endl;
            } else {
                std::cout << "CE" << std::endl;
                return false;
            }
        } else {
            std::cout << "The " << file_path << " binary is already up-to-date" << std::endl;
        }
    }
    return true;
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



const std::string RED = "\033[0;31m";
const std::string GREEN = "\033[0;32m";
const std::string BLUE="\033[0;34m";
const std::string NC="\033[0m"; // No Color


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
private:
    std::chrono::high_resolution_clock::time_point start;
};


void cat_file(const fs::path& path, std::ofstream& os) {
    std::ifstream tmp(path);
    std::string line;
    while (std::getline(tmp, line)) {
        os << line << '\n';
    }
    tmp.close();
}