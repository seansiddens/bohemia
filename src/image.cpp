#include "image.h"
#include <cstring>
#include <thread>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <filesystem> // Added for std::filesystem::create_directories

#include "tinycolormap.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "logger.h"

ImageHistogram::ImageHistogram(int resolution, double rmin, double rmax, double imin, double imax)
    : real_min(rmin), real_max(rmax), imag_min(imin), imag_max(imax) {
    double real_range = real_max - real_min;
    double imag_range = imag_max - imag_min;
    double aspect_ratio = real_range / imag_range;
    if (aspect_ratio >= 1.0) {
        height = resolution;
        width = static_cast<int>(resolution * aspect_ratio);
    } else {
        width = resolution;
        height = static_cast<int>(resolution / aspect_ratio);
    }

    histogram = new std::atomic<uint64_t>[width * height];
    for (int i = 0; i < width * height; ++i) {
        histogram[i].store(0, std::memory_order_relaxed);
    }
}

void ImageHistogram::add_point(const std::complex<double>& point) {
    int x = static_cast<int>((point.real() - real_min) / (real_max - real_min) * width);
    int y = static_cast<int>((point.imag() - imag_min) / (imag_max - imag_min) * height);
    
    if (x >= 0 && x < width && y >= 0 && y < height) {
        histogram[y * width + x].fetch_add(1, std::memory_order_relaxed);
    }
}

uint64_t ImageHistogram::parallel_max() const {
    const int num_threads = std::thread::hardware_concurrency();
    const int chunk_size = (width * height + num_threads - 1) / num_threads;

    std::vector<uint64_t> local_maxima(num_threads, 0);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            int start = i * chunk_size;
            int end = std::min(start + chunk_size, width * height);
            uint64_t local_max = 0;
            for (int j = start; j < end; ++j) {
                local_max = std::max(local_max, histogram[j].load(std::memory_order_relaxed));
            }
            local_maxima[i] = local_max;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return *std::max_element(local_maxima.begin(), local_maxima.end());
}

// void ImageHistogram::save_image(const std::string& filename, double gamma, std::string color_map, const EigenvaluePMF& pmf) {
//     // Iterate over all bins in the PMF, and add the count to the histogram.
//     for (const auto& [discretized_ev, count_atomic] : pmf.bins) {
//         uint64_t count = count_atomic.load(std::memory_order_relaxed);
//         if (count == 0) continue;

//         // Un-discretize the eigenvalue
//         std::complex<double> ev = pmf.undiscretize(discretized_ev);

//         // Map the eigenvalue to image coordinates
//         int x = static_cast<int>((ev.real() - real_min) / (real_max - real_min) * width);
//         int y = static_cast<int>((ev.imag() - imag_min) / (imag_max - imag_min) * height);

//         // Ensure the coordinates are within bounds
//         if (x >= 0 && x < width && y >= 0 && y < height) {
//             histogram[y * width + x].fetch_add(count, std::memory_order_relaxed);
//         }
//     }

//     // Find the maximum count in the histogram
//     uint64_t max_count = parallel_max();

//     // Calculate log(max_count + 1) once
//     double log_max = std::log(max_count + 1);

//     tinycolormap::ColormapType color_map_type;
//     bool binary = false;
//     if (color_map == "viridis") {
//         color_map_type = tinycolormap::ColormapType::Viridis;
//     } else if (color_map == "plasma") {
//         color_map_type = tinycolormap::ColormapType::Plasma;
//     } else if (color_map == "binary") {
//         binary = true;
//     } else {
//         LOG_ERROR << "Unknown color map type! Defaulting to grayscale.";
//         color_map_type = tinycolormap::ColormapType::Gray;
//     }

//     // Create the image
//     std::vector<unsigned char> image(width * height * 3, 0);

//     for (int y = 0; y < height; ++y) {
//         for (int x = 0; x < width; ++x) {
//             int index = (y * width + x) * 3;
//             uint64_t count = histogram[y * width + x].load(std::memory_order_relaxed);

//             if (binary) {
//                 // Do a simple binary thresholding.
//                 count = count > 0 ? 255 : 0;
//                 image[index] = static_cast<unsigned char>(count);
//                 image[index + 1] = static_cast<unsigned char>(count);
//                 image[index + 2] = static_cast<unsigned char>(count);
//                 continue;
//             }
//             if (count == 0) {
//                 // Set color to black for zero count
//                 image[index] = 0;     // R
//                 image[index + 1] = 0; // G
//                 image[index + 2] = 0; // B
//             } else {
//                 // Apply logarithmic scaling
//                 double log_scaled = std::log(count + 1) / log_max;
                
//                 // Apply gamma correction
//                 double gamma_corrected = std::pow(log_scaled, 1.0 / gamma);

//                 tinycolormap::Color color = tinycolormap::GetColor(gamma_corrected, color_map_type);
//                 image[index] = static_cast<unsigned char>(color.r() * 255);     // R
//                 image[index + 1] = static_cast<unsigned char>(color.g() * 255); // G
//                 image[index + 2] = static_cast<unsigned char>(color.b() * 255); // B
//             }
//         }
//     }

//     stbi_write_png(filename.c_str(), width, height, 3, image.data(), width * 3);
// }

void ImageHistogram::save_from_histogram(const std::string& filename, double gamma, std::string color_map) {
    // Create the output directory if it doesn't exist
    std::filesystem::create_directories("output");

    // Prepend the output directory to the filename
    std::string output_filename = "output/" + filename;

    uint64_t max_count = parallel_max();
    LOG_INFO << "Max bin count: " << max_count;
    std::vector<unsigned char> image(width * height * 3, 0);

    // Calculate log(max_count + 1) once
    double log_max = std::log(max_count + 1);

    tinycolormap::ColormapType color_map_type;
    if (color_map == "viridis") {
        color_map_type = tinycolormap::ColormapType::Viridis;
    } else if (color_map == "plasma") {
        color_map_type = tinycolormap::ColormapType::Plasma;
    } else {
        LOG_ERROR << "Unknown color map type! Defaulting to grayscale.";
        color_map_type = tinycolormap::ColormapType::Gray;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint64_t count = histogram[y * width + x].load(std::memory_order_relaxed);
            
            int index = (y * width + x) * 3;
            if (count == 0) {
                // Set color to black for zero count
                image[index] = 0;     // R
                image[index + 1] = 0; // G
                image[index + 2] = 0; // B
            } else {
                // Apply logarithmic scaling
                double log_scaled = std::log(count + 1) / log_max;
                
                // Apply gamma correction
                double gamma_corrected = std::pow(log_scaled, 1.0 / gamma);


                tinycolormap::Color color = tinycolormap::GetColor(gamma_corrected, color_map_type);
                image[index] = static_cast<unsigned char>(color.r() * 255);     // R
                image[index + 1] = static_cast<unsigned char>(color.g() * 255); // G
                image[index + 2] = static_cast<unsigned char>(color.b() * 255); // B
            }
        }
    }

    stbi_write_png(output_filename.c_str(), width, height, 3, image.data(), width * 3);
}

ImageHistogram::~ImageHistogram() {
    delete[] histogram;
}