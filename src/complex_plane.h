#pragma once

#include <vector> 
#include <complex>
#include <string>
#include <cstdint>
#include <atomic>

class ComplexPlane {
public:
    ComplexPlane(int resolution, double rmin, double rmax, double imin, double imax);
    ~ComplexPlane();
    void add_point(const std::complex<double>& point);
    uint64_t parallel_max() const;
    void save_image(const std::string& filename, double gamma);

private:
    int width, height;
    double real_min, real_max, imag_min, imag_max;
    std::atomic<uint64_t> *histogram;
};