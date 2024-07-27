#include <iostream>
#include <vector>
#include <complex>
#include <functional>
#include <random>

#include "matrix_generator.h"

// Declare the LAPACK routine
extern "C" {
    void zgeev_(char* jobvl, char* jobvr, int* n, std::complex<double>* a,
                int* lda, std::complex<double>* w, std::complex<double>* vl,
                int* ldvl, std::complex<double>* vr, int* ldvr,
                std::complex<double>* work, int* lwork, double* rwork,
                int* info);
}

std::vector<std::complex<double>> compute_eigenvalues(std::vector<std::complex<double>>& matrix, int n) {
    char jobvl = 'N', jobvr = 'N';
    int lda = n, ldvl = 1, ldvr = 1, info, lwork = 4*n;
    std::vector<std::complex<double>> w(n), work(lwork);
    std::vector<double> rwork(2*n);

    zgeev_(&jobvl, &jobvr, &n, matrix.data(), &lda, w.data(), nullptr,
           &ldvl, nullptr, &ldvr, work.data(), &lwork, rwork.data(), &info);

    if (info != 0) {
        std::cerr << "ZGEEV failed with error code " << info << std::endl;
        // Return 0 matrix
        return std::vector<std::complex<double>>(n, 0);
    }

    return w;
}

int main() {
    // Example usage
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);

    auto random_complex = [&gen, &dis](int i, int j) {
        return std::complex<double>(dis(gen), dis(gen));
    };

    // MatrixGenerator mat_gen(3, 3, random_complex);
    // auto matrix = mat_gen.generate();
    auto mat_gen = MatrixGenerator::tridiagonal_10x10();
    std::cout << mat_gen.shape()[0] << " " << mat_gen.shape()[1] << std::endl;
    auto matrix = mat_gen.generate();

    // Print the matrix
    for (int i = 0; i < mat_gen.shape()[0]; ++i) {
        for (int j = 0; j < mat_gen.shape()[1]; ++j) {
            std::cout << matrix[i * mat_gen.shape()[1] + j] << " ";
        }
        std::cout << std::endl;
    }

    // Compute eigenvalues
    auto eigenvalues = compute_eigenvalues(matrix, mat_gen.shape()[0]);

    // Print eigenvalues
    std::cout << "Eigenvalues:" << std::endl;
    for (const auto& eigenvalue : eigenvalues) {
        std::cout << eigenvalue << std::endl;
    }
    return 0;
}
