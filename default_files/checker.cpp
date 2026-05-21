#include "testlib.h"

// ouf - user output
// ans - jury answer

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);


    // if (jr < pr) {
    //     quitf(_wa, "Jury's answer is better than participant's");
    // } else if (pr < jr) {
    //     quitf(_fail, "Participant's answer is better than jury's");
    // }

    quitf(_ok, "Ok");

}
