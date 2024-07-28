#pragma once

#include <atomic>
#include <vector>
#include <complex>
#include <cinttypes>

#include <tbb/concurrent_unordered_map.h>

#include "matrix_generator.h"

namespace std {
    template<>
    struct hash<std::complex<int>> {
        size_t operator()(const std::complex<int>& c) const {
            return hash<int>()(c.real()) ^ hash<int>()(c.imag());
        }
    };
}

class EigenvaluePMF {
public:
    static EigenvaluePMF compute_pmf(const MatrixGenerator& generator, int num_samples, int precision=3);
    static EigenvaluePMF from_file(const std::string& filename);
    void insert(const std::complex<double>& eigenvalue);
    static void compute_eigenvalues(int thread_id, int num_samples, MatrixGenerator thread_local_generator, std::atomic<int>& progress, EigenvaluePMF& pmf);
    void get_max_count();
    uint64_t get_count(const std::complex<int>& discretized_eigenvalue) const;
    size_t bin_count() const { return bins.size(); }

    // Delete default constructor, copy constructor, and assignment operator.
    EigenvaluePMF() = delete;
    EigenvaluePMF(const EigenvaluePMF&) = delete;
    EigenvaluePMF& operator=(const EigenvaluePMF&) = delete;

    // Allow move operations
    EigenvaluePMF(EigenvaluePMF&&) = default;
    EigenvaluePMF& operator=(EigenvaluePMF&&) = default;

    std::complex<int> discretize(const std::complex<double>& value) const;
    std::complex<double> undiscretize(const std::complex<int>& discretized_value) const;
    uint64_t max_count = 0;
    uint64_t total_eigenvalues = 0;

    tbb::concurrent_unordered_map<std::complex<int>, std::atomic<uint64_t>> bins;

private:
    MatrixGenerator generator;
    double discretization_factor = 0.001;

    EigenvaluePMF(const MatrixGenerator& generator, double discretization_factor);
};