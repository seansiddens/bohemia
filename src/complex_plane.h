#pragma once

#include <vector> 
#include <complex>
#include <string>
#include <cstdint>
#include <atomic>

class ComplexPlane {
public:
    ComplexPlane(int w, int h, double rmin, double rmax, double imin, double imax);
    ~ComplexPlane();
    void add_point(const std::complex<double>& point);
    void save_image(const std::string& filename);

private:
    int width, height;
    double real_min, real_max, imag_min, imag_max;
    std::atomic<uint64_t> *bins;
};