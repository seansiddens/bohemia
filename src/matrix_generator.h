#pragma once

#include <vector>
#include <complex>
#include <functional>
#include <random>

#include <chrono>

#include "XoshiroCpp.h"

#include "matrix.h"
#include "util.h"

class MatrixGenerator {
private:
    int size;
    std::function<std::complex<double>(int, int)> generator;

public:
    MatrixGenerator(int size, std::function<std::complex<double>(int, int)> generator);

    Matrix generate() const;

    // Pre-defined generators
    template<int N>
    static MatrixGenerator tridiagonal() {
        const std::vector<std::complex<double>> values = {
            0.0, 1.0, -1.0, std::complex<double>(0, 1), std::complex<double>(0, -1),
            20.0, -20.0, std::complex<double>(0, 20), std::complex<double>(0, -20)
        };

        auto tridiagonal_generator = [values](int i, int j) mutable {
            static thread_local XoshiroCpp::Xoshiro256PlusPlus rng(GlobalSeedGenerator::get_next_seed());
            if (i == j || i == j + 1 || i == j - 1) {
                size_t index = rng() % values.size();
                return values[index];
            }
            return std::complex<double>(0, 0);
        };

        return MatrixGenerator(N, tridiagonal_generator);
    }
};