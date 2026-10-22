#include "testlib.h"
#include <iomanip>

using namespace std;

typedef long long ll;


int main(int argc, char *argv[]) {
    registerGen(argc, argv, 1);

    int t = opt<int>("t");

    cout << t << "\n1 2 3\n";
    cout << std::fixed << std::setprecision(5) << rnd.next((double)1.0, (double)10.0) << "\nmaks\n";

    return 0;
}
