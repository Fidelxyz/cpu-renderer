#pragma once
#ifndef MSAA_H
#define MSAA_H

#include <array>

#include "global.hpp"
#include "texture/texture.hpp"

extern "C++" {

namespace msaa {

const size_t MSAA_LEVEL = 4;

template <typename T>
Texture<T> msaa_filter(const Texture<bool> &full_covered,
                       const Texture<std::array<T, MSAA_LEVEL>> &src) {
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, vec3>);

    Texture<T> res(src.width, src.height);

#pragma omp parallel for
    for (size_t i = 0; i < src.width * src.height; i++) {
        // test if full covered
        if (full_covered[i]) {
            res[i] = src[i][0];
        } else {
            // init
            if constexpr (std::is_same_v<T, float>) {  // float
                res[i] = 0;
            } else {  // vec3
                res[i] = vec3(0, 0, 0);
            }

            // average
            for (size_t j = 0; j < MSAA_LEVEL; j++) {
                res[i] += src[i][j];
            }
            res[i] /= MSAA_LEVEL;
        }
    }

    return res;
}

template <typename T>
std::array<T, MSAA_LEVEL> texture_init_val(const T &val) {
    std::array<T, msaa::MSAA_LEVEL> ret;
    for (auto &v : ret) {
        v = val;
    }
    return ret;
}

// * samples_coord_delta
// * center: the left top corner of the pixel

// * MSAA 4x
const vec2 samples_coord_delta[MSAA_LEVEL] = {
    vec2(0.25, 0.25), vec2(0.75, 0.25), vec2(0.25, 0.75), vec2(0.75, 0.75)};

// * MSAA 4x (rotated sampling points)
// const vec2 samples_coord_delta[MSAA_LEVEL] = {
//     vec2(0.5915063509461, 0.1584936490539),
//     vec2(0.1584936490539, 0.4084936490539),
//     vec2(0.8415063509461, 0.5915063509461),
//     vec2(0.4084936490539, 0.8415063509461)};

// * MSAA OFF
// const vec2 samples_coord_delta[MSAA_LEVEL] = {vec2(0.5, 0.5)};

}  // namespace msaa
}

#endif