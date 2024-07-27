#include "matrix_generator.h"

#include <random>

MatrixGenerator::MatrixGenerator(int rows, int cols, std::function<std::complex<double>(int, int)> generator)
    : rows(rows), cols(cols), generator(generator) {}


std::vector<std::complex<double>> MatrixGenerator::generate() {
    std::vector<std::complex<double>> matrix(rows * cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i * cols + j] = generator(i, j);
        }
    }
    return matrix;
}

std::vector<int> MatrixGenerator::shape() {
    return {rows, cols};
}

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

    return MatrixGenerator(10, 10, tridiagonal_generator);
}