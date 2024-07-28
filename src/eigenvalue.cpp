#include "eigenvalue.h"

#include <thread>
#include <unordered_map>

#include "logger.h"

std::complex<int> EigenvaluePMF::discretize(const std::complex<double>& eigenvalue) const {
    return {
        static_cast<int>(std::round(eigenvalue.real() / discretization_factor)),
        static_cast<int>(std::round(eigenvalue.imag() / discretization_factor))
    };
}

void EigenvaluePMF::insert(const std::complex<double>& eigenvalue) {
    auto discretized_eigenvalue = discretize(eigenvalue);
    bins[discretized_eigenvalue].fetch_add(1, std::memory_order_relaxed);
}

void EigenvaluePMF::compute_eigenvalues(int thread_id, int num_samples, MatrixGenerator thread_local_generator, std::atomic<int>& progress, EigenvaluePMF& pmf) {
    for (int i = 0; i < num_samples; i++) {
        auto matrix = thread_local_generator.generate();
        auto eigenvalues = matrix.compute_eigenvalues();
        for (const auto& eigenvalue : eigenvalues) {
            // TODO: Ignore real eigenvalues.
            if (eigenvalue.imag() == 0) {
                continue;
            }
            pmf.insert(eigenvalue);
        }
    }
}

void EigenvaluePMF::get_max_count() {
    for (const auto& bin : bins) {
        auto count = bin.second.load(std::memory_order_relaxed);
        if (count > max_count) {
            max_count = count;
        }
        total_eigenvalues += count;
    }

    LOG_DEBUG << "Max count: " << max_count;
    LOG_DEBUG << "Total eigenvalues: " << total_eigenvalues;
}

EigenvaluePMF EigenvaluePMF::compute_pmf(const MatrixGenerator& generator, int num_samples, int precision) {
    double discretization_factor = 1.0 / std::pow(10, precision);
    auto pmf = EigenvaluePMF(generator, discretization_factor);

    int num_threads = std::thread::hardware_concurrency();
    int samples_per_thread = num_samples / num_threads;
    LOG_DEBUG << "Num threads: " << num_threads;
    LOG_DEBUG << "Samples per thread: " << samples_per_thread;

    std::vector<std::thread> worker_threads;
    std::atomic<int> progress(0);

    // Spawn threads
    for (int i = 0; i < num_threads; i++) {
        worker_threads.push_back(std::thread(&EigenvaluePMF::compute_eigenvalues, i, samples_per_thread, generator, std::ref(progress), std::ref(pmf)));
    }
    
    LOG_DEBUG << "Launched eigenvalue computation threads.";

    // Join threads
    for (auto& thread : worker_threads) {
        thread.join();
    }

    LOG_DEBUG << "Joined eigenvalue computation threads.";

    pmf.get_max_count();

    return pmf;
}

EigenvaluePMF::EigenvaluePMF(const MatrixGenerator& generator, double discretization_factor) 
    : generator(generator), discretization_factor(discretization_factor) {
}

uint64_t EigenvaluePMF::get_count(const std::complex<int>& discretized_eigenvalue) const {
    auto it = bins.find(discretized_eigenvalue);
    if (it != bins.end()) {
        return it->second.load(std::memory_order_relaxed);
    }
    return 0;
}

std::complex<double> EigenvaluePMF::undiscretize(const std::complex<int>& discretized_value) const {
    return {
        discretized_value.real() * discretization_factor,
        discretized_value.imag() * discretization_factor
    };
}