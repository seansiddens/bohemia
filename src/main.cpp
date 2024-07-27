#include <iostream>
#include <vector>
#include <complex>
#include <functional>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>
#include <mutex>

#include "matrix.h"
#include "matrix_generator.h"
#include "complex_plane.h"
#include "logger.h"

#include "json.h"
using json = nlohmann::json;

// Function to write eigenvalues to a binary file
void write_eigenvalues_to_file(const std::vector<std::complex<double>>& eigenvalues, const std::string& filename) {
    // TODO: Instead of just saving a vector of eigenvalues, construct a histogram of eigenvalues and save the histogram to a file.
    //       Allows us to use some sort of RLE compression. Additionally, the process of loading and plotting the eigenvalues could 
    //       potentially be much faster, since incrementing a bin for each eigenvalue would be a single operation.
    const size_t MAX_FILE_SIZE = 10ULL * 1024 * 1024 * 1024; // 10 GB limit
    size_t size = eigenvalues.size();
    size_t total_size = sizeof(size_t) + size * sizeof(std::complex<double>);

    if (total_size > MAX_FILE_SIZE) {
        size_t max_eigenvalues = (MAX_FILE_SIZE - sizeof(size_t)) / sizeof(std::complex<double>);
        size = max_eigenvalues;
        total_size = sizeof(size_t) + size * sizeof(std::complex<double>);
        LOG_ERROR << "File size exceeds the limit. Writing only " << size << " eigenvalues.";
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR << "Failed to open file for writing: " << filename;
        return;
    }

    file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(eigenvalues.data()), size * sizeof(std::complex<double>));

    if (file.good()) {
        LOG_INFO << "Successfully wrote " << size << " eigenvalues to " << filename;
    } else {
        LOG_ERROR << "Error occurred while writing eigenvalues to " << filename;
    }
    file.close();
}

// Function to load eigenvalues from a binary file
std::vector<std::complex<double>> load_eigenvalues_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR << "Failed to open file for reading: " << filename;
        return {};
    }

    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size_t));

    std::vector<std::complex<double>> eigenvalues(size);
    file.read(reinterpret_cast<char*>(eigenvalues.data()), size * sizeof(std::complex<double>));

    if (file.good()) {
        LOG_INFO << "Successfully loaded " << size << " eigenvalues from " << filename;
    } else {
        LOG_ERROR << "Error occurred while reading eigenvalues from " << filename;
        eigenvalues.clear();
    }
    file.close();

    return eigenvalues;
}

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
    auto& eigenvalue_config = config["eigenvalues"];
    std::string eigenvalue_mode = eigenvalue_config["mode"];
    std::string eigenvalue_file = eigenvalue_config["file"];
    
    // Extract visualization parameters
    auto& visualization_params = config["visualization_params"];
    double real_min = visualization_params["real_min"];
    double real_max = visualization_params["real_max"];
    double imaginary_min = visualization_params["imaginary_min"];
    double imaginary_max = visualization_params["imaginary_max"];
    std::string output_file = visualization_params["output_file"];
    std::string color_map = visualization_params["color_map"];
    double gamma = visualization_params["gamma"];
    assert(gamma > 0);

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
    std::vector<std::vector<std::complex<double>>> all_eigenvalues(num_threads);
    std::mutex eigenvalues_mutex;

    std::vector<std::complex<double>> eigenvalues;

    if (eigenvalue_mode == "load") {
        LOG_INFO << "Loading eigenvalues from file: " << eigenvalue_file;
        eigenvalues = load_eigenvalues_from_file(eigenvalue_file);
        if (eigenvalues.empty()) {
            LOG_ERROR << "Failed to load eigenvalues. Exiting.";
            return 1;
        }
        // Plot the loaded eigenvalues
        for (const auto& eigenvalue : eigenvalues) {
            plane.add_point(eigenvalue);
        }
    } else {
        LOG_INFO << "Plotting eigenvalues using " << num_threads << " threads";
        std::cout << "Processing: " << std::flush;

        auto worker = [&](int thread_id, int thread_samples) {
            std::vector<std::complex<double>> local_eigenvalues;
            for (int i = 0; i < thread_samples; i++) {
                auto matrix = mat_gen.generate();
                auto eigenvalues = matrix.compute_eigenvalues();
                for (const auto& eigenvalue : eigenvalues) {
                    plane.add_point(eigenvalue);
                    if (eigenvalue_mode == "dump") {
                        local_eigenvalues.push_back(eigenvalue);
                    }
                }
                
                // Update progress
                int current_progress = ++progress;
                if (current_progress % (samples / 100) == 0) {
                    std::cout << "\rProcessing: [" << std::string(current_progress / (samples / 100), '#') << std::string(100 - current_progress / (samples / 100), ' ') << "] " << current_progress / (samples / 100) << "%" << std::flush;
                }
            }
            if (eigenvalue_mode == "dump") {
                std::lock_guard<std::mutex> lock(eigenvalues_mutex);
                all_eigenvalues[thread_id] = std::move(local_eigenvalues);
            }
        };

        // Spawn threads
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back(worker, i, samples_per_thread);
        }

        // Wait for all threads to finish
        for (auto& thread : threads) {
            thread.join();
        }

        std::cout << "\rProcessing: [" << std::string(100, '#') << "] 100%" << std::endl;
        LOG_INFO << "Finished plotting eigenvalues";

        if (eigenvalue_mode == "dump") {
            LOG_INFO << "Dumping eigenvalues to file: " << eigenvalue_file;
            std::vector<std::complex<double>> combined_eigenvalues;
            for (const auto& thread_eigenvalues : all_eigenvalues) {
                combined_eigenvalues.insert(combined_eigenvalues.end(), thread_eigenvalues.begin(), thread_eigenvalues.end());
            }
            write_eigenvalues_to_file(combined_eigenvalues, eigenvalue_file);
        }
    }

    LOG_INFO << "Saving image...";

    plane.save_image(output_file, gamma, color_map);

    LOG_INFO << "Finished saving image";

    return 0;
}