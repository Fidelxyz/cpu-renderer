#pragma once
#ifndef RIMLIGHT_H
#define RIMLIGHT_H

#include <vector>

#include "effects/msaa.hpp"
#include "global.hpp"
#include "scene/camera.hpp"
#include "texture/texture.hpp"
#include "utils/functions.hpp"

namespace rimlight {
const size_t RIMLIGHT_WIDTH_LEFT = 5;
const size_t RIMLIGHT_WIDTH_RIGHT = 5;

// delta_x, delta_y, intensity
const std::vector<std::tuple<size_t, size_t, float>> RIMLIGHT_DELTA = {
    {-10, 0, 1.5}, {10, 0, 1.5}};

float linear_depth(const float z, const Camera &camera) {
    float n = camera.near_plane;
    float f = camera.far_plane;
    return (n * f + n) / (n + f + (n - f) * z);
    // z = (n * f + n) / (n + f + (n - f) * z')
}

void rimlight(Texture<std::array<vec3, msaa::MSAA_LEVEL>> *frame_buffer,
              const Texture<std::array<float, msaa::MSAA_LEVEL>> &z_buffer,
              const Camera &camera) {
#pragma omp parallel for
    for (size_t y = 0; y < z_buffer.height; y++) {
        for (size_t x = 0; x < z_buffer.width; x++) {
            for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                float depth = linear_depth(z_buffer.at(x, y)[i], camera);
                for (auto [dx, dy, intensity] : RIMLIGHT_DELTA) {
                    size_t tx = x + dx;
                    size_t ty = y + dy;

                    if ((tx < 0 || tx >= z_buffer.width) ||
                        (ty < 0 || ty >= z_buffer.height))
                        continue;

                    float t_depth =
                        linear_depth(z_buffer.at(tx, ty)[i], camera);
                    if (t_depth - depth > 1.f) {
                        frame_buffer->at(x, y)[i] *= intensity;
                    }
                }
            }
        }
    }
}
}  // namespace rimlight

#endif