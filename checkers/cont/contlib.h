#ifndef CONTLIB_H
#define CONTLIB_H

#include <algorithm>
#include <climits>
#include <cstring>
#include <fstream>
#include <stdarg.h>
#include <string>
#include <vector>

#include <iostream>

#define AC 0xAC
#define WA 0xAB
#define PE 0xAA
#define FL 0xA3

#ifdef _MSC_VER
#   define NORETURN __declspec(noreturn)
#elif defined __GNUC__
#   define NORETURN __attribute__ ((noreturn))
#else
#   define NORETURN
#endif

enum EResult {
    _ok = 0,
    _wa = 1,
    _pe = 2,
    _fail = 3,
    _dirt = 4,
    _points = 5,
    _unexpected_eof = 8,
    _partially = 16
};

#define LF ((char)10)
#define CR ((char)13)
#define TAB ((char)9)
#define SPACE ((char)' ')
#define EOFC (255)

template<typename T>
static inline T __cont_abs(const T& val) {
    return val < 0 ? -val : val;
}

template<typename T>
static inline T __cont_max(const T& l, const T& r) {
    return l < r ? r : l;
}

template<typename T>
static inline T __cont_min(const T& l, const T& r) {
    return l < r ? l : r;
}

template<typename T>
static std::string vtos(const T& t) {
    if (t == 0) {
        return "0";
    } else {
        T n(t);
        bool neg = false;
        std::string s;
        if (n < 0) {
            neg = true;
            n = -n;
        }
        while (n != 0) {
            s.push_back(n % 10 + '0');
            n /= 10;
        }
        if (neg) {
            s.push_back('-');
        }
        std::reverse(s.begin(), s.end());
        return s;
    }
}

static bool __cont_isNaN(double r) {
    return ((r != r) == true) && ((r == r) == false) && ((1.0 > r) == false) && ((1.0 < r) == false);
}

static bool __cont_isInfinite(double r) {
    volatile double ra = r;
    return (ra > 1E300 || ra < -1E300);
}

inline std::string englishEnding(int x) {
    x %= 100;
    if (x / 10 == 1)
        return "th";
    if (x % 10 == 1)
        return "st";
    if (x % 10 == 2)
        return "nd";
    if (x % 10 == 3)
        return "rd";
    return "th";
}

enum EMode {
    _input,
    _output,
    _answer
};

template<typename C>
inline bool isBlanks(C c) {
    // std::cerr << "blanc: " << c + 0 << std::endl;
    return (c == LF || c == CR || c == SPACE || c == TAB);
}

inline std::string trim(std::string& s) {
    if (s.empty())
        return s;

    int left = 0;
    while (left < int(s.length()) && isBlanks(s[left]))
        left++;
    if (left >= int(s.length()))
        return "";

    int right = int(s.length()) - 1;
    while (right >= 0 && isBlanks(s[right]))
        right--;
    if (right < 0)
        return "";

    return s.substr(left, right - left + 1);
}


static std::string compress(const std::string& s) {
    std::string t;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] != '\0') {
            t += s[i];
        }
        else {
            t += '~';
        }
    }
    if (t.length() <= 64) {
        return t;
    } else {
        return t.substr(0, 30) + "..." + t.substr(s.length() - 31, 31);
    }
}

template<typename T>
static std::string toHumanReadableString(const T &n) {
    if (n == 0)
        return vtos(n);
    int trailingZeroCount = 0;
    T n_ = n;
    while (n_ % 10 == 0)
        n_ /= 10, trailingZeroCount++;
    if (trailingZeroCount >= 7) {
        if (n_ == 1)
            return "10^" + vtos(trailingZeroCount);
        else if (n_ == -1)
            return "-10^" + vtos(trailingZeroCount);
        else
            return vtos(n_) + "*10^" + vtos(trailingZeroCount);
    } else
        return vtos(n);
}

NORETURN void quit(EResult result, const char* msg);

class TFileReader {
private:
    std::FILE* file;
    std::string name;

    char* buffer;
    size_t buffer_size;
    size_t buffer_pos;


    const static size_t MAX_BUFFER_SIZE = 1 << 21;

    bool refill() {
        if (file == nullptr) {
            quit(_fail, ("TFileReader: file == nullptr (" + get_name() + ")").c_str());
        }
        if (buffer_pos < buffer_size) {
            return true;
        }
        size_t read_size = fread(buffer, 1, MAX_BUFFER_SIZE, file);

        if (read_size < MAX_BUFFER_SIZE && ferror(file)) {
            perror("Ferror details");
            quit(_fail, ("TFileReader: unable to read(" + get_name() + ")").c_str());
        }

        buffer_size = read_size;
        buffer_pos = 0;

        return read_size > 0;
    }

    std::string get_name() {
        return name;
    }

public:
    TFileReader(std::FILE* file, const std::string& name)
        : file(file)
        , name(name)
        , buffer(nullptr)
        , buffer_size(0)
        , buffer_pos(0)
    {
        buffer = new char[MAX_BUFFER_SIZE];
        refill();
    }

    char cur_char() {
        if (!refill()) {
            return EOFC;
        }
        return buffer[buffer_pos];
    }

    char next_char() {
        if (!refill()) {
            return EOFC;
        }
        return buffer[++buffer_pos];
    }

    void skip_char() {
        ++buffer_pos;
    }

    bool is_eof() {
        return !refill() || cur_char() == EOFC;
    }

    ~TFileReader() {
        if (file != nullptr) {
            fclose(file);
            file = nullptr;
        }
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
        }
    }
};


struct TInStream {
public:
    TFileReader* reader;
    EMode mode;

    size_t max_file_size;
    size_t max_token_length;

    TInStream();

    void init(const std::string& filename, EMode mode);

    void skipBlanks();

    int readInt(int min_val, int max_val, const std::string& var_name = "");
    std::vector<int> readInts(size_t size, int min_val, int max_val, const std::string& var_name = "");

    long long readLong(long long min_val, long long max_val, const std::string& var_name = "");
    long long readLong(const std::string& var_name = "");
    std::vector<long long> readLongs(size_t size, long long min_val, long long max_val, const std::string& var_name = "");

    double readReal();
    double readDouble();

    std::string readWord();

    bool seekEof();

    NORETURN void quit(EResult result, const char* msg);

    NORETURN void quitf(EResult result, const char* format, ...);

    void close();

private:
    std::string read_token();

    long long token_to_ll(const std::string& token);

    double string_to_double(const std::string& token);

    int _readInt();
};

TInStream inf;
TInStream ouf;
TInStream ans;


const size_t MESSAGE_SIZE = 1024;
char __quit_message[MESSAGE_SIZE];

#define __cont_read_many(read_many, read_one, typeName)                 \
    if (size < 0) {                                                     \
        quit(_fail, #read_many ": size should be non-negative.");       \
    }                                                                   \
    if (size > 100000000) {                                             \
        quit(_fail, #read_many ": size should be at most 100000000.");  \
    }                                                                   \
    std::vector<typeName> result(size);                                 \
    for (int i = 0; i < size; ++i) {                                    \
        result[i] = read_one;                                           \
    }                                                                   \
    return result;                                                      \

#define FMT_TO_RESULT(fmt, cstr, result) va_list ap;    \
    va_start(ap, fmt);                                  \
    vsnprintf(__quit_message, MESSAGE_SIZE, cstr, ap);  \
    va_end(ap);                                         \

std::string TInStream::read_token() {
    skipBlanks();

    char cur = reader->cur_char();

    if (cur == EOFC) {
        quit(_unexpected_eof, "Unexpected end of file - token expected");
    }

    std::string result;
    while (!(isBlanks(cur) || cur == EOFC)) {
        result.push_back(cur);
        if (result.length() > max_token_length) {
            quitf(_pe, "Length of token exceeds %d, token is '%s...'", int(max_token_length), compress(result).c_str());
        }
        cur = reader->next_char();
    }

    return result;
}

long long TInStream::token_to_ll(const std::string& token) {
    size_t len = token.size();
    if (len == 0 || len > 20) {
        this->quit(_pe, ("Expected integer, but\"" + compress(token) + "\" found").c_str());
    }

    bool has_minus = (len > 1 && token[0] == '-');
    int zeros = 0;
    bool leading_zeros = true;

    for (int i = int(has_minus); i < int(len); ++i) {
        if (token[i] == '0' && leading_zeros) {
            ++zeros;
        } else {
            leading_zeros = false;
        }
        if (token[i] < '0' || token[i] > '9') {
            this->quit(_pe, ("Expected integer, but \"" + compress(token) + "\" found").c_str());
        }
    }

    long long result;
    try {
        result = std::stoll(token.c_str());
    } catch (const std::exception&) {
        this->quit(_pe, ("Expected integer, but \"" + compress(token) + "\" found").c_str());
    } catch (...) {
        this->quit(_pe, ("Expected integer, but \"" + compress(token) + "\" found").c_str());
    }

    if (zeros > 0 && (result != 0 || has_minus) || zeros > 1) {
        this->quit(_pe, ("Expected integer, but \"" + compress(token) + "\" found").c_str());
    }

    return result;
}

double TInStream::string_to_double(const std::string& token) {
    double result;
    size_t length = token.size();

    int digit_count = 0;
    int e_count = 0;
    int minus_count = 0;
    int plus_count = 0;
    int dot_count = 0;
    for (size_t i = 0; i < length; ++i) {
        if (
            ('0' <= token[i] && token[i] <= '9') ||
            token[i] == '.' ||
            token[i] == 'e' || token[i] == 'E' ||
            token[i] == '-' || token[i] == '+'
        ) {
            if ('0' <= token[i] && token[i] <= '9') {
                ++digit_count;
            } else if (token[i] == 'e' || token[i] == 'E') {
                ++e_count;
            } else if (token[i] == '-') {
                ++minus_count;
            } else if (token[i] == '+') {
                ++plus_count;
            } else if (token[i] == '.') {
                ++dot_count;
            }
        } else {
            this->quit(_pe, ("Expected double, but \"" + compress(token) + "\" found").c_str());
        }
    }
    if (digit_count == 0 || minus_count > 2 || plus_count > 2 || dot_count > 1 || e_count > 1) {
        this->quit(_pe, ("Expected double, but \"" + compress(token) + "\" found").c_str());
    }

    char *suffix = new char[length + 1];
    std::memset(suffix, 0, length + 1);
    int scanned = std::sscanf(token.c_str(), "%lf%s", &result, suffix);
    bool empty = strlen(suffix) == 0;
    delete[] suffix;

    if (scanned == 1 || (scanned == 2 && empty)) {
        if (__cont_isNaN(result)) {
            this->quit(_pe, ("Expected double, but \"" + compress(token) + "\" found").c_str());
        }
        return result;
    } else {
        this->quit(_pe, ("Expected double, but \"" + compress(token) + "\" found").c_str());
    }
}

int TInStream::_readInt() {
    if (seekEof()) {
        TInStream::quit(_unexpected_eof, "Unexpected end of file - int32 expected");
    }
    std::string token = read_token();
    long long value = token_to_ll(token);
    if (value < (long long)INT_MIN || value > (long long)INT_MAX) {
        TInStream::quit(_pe, ("Expected integer, but \"" + compress(token) + "\" found").c_str());
    }
    return (int)value;
};


TInStream::TInStream() {
    reader = nullptr;
    mode = _input;
    max_file_size = 128 * 1024 * 1024; // 128MB.
    max_token_length = 32 * 1024 * 1024; // 32MB.
}

void TInStream::init(const std::string& filename, EMode mode) {
    this->mode = mode;

    std::ifstream stream;
    stream.open(filename.c_str(), std::ios::in);
    if (stream.is_open()) {
        std::streampos start = stream.tellg();
        stream.seekg(0, std::ios::end);
        std::streampos end = stream.tellg();
        size_t fileSize = size_t(end - start);
        stream.close();

        if (fileSize > max_file_size) {
            quitf(_pe, "File size exceeds %d bytes, size is %d", int(max_file_size), int(fileSize));
        }
    }

    std::FILE* file = std::fopen(filename.c_str(), "rb");
    if (file != nullptr) {
        reader = new TFileReader(file, filename);
    }

}

void TInStream::skipBlanks() {
    while (isBlanks(reader->cur_char())) {
        reader->skip_char();
    }
}

int TInStream::readInt(int min_val, int max_val, const std::string& var_name) {
    int result = _readInt();
    if (result < min_val || result > max_val) {
        TInStream::quit(_wa, ("Integer parameter [name=" + std::string(var_name) + "] equals to " + vtos(result) +
            ", violates the range [" + toHumanReadableString(min_val) + ", " + toHumanReadableString(max_val) + "]").c_str());
    }
    return result;
}

std::vector<int> TInStream::readInts(size_t size, int min_val, int max_val, const std::string& var_name) {
    __cont_read_many(readInts, readInt(min_val, max_val, var_name), int);
}

long long TInStream::readLong(long long min_val, long long max_val, const std::string& var_name) {
    if (seekEof()) {
        TInStream::quit(_unexpected_eof, "Unexpected end of file - double expected");
    }
    long long result = token_to_ll(read_token());
    if (result < min_val || result > max_val) {
        TInStream::quit(_wa, ("Integer parameter [name=" + std::string(var_name) + "] equals to " + vtos(result) +
            ", violates the range [" + toHumanReadableString(min_val) + ", " + toHumanReadableString(max_val) + "]").c_str());
    }
    return result;
}

long long TInStream::readLong(const std::string& var_name) {
    return readLong(LLONG_MIN, LLONG_MAX, var_name);
}

std::vector<long long> TInStream::readLongs(size_t size, long long min_val, long long max_val, const std::string& var_name) {
    __cont_read_many(readLongs, readLong(min_val, max_val, var_name), long long);
}

double TInStream::readReal() {
    if (seekEof()) {
        quit(_unexpected_eof, "Unexpected end of file - double expected");
    }

    return string_to_double(readWord());
}

double TInStream::readDouble() {
    return readReal();
}

std::string TInStream::readWord() {
    skipBlanks();
    int cur = reader->cur_char();

    if (cur == EOFC) {
        quit(_unexpected_eof, "Unexpected end of file - token expected");
    }

    std::string result;
    while (!(isBlanks(cur) || cur == EOFC)) {
        result.push_back(cur);

        if (result.length() > max_token_length) {
            quitf(_pe, "Length of token exceeds %d, token is '%s...'", int(max_token_length),
                  compress(result).c_str());
        }

        cur = reader->next_char();
    }
    return result;
}

bool TInStream::seekEof() {
    skipBlanks();
    return reader->is_eof();
}

NORETURN void TInStream::quit(EResult result, const char* msg) {
    std::string message(msg);
    message = trim(message);

    if (result == _pe && mode == _answer) {
        result = _fail;
    }

    int exit_code;
    std::string str_code;
    switch (result) {
        case _ok:
            exit_code = AC;
            str_code = "ok";
            break;
        case _wa:
            exit_code = WA;
            str_code = "wa";
            break;
        case _pe:
            exit_code = PE;
            str_code = "pe";
            break;
        case _fail:
            exit_code = FL;
            str_code = "fail";
            break;
        case _unexpected_eof:
            exit_code = PE;
            str_code = "unexpected eof";
            break;
    }
    std::fprintf(stderr, "%s: %s\n", str_code.c_str(), msg);

    inf.close();
    ouf.close();
    ans.close();

    std::exit(exit_code);
}

NORETURN void TInStream::quitf(EResult result, const char* format, ...) {
    FMT_TO_RESULT(format, format, message);
    TInStream::quit(result, __quit_message);
}

void TInStream::close() {
    if (NULL != reader) {
        // reader->close();
        delete reader;
        reader = NULL;
    }
}

void registerTestlibCmd(int argc, char* argv[]) {
    ouf.init("output.txt", _input);
    inf.init("input.txt", _output);
    ans.init("pattern.txt", _answer);
}

void setName(const char* format, ...) {}

NORETURN void quit(EResult result, const char* msg) {
    ouf.quit(result, msg);
}

NORETURN void quitf(EResult result, const char* format, ...) {
    FMT_TO_RESULT(format, format, message);
    quit(result, __quit_message);
}

inline bool doubleCompare(double expected, double result, double MAX_ERROR) {
    MAX_ERROR += 1e-15;
    if (__cont_isNaN(expected)) {
        return __cont_isNaN(result);
    } else if (__cont_isInfinite(expected)) {
        return __cont_isInfinite(result) && ((expected > 0) == (result > 0));
    } else if (__cont_isNaN(result) || __cont_isInfinite(result)) {
        return false;
    } else if (__cont_abs(expected - result) < MAX_ERROR) {
        return true;
    } else {
        double min_v = __cont_min(expected * (1.0 - MAX_ERROR), expected * (1.0 + MAX_ERROR));
        double max_v = __cont_max(expected * (1.0 - MAX_ERROR), expected * (1.0 + MAX_ERROR));
        return min_v <= result && result <= max_v;
    }
}

inline double doubleDelta(double expected, double result) {
    double absolute = __cont_abs(result - expected);

    if (__cont_abs(expected) > 1E-9) {
        double relative = __cont_abs(absolute / expected);
        return __cont_min(absolute, relative);
    } else
        return absolute;
}

#endif  // CONTLIB_H
