#include "testlib.h"

int main(int argc, char *argv[]) {
    registerValidation(argc, argv);

    int t = inf.readInt(1, 10, "t");
    inf.readEoln();
    inf.readInts(3, 1, 3,"ai");
    inf.readEoln();

    inf.readEof();

}
