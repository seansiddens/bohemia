#pragma once

#include <vector>
#include <complex>
#include <functional>

class MatrixGenerator {
private:
    int rows, cols;
    std::function<std::complex<double>(int, int)> generator;

public:
    MatrixGenerator(int rows, int cols, std::function<std::complex<double>(int, int)> generator);

    std::vector<std::complex<double>> generate();

    std::vector<int> shape();

    // Declare the static function here
    static MatrixGenerator tridiagonal_10x10();
};