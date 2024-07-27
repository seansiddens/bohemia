#pragma once

#include <vector>
#include <complex>
#include <functional>

#include "matrix.h"

class MatrixGenerator {
private:
    int size;
    std::function<std::complex<double>(int, int)> generator;

public:
    MatrixGenerator(int size, std::function<std::complex<double>(int, int)> generator);

    Matrix generate();

    // Pre-defined generators
    static MatrixGenerator tridiagonal_10x10();
};