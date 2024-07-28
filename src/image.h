#pragma once

#include <vector> 
#include <complex>
#include <string>
#include <cstdint>
#include <atomic>

#include "eigenvalue.h"

class ImageHistogram {
public:
    ImageHistogram(int resolution, double rmin, double rmax, double imin, double imax);
    ~ImageHistogram();
    void add_point(const std::complex<double>& point);
    uint64_t parallel_max() const;
    // void save_image(const std::string& filename, double gamma, std::string color_map, const EigenvaluePMF& pmf);
    void save_from_histogram(const std::string& filename, double gamma, std::string color_map);

private:
    int width, height;
    double real_min, real_max, imag_min, imag_max;
    std::atomic<uint64_t> *histogram;
};