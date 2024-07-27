#pragma once

#include <vector>
#include <complex>
#include <iostream>

class Matrix {
public:
    Matrix(int n, std::vector<std::complex<double>> values);
    std::complex<double> get(int row, int col) const;
    void set(int row, int col, std::complex<double> value);
    std::complex<double>* data();
    std::vector<std::complex<double>> compute_eigenvalues();
    int size;

    friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

private:
    std::vector<std::complex<double>> values;
};

std::ostream& operator<<(std::ostream& os, const Matrix& matrix);