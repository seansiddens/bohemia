#include "matrix.h"
#include <cassert>
#include <iostream>
#include <iomanip>

#include "logger.h"

// Declare the LAPACK routine
extern "C" {
    void zgeev_(char* jobvl, char* jobvr, int* n, std::complex<double>* a,
                int* lda, std::complex<double>* w, std::complex<double>* vl,
                int* ldvl, std::complex<double>* vr, int* ldvr,
                std::complex<double>* work, int* lwork, double* rwork,
                int* info);
}

Matrix::Matrix(int n, std::vector<std::complex<double>> _values) {
    size = n;
    values = std::move(_values);
    assert(static_cast<int>(values.size()) == n * n && "Number of values must match the matrix size");
}

std::complex<double> Matrix::get(int row, int col) const {
    assert(row >= 0 && row < size && col >= 0 && col < size && "Matrix indices out of range");
    return values[row * size + col];
}

void Matrix::set(int row, int col, std::complex<double> value) {
    assert(row >= 0 && row < size && col >= 0 && col < size && "Matrix indices out of range");
    values[row * size + col] = value;
}

std::complex<double>* Matrix::data() {
    return values.data();
}

std::vector<std::complex<double>> Matrix::compute_eigenvalues() {
    int n = size;  // Assuming square matrix
    char jobvl = 'N', jobvr = 'N';
    int lda = n, ldvl = 1, ldvr = 1, info, lwork = 4*n;
    std::vector<std::complex<double>> w(n), work(lwork);
    std::vector<double> rwork(2*n);

    zgeev_(&jobvl, &jobvr, &n, this->data(), &lda, w.data(), nullptr,
           &ldvl, nullptr, &ldvr, work.data(), &lwork, rwork.data(), &info);

    if (info != 0) {
        LOG_ERROR << "ZGEEV failed with error code " << info;
        return std::vector<std::complex<double>>(n, 0);
    }

    return w;
}

std::ostream& operator<<(std::ostream& os, const Matrix& matrix) {
    const int width = 5; 
    const int numWidth = 6; // Width for each number (real and imaginary parts)
    os << std::fixed << std::setprecision(2);
    for (int i = 0; i < matrix.size; ++i) {
        for (int j = 0; j < matrix.size; ++j) {
            std::complex<double> value = matrix.get(i, j);
            os << std::setw(width) << std::right 
               << "(" << std::setw(numWidth) << value.real() << std::showpos 
               << std::setw(numWidth) << value.imag() << "i)";
        }
        os << "\n";
    }
    return os;
}