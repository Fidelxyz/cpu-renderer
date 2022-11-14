#pragma once
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "global.hpp"

__attribute_used__ static float smoothstep(const float t1, const float t2,
                                           const float x) {
    if (x <= t1) return 0.f;
    if (x >= t2) return 1.f;
    float k = (x - t1) / (t2 - t1);
    return k * k * (3.f - 2.f * k);
}

__attribute_used__ static float gamma_correction(const float val,
                                                 const float gamma) {
    return std::pow(val, gamma);
}

__attribute_used__ static vec3 gamma_correction(const vec3 &val,
                                                const float gamma) {
    return vec3(gamma_correction(val.x(), gamma),
                gamma_correction(val.y(), gamma),
                gamma_correction(val.z(), gamma));
}

__attribute_used__ static float luminance(const vec3 &color) {
    return 0.2126f * color.x() + 0.7152f * color.y() + 0.0722f * color.z();
}

__attribute_used__ static float fract(const float &x) {
    return x - std::floor(x);
}

#endif