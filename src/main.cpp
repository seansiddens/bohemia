#include <iostream>
#include <vector>
#include <complex>
#include <functional>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>

#include "matrix.h"
#include "matrix_generator.h"
#include "complex_plane.h"
#include "logger.h"

#include "json.h"
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_config_file>" << std::endl;
        return 1;
    }

    // Load configuration from JSON file
    std::ifstream config_file(argv[1]);
    if (!config_file.is_open()) {
        std::cerr << "Error: Unable to open config file." << std::endl;
        return 1;
    }

    json config;
    try {
        config_file >> config;
    } catch (const json::parse_error& e) {
        std::cerr << "Error: Failed to parse config file. " << e.what() << std::endl;
        return 1;
    }

    // Extract parameters from config
    int resolution = config["resolution"];
    int samples = config["samples"];
    
    // Extract visualization parameters
    auto& visualization_params = config["visualization_params"];
    double real_min = visualization_params["real_min"];
    double real_max = visualization_params["real_max"];
    double imaginary_min = visualization_params["imaginary_min"];
    double imaginary_max = visualization_params["imaginary_max"];
    std::string output_file = visualization_params["output_file"];
    double gamma = visualization_params["gamma"];

    // Define matrix generator.
    auto mat_gen = MatrixGenerator::tridiagonal_10x10();

    // Define plane using the loaded parameters
    auto plane = ComplexPlane(resolution, real_min, real_max, imaginary_min, imaginary_max);

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

    plane.save_image(output_file, gamma);

    LOG_INFO << "Finished saving image";

    return 0;
}