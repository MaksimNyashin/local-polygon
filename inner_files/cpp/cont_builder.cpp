#include "utils.h"

int main() {
    TConfig config = NConfig::TConfig();
    TPaths paths = NPaths::TPaths();

    // Copy tests
    NBuild::createFolder(paths.cont_tests);
    for (const auto& test_filename : fs::directory_iterator(paths.tests_tests)) {
        if (fs::is_directory(test_filename)) {
            continue;
        }
        if (test_filename.path().extension() == ".ans") {
            fs::copy_file(
                test_filename.path(),
                paths.cont_tests / ("test_" + test_filename.path().stem().string() + "_pattern.txt"),
                fs::copy_options::overwrite_existing
            );
        } else {
            fs::copy_file(
                test_filename.path(),
                paths.cont_tests / ("test_" + test_filename.path().stem().string() + "_input.txt"),
                fs::copy_options::overwrite_existing
            );
        }
    }

    // Copy main solution
    fs::copy_file(
        paths.home_dir / (config.solution.main + "." + config.solution.lang),
        paths.cont_dir / ("main." + config.solution.lang),
        fs::copy_options::overwrite_existing
    );

    // Copy legend
    fs::copy_file(
        paths.home_dir / "legend.html",
        paths.cont_dir / "legend.html",
        fs::copy_options::overwrite_existing
    );

    // Copy checker
    std::ifstream t_ch(paths.checkers / (config.task.checker + ".cpp"));
    std::stringstream checker;
    checker << t_ch.rdbuf();
    t_ch.close();
    std::string tmp;
    std::getline(checker, tmp);
    std::string remaining;
    std::getline(checker, remaining, '\0');

    std::ifstream t_cont(paths.checkers_cont);
    std::stringstream contlib;
    contlib << t_cont.rdbuf();
    t_cont.close();

    std::ofstream cont_ch(paths.cont_dir / "checker.cpp", std::ios::out);
    cont_ch << contlib.str() << "\n\n" << remaining;
    cont_ch.close();

    // Create xml
    std::string xml = std::format("<?xml version=\"1.0\" encoding=\"windows-1251\"?>\n\
<Problem generator=\"Contester 2.4\">\n\
<Name lang=\"ru\">{}</Name>\n\
<Contest><Name lang=\"ru\">{}</Name></Contest>\n\
<PreContest lang=\"ru\">{}</PreContest>\n\
<TimeLimit platform=\"native\">{}</TimeLimit>\n\
<TimeLimit platform=\"javavm\">{}</TimeLimit>\n\
<TimeLimit platform=\"dotnet\">{}</TimeLimit>\n\
<TimeLimit platform=\"custom\">{}</TimeLimit>\n\
<MemoryLimit platform=\"native\">{}</MemoryLimit>\n\
<MemoryLimit platform=\"javavm\">{}</MemoryLimit>\n\
<MemoryLimit platform=\"dotnet\">{}</MemoryLimit>\n\
<MemoryLimit platform=\"custom\">{}</MemoryLimit>\n\
<AutoLimits>0</AutoLimits>\n\
<Statement lang=\"ru\" src=\"legend.html\" />\n\
<TestList inputmask=\"tests/test_*_input.txt\" patternmask=\"tests/test_*_pattern.txt\" />\n\
<Judge><Checker src=\"checker.cpp\" /></Judge>\n\
<Attempt><Solver src=\"main.cpp\" /></Attempt>\n\
</Problem>\n\
",
        config.task.task_name,
        config.contest,
        config.contest_let,
        config.task.tl * 1000,
        config.task.tl * 1000,
        config.task.tl * 1000,
        config.task.tl * 1000,
        config.task.ml * 1000,
        config.task.ml * 1000,
        config.task.ml * 1000,
        config.task.ml * 1000
    );
    std::ofstream xml_res(paths.cont_dir / "task.xml", std::ios::out);
    xml_res << xml;
    xml_res.close();

    std::system(std::format(
        "{} {} \"task.xml\" \"legend.html\"",
        config.build.py_version,
        (paths.my_dir / "cont_converter.py").string()
    ).c_str());

    return 0;
}