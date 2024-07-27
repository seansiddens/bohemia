#include "matrix_generator.h"

#include "logger.h"

#include <random>

MatrixGenerator::MatrixGenerator(int size, std::function<std::complex<double>(int, int)> generator)
    : size(size), generator(generator) {}

/// Generate a matrix of size n x n with random values>
Matrix MatrixGenerator::generate() {
    std::vector<std::complex<double>> values(size * size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            values[i * size + j] = generator(i, j);
        }
    }

    return Matrix(size, std::move(values));
}

/// Generate a tridiagonal matrix of size 10x10 with random values
/// Values randomly chosen from {0, 1, -1, i, -i, 20, -20, i*20, -i*20}
MatrixGenerator MatrixGenerator::tridiagonal_10x10() {
    const std::vector<std::complex<double>> values = {
        0.0, 1.0, -1.0, std::complex<double>(0, 1), std::complex<double>(0, -1),
        20.0, -20.0, std::complex<double>(0, 20), std::complex<double>(0, -20)
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, values.size() - 1);

    auto tridiagonal_generator = [values, gen, dis](int i, int j) mutable {
        if (i == j || i == j + 1 || i == j - 1) {
            return values[dis(gen)];
        }
        return std::complex<double>(0, 0);
    };

    return MatrixGenerator(10, tridiagonal_generator);
}