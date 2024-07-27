#include "complex_plane.h"
#include <cstring>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

ComplexPlane::ComplexPlane(int w, int h, double rmin, double rmax, double imin, double imax)
    : width(w), height(h), real_min(rmin), real_max(rmax), imag_min(imin), imag_max(imax) {
    bins = new std::atomic<uint64_t>[width * height];
    for (int i = 0; i < width * height; ++i) {
        bins[i].store(0, std::memory_order_relaxed);
    }
}

void ComplexPlane::add_point(const std::complex<double>& point) {
    int x = static_cast<int>((point.real() - real_min) / (real_max - real_min) * width);
    int y = static_cast<int>((point.imag() - imag_min) / (imag_max - imag_min) * height);
    
    if (x >= 0 && x < width && y >= 0 && y < height) {
        bins[y * width + x].fetch_add(1, std::memory_order_relaxed);
    }
}

// Method to save the image using stb_image_write
void ComplexPlane::save_image(const std::string& filename) {
    std::vector<unsigned char> image(width * height, 0);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (bins[y * width + x].load(std::memory_order_relaxed) > 0) {
                image[y * width + x] = 255;  // White
            }
        }
    }

    stbi_write_png(filename.c_str(), width, height, 1, image.data(), width);
}

ComplexPlane::~ComplexPlane() {
    delete[] bins;
}