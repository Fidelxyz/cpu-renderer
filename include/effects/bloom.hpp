#pragma once
#ifndef BLOOM_H
#define BLOOM_H

#include <vector>

#include "global.hpp"
#include "texture/texture.hpp"

namespace bloom {

float offset;
vec2 texel_size;
vec2 texel_size_half;

vec3 down_sample(const vec2 &uv, const Texture<vec3> &sampler_tex) {
    vec3 sum = sampler_tex.sample_no_repeat(uv) * 4;
    sum += sampler_tex.sample_no_repeat(uv + texel_size_half * offset);
    sum += sampler_tex.sample_no_repeat(
        uv + vec2(texel_size_half.x(), -texel_size_half.y()) * offset);
    sum += sampler_tex.sample_no_repeat(uv - texel_size_half * offset);
    sum += sampler_tex.sample_no_repeat(
        uv - vec2(texel_size_half.x(), -texel_size_half.y()) * offset);

    sum /= 8.f;
    return sum;
}

vec3 up_sample(const vec2 &uv, const Texture<vec3> &sampler_tex) {
    vec3 sum = vec3(0, 0, 0);

    sum += sampler_tex.sample_no_repeat(uv + vec2(texel_size.x(), 0) * offset);
    sum += sampler_tex.sample_no_repeat(
               uv + vec2(-texel_size_half.x(), texel_size_half.y()) * offset) *
           2;
    sum += sampler_tex.sample_no_repeat(uv + vec2(0, texel_size.y()) * offset);
    sum += sampler_tex.sample_no_repeat(uv + texel_size_half) * 2;
    sum += sampler_tex.sample_no_repeat(uv - vec2(texel_size.x(), 0) * offset);
    sum += sampler_tex.sample_no_repeat(
               uv - vec2(-texel_size_half.x(), texel_size_half.y()) * offset) *
           2;
    sum += sampler_tex.sample_no_repeat(uv - vec2(0, texel_size.y()) * offset);
    sum += sampler_tex.sample_no_repeat(uv - texel_size_half) * 2;

    sum /= 12.f;

    return sum;
}

Texture<vec3> bloom_filter(const Texture<vec3> &orig_frame,
                           const float strength, const float blur_radius,
                           const int blur_iteration) {
    assert(0.f <= strength && strength <= 1.f);

    // init buffer
    std::vector<Texture<vec3>> buffer;
    buffer.reserve(blur_iteration + 1u);
    buffer.emplace_back(orig_frame);

    for (int i = 1; i <= blur_iteration; i++) {
        buffer.emplace_back(buffer[i - 1].width / 2, buffer[i - 1].height / 2);
    }

    offset = 1.f + blur_radius;

    // generate UV cache
    std::vector<std::vector<float>> u;
    std::vector<std::vector<float>> v;
    u.reserve(blur_iteration);
    v.reserve(blur_iteration);
    for (int i = 0; i <= blur_iteration; i++) {
        vec2 texel_size = vec2(1.f / buffer[i].width, 1.f / buffer[i].height);
        vec2 texel_size_half = texel_size * 0.5f;
        u.emplace_back();
        v.emplace_back();
        u[i].reserve(buffer[i].width);
        v[i].reserve(buffer[i].height);
        for (size_t x = 0; x < buffer[i].width; x++) {
            u[i][x] = x * texel_size.x() + texel_size_half.x();
        }
        for (size_t y = 0; y < buffer[i].height; y++) {
            v[i][y] = 1.f - (y * texel_size.y() + texel_size_half.y());
        }
    }

    // down sample

    // iteration
    for (int i = 1; i <= blur_iteration; i++) {
        texel_size = vec2(1.f / buffer[i].width, 1.f / buffer[i].height);
        texel_size_half = texel_size * 0.5f;

        // each pixel
#pragma omp parallel for
        for (size_t x = 0; x < buffer[i].width; x++) {
            for (size_t y = 0; y < buffer[i].height; y++) {
                buffer[i].at(x, y) =
                    down_sample(vec2(u[i][x], v[i][y]), buffer[i - 1]);
            }
        }
    }

    // up sample

    // iteration
    for (int i = blur_iteration - 1; i >= 0; i--) {
        texel_size = vec2(1.f / buffer[i].width, 1.f / buffer[i].height);
        texel_size_half = texel_size * 0.5f;

        // each pixel
#pragma omp parallel for
        for (size_t x = 0; x < buffer[i].width; x++) {
            for (size_t y = 0; y < buffer[i].height; y++) {
                buffer[i].at(x, y) =
                    up_sample(vec2(u[i][x], v[i][y]), buffer[i + 1]);
            }
        }
    }

    // mix
#pragma omp parallel for
    for (size_t i = 0; i < orig_frame.width * orig_frame.height; i++) {
        // RGB
        for (size_t j = 0; j < 3; j++) {
            if (buffer[0][i][j] > orig_frame[i][j]) {
                buffer[0][i][j] = strength * buffer[0][i][j] +
                                  (1.f - strength) * orig_frame[i][j];
            } else {
                buffer[0][i][j] = orig_frame[i][j];
            }
        }
    }

    return buffer[0];
}

}  // namespace bloom

#endif