#ifndef UTILS_CUH
#define UTILS_CUH

#include <cmath>

__device__ inline
double deviceNormalCDF(double x, double mu, double sigma) {
    return erfc(-(x - mu) / (sigma * sqrt(2.0))) / 2;
}

#endif
