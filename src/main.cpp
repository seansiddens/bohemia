#include <iostream>
#include <vector>
#include <complex>
#include <functional>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>

#include "matrix.h"
#include "matrix_generator.h"
#include "complex_plane.h"
#include "logger.h"

int main() {
    // Define matrix generator.
    auto mat_gen = MatrixGenerator::tridiagonal_10x10();

    // Define plane.
    auto resolution = 1024 * 2;
    auto plane = ComplexPlane(resolution, resolution, -48.0, 48.0, -48.0, 48.0);

    int samples = 1024 * 1024;
    int num_threads = std::thread::hardware_concurrency();
    int samples_per_thread = samples / num_threads;
    LOG_INFO << "Total samples: " << samples;
    LOG_INFO << "Samples per thread: " << samples_per_thread;
    LOG_INFO << "Number of threads: " << num_threads;

    std::vector<std::thread> threads;
    std::atomic<int> progress(0);

    LOG_INFO << "Plotting eigenvalues using " << num_threads << " threads";
    std::cout << "Processing: " << std::flush;

    auto worker = [&](int thread_samples) {
        for (int i = 0; i < thread_samples; i++) {
            auto matrix = mat_gen.generate();
            auto eigenvalues = matrix.compute_eigenvalues();
            for (const auto& eigenvalue : eigenvalues) {
                plane.add_point(eigenvalue);
            }
            
            // Update progress
            int current_progress = ++progress;
            if (current_progress % (samples / 100) == 0) {
                std::cout << "\rProcessing: [" << std::string(current_progress / (samples / 100), '#') << std::string(100 - current_progress / (samples / 100), ' ') << "] " << current_progress / (samples / 100) << "%" << std::flush;
            }
        }
    };

    // Spawn threads
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(worker, samples_per_thread);
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "\rProcessing: [" << std::string(100, '#') << "] 100%" << std::endl;
    LOG_INFO << "Finished plotting eigenvalues";

    LOG_INFO << "Saving image...";

    plane.save_image("eigenvalues.png");

    LOG_INFO << "Finished saving image";

    return 0;
}