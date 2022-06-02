#pragma once
#ifndef MSAA_H
#define MSAA_H

#include <array>

#include "global.hpp"
#include "texture/texture.hpp"

namespace msaa {

static const size_t MSAA_LEVEL = 4;

__attribute_used__ static Texture<vec3> msaa_filter(
    const Texture<std::array<vec3, MSAA_LEVEL>> &src) {
    Texture<vec3> res(src.width, src.height, vec3(0, 0, 0));
    for (size_t i = 0; i < src.width * src.height; i++) {
        for (size_t j = 0; j < MSAA_LEVEL; j++) {
            res[i] += src[i][j];
        }
        res[i] /= MSAA_LEVEL;
    }
    return res;
}

template <typename T>
static std::array<T, MSAA_LEVEL> texture_init_val(const T &val) {
    std::array<T, msaa::MSAA_LEVEL> ret;
    for (auto &v : ret) {
        v = val;
    }
    return ret;
}

// * samples_coord_delta
// * center: the left top corner of the pixel

// * MSAA 4x
// static const vec2 samples_coord_delta[MSAA_LEVEL] = {
//     vec2(0.25, 0.25), vec2(0.75, 0.25), vec2(0.25, 0.75), vec2(0.75, 0.75)};

// * MSAA 4x (rotated sampling points)
static const vec2 samples_coord_delta[MSAA_LEVEL] = {
    vec2(0.5915063509461, 0.1584936490539),
    vec2(0.1584936490539, 0.4084936490539),
    vec2(0.8415063509461, 0.5915063509461),
    vec2(0.4084936490539, 0.8415063509461)};

// * MSAA OFF
// static const vec2 samples_coord_delta[MSAA_LEVEL] = {vec2(0.5, 0.5)};

}  // namespace msaa

using frame_buffer_t = Texture<std::array<vec3, msaa::MSAA_LEVEL>>;
using z_buffer_t = Texture<std::array<float, msaa::MSAA_LEVEL>>;

#endif