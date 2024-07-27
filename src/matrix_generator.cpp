#include "matrix_generator.h"

#include "logger.h"

MatrixGenerator::MatrixGenerator(int size, std::function<std::complex<double>(int, int)> generator)
    : size(size), generator(generator) {}

/// Generate a matrix of size n x n with random values>
Matrix MatrixGenerator::generate() const {
    std::vector<std::complex<double>> values(size * size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            values[i * size + j] = generator(i, j);
        }
    }

    return Matrix(size, std::move(values));
}
