#include "testlib.h"

int main(int argc, char *argv[]) {
    registerValidation(argc, argv);

    int t = inf.readInt(1, 10, "t");
    inf.readEoln();
    inf.readInts(3, 1, 3,"ai");
    inf.readEoln();
    inf.readDouble(1, 10, "db");
    inf.readEoln();
    inf.readToken("[a-z]{4}", "s");
    inf.readEoln();

    inf.readEof();

}
