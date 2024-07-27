#include <complex>
#include <vector>
#include <iostream>

// Declare the LAPACK routine
extern "C" {
    void zgeev_(char* jobvl, char* jobvr, int* n, std::complex<double>* a,
                int* lda, std::complex<double>* w, std::complex<double>* vl,
                int* ldvl, std::complex<double>* vr, int* ldvr,
                std::complex<double>* work, int* lwork, double* rwork,
                int* info);
}

std::vector<std::complex<double>> computeEigenvalues(std::vector<std::complex<double>>& matrix, int n) {
    char jobvl = 'N', jobvr = 'N';
    int lda = n, ldvl = 1, ldvr = 1, info, lwork = 4*n;
    std::vector<std::complex<double>> w(n), work(lwork);
    std::vector<double> rwork(2*n);

    zgeev_(&jobvl, &jobvr, &n, matrix.data(), &lda, w.data(), nullptr,
           &ldvl, nullptr, &ldvr, work.data(), &lwork, rwork.data(), &info);

    if (info != 0) {
        std::cerr << "ZGEEV failed with error code " << info << std::endl;
    }

    return w;
}

int main() {
    int n = 3;
    std::vector<std::complex<double>> matrix = {
        {1, 1}, {2, 0}, {3, 0},
        {4, 0}, {5, -2}, {6, 0},
        {7, 0}, {8, 0}, {9, 3}
    };

    auto eigenvalues = computeEigenvalues(matrix, n);

    std::cout << "Eigenvalues:" << std::endl;
    for (const auto& ev : eigenvalues) {
        std::cout << ev.real() << " + " << ev.imag() << "i" << std::endl;
    }

    return 0;
}