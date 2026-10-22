#include "utils.h"
#include <map>

class TStringReader {
private:
    std::stringstream ss;
public:
    TStringReader(const std::string& s) : ss(s) {}
    inline std::string read(char c = '\0') {
        std::string res;
        if (std::getline(ss, res, c)) {
            return res;
        } else {
            return "";
        }
    }
    inline void skip(char c) {
        read(c);
    }
};

constexpr double EPS = 1e-10;

struct THit {
    union {
        long long ll;
        double ld;
    } val;
    bool is_hit;
    bool is_set;
    bool is_const;
    bool is_int;
    THit()
        : val(0)
        , is_hit(false)
        , is_set(false)
        , is_const(true)
        , is_int(true)
    {}

    inline void add_hit() {
        if (is_const) {
            is_hit = true;
        }
    }

    template<typename T>
    inline void set_bound(T bound) {
        if (!is_const) {
            return;
        }
        if (!is_set) {
            is_int = std::is_integral_v<T>;
            if (is_int) {
                val.ll = (long long)bound;
            } else {
                val.ld = (double)bound;
            }
            is_set = true;
            return;
        }
        if (std::is_integral_v<T> != is_int) {
            is_const = false;
            return;
        }
        if (is_int) {
            if (val.ll != (long long)bound) {
                is_const = false;
            }
        } else {
            if (abs(val.ld - (double)bound) > EPS) {
                is_const = false;
            }
        }
    }

    inline void write_bound() const {
        if (!is_const) {
            std::cout << " ?";
        } else {
            if (is_int) {
                std::cout << " " << val.ll;
            } else {
                std::cout << " " << std::format("{:.6f}", val.ld);
            }
        }
    }
};

struct TBoundsValue {
    inline void add_hit(bool is_max) {
        if (is_max) {
            mx.add_hit();
        } else {
            mn.add_hit();
        }
    }

    template<typename T>
    inline void set_bound(bool is_max, T bound) {
        if (is_max) {
            mx.set_bound(bound);
        } else {
            mn.set_bound(bound);
        }
    }

    inline void write_hit_string(const std::string& name) const {
        if (!mn.is_set || !mx.is_set) {
            return;
        }
        std::string res;
        int cnt = 0;
        if (mn.is_const && mn.is_hit) {
            res += " min-value-hit";
            ++cnt;
        }
        if (mx.is_const && mx.is_hit) {
            res += " max-value-hit";
            ++cnt;
        }
        std::cout << (cnt == 2 ? "" : YELLOW) << name << ":" << res << NC << '\n';

    }

    inline void write_constant_bounds(const std::string& name) const {
        if (!mn.is_set || !mx.is_set) {
            return;
        }
        std::cout << "constant-bounds " << name << ":";
        mn.write_bound();
        mx.write_bound();
        std::cout << '\n';
    }

private:
    THit mn;
    THit mx;
};

struct TBounds {
private:
    std::map<std::string, TBoundsValue> mp;

    inline void add_var(const std::string& s) {
        if (mp.count(s) == 0) {
            mp[s] = TBoundsValue();
        }
    }

    inline void add_hit(const std::string& s, bool is_max) {
        mp[s].add_hit(is_max);
    }

    template<typename T>
    inline void set_bound(const std::string& s, bool is_max, T bound) {
        mp[s].set_bound(is_max, bound);
    }

public:
    inline void write_results() const {
        for (const auto& [key, val] : mp) {
            val.write_hit_string(key);
        }
        for (const auto& [key, val] : mp) {
            val.write_constant_bounds(key);
        }
        for (const auto& [key, val] : mp) {
            std::cout << "variable " << key << '\n';
        }
    }

    void read_bounds(const TPaths& paths) {
        std::ifstream logs(paths.validator_logs);
        std::vector<TStringReader> hits;
        std::vector<TStringReader> constant_bounds;
        std::string _row;
        while (std::getline(logs, _row)) {
            TStringReader row = TStringReader(_row);
            if (_row[0] == '\"') {
                hits.push_back(std::move(row));
            } else if (_row[0] == 'c') {
                row.skip(' ');
                constant_bounds.push_back(std::move(row));
            } else if (_row[0] == 'v') {
                row.skip(' ');
                add_var(row.read('\0'));
            } else {
                std::cerr << "Unknown bounds string: " << row.read() << std::endl;
                exit(1);
            }
        }
        logs.close();

        for (auto& i : hits) {
            std::string name = i.read(':');
            i.skip(' ');
            while (true) {
                std::string cur = i.read(' ');
                if (cur.empty()) {
                    break;
                }
                add_hit(name, cur[1] == 'a');
            }
        }

        for (auto& i : constant_bounds) {
            std::string name = i.read(':');
            i.skip(' ');

            std::string min_bound = i.read(' ');
            if (min_bound.find('.') != std::string::npos) {
                set_bound(name, false, std::stod(min_bound));
            } else {
                set_bound(name, false, std::stoll(min_bound));
            }

            std::string max_bound = i.read();
            if (max_bound.find('.') != std::string::npos) {
                set_bound(name, true, std::stod(max_bound));
            } else {
                set_bound(name, true, std::stoll(max_bound));
            }
        }
    }
};

int main() {
    TConfig config = NConfig::TConfig();
    TPaths paths = NPaths::TPaths();

    TBounds bounds;

    fs::path built_validator = NBuild::buildLine(paths, "validator");
    if (!NBuild::buildFiles(paths, config, "validator", "cpp")) {
        std::cout << RED << "Failed to compile validator.cpp" << NC << std::endl;
        NBuild::write_build_errors(paths);
    }

    NPaths::clear_file(paths.bounds_err);

    int tst_num = config.task.first_test_num;
    std::string group = "";
    for (const std::string& line : get_gen_strings(paths)) {
        if (line.starts_with("--group=")) {
            group = line.substr(8);
            continue;
        }
        fs::path val_in = paths.tests_tests / std::format("{:0>2}.tst", tst_num);
        int val_ret = run_validator(paths, built_validator, group, val_in, paths.bounds_err);
        bounds.read_bounds(paths);
        ++tst_num;
        if (!val_ret) {
            NBuild::write_errors(paths.bounds_err, "bounds_err", true);
        }
    }
    bounds.write_results();
}