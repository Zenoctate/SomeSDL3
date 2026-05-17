#include "lib/math.h"

double math_absf(double x) {
    if(x >= 0) {
        return x;
    } else {
        return -x;
    }
}