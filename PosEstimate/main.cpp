#include <iostream>
#include "itrbase.h"
#include "itrvision.h"
#include "itrsystem.h"
#include "observe.h"
using namespace std;

int main() {
    itr_math::MathObjStandInit();
    Vector tmp(3);
    Observe obs;
    obs.Init();
    obs.PosEstimate(1, 1, 1, tmp, tmp);
    cout << "Hello, World!" << endl;
    return 0;
}