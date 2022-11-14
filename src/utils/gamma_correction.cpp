#include "utils/gamma_correction.hpp"

#include <cmath>

const float GammaCorrection::gamma = 2.2;

cv::Mat GammaCorrection::lut_from_linear;
cv::Mat GammaCorrection::lut_to_linear;

void GammaCorrection::init_luts() {
    // LUT to linear (gamma)
    lut_to_linear = cv::Mat(1, 256, CV_8U);
    uchar *p = lut_to_linear.data;
    for (int i = 0; i < 256; i++) {
        p[i] = cv::saturate_cast<uchar>(
            std::pow(static_cast<float>(i) / 255.f, gamma) * 255.f);
    }

    // LUT from linear (1 / gamma)
    lut_from_linear = cv::Mat(1, 256, CV_8U);
    p = lut_from_linear.data;
    for (int i = 0; i < 256; i++) {
        p[i] = cv::saturate_cast<uchar>(
            std::pow(static_cast<float>(i) / 255.f, 1.f / gamma) * 255.f);
    }
}

void GammaCorrection::from_linear(cv::Mat *src) {
    if (lut_from_linear.empty()) {
        init_luts();
    }

    cv::LUT(*src, lut_from_linear, *src);
}

void GammaCorrection::to_linear(cv::Mat *src) {
    if (lut_to_linear.empty()) {
        init_luts();
    }

    cv::LUT(*src, lut_to_linear, *src);
}