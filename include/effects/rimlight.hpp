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

// delta_x, delta_y, intensity
const std::vector<std::tuple<size_t, size_t, float>> RIMLIGHT_DELTA = {
    {-8, 0, 1.2}, {8, 0, 1.2}};

float calc_depth(const vec3 &pos, const Camera &camera) {
    return (pos - camera.pos).norm();
}

void rimlight(Buffer *buffer, const Camera &camera) {
#pragma omp parallel for
    for (size_t y = 0; y < buffer->pos_buffer->height; y++) {
        for (size_t x = 0; x < buffer->pos_buffer->width; x++) {
            for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                if (buffer->z_buffer->at(x, y)[i] > 1.f - EPS)
                    continue;  // at background

                float depth =
                    calc_depth(buffer->pos_buffer->at(x, y)[i], camera);

                // for each sample
                for (auto [dx, dy, intensity] : RIMLIGHT_DELTA) {
                    size_t tx = x + dx;
                    size_t ty = y + dy;

                    if ((tx < 0 || tx >= buffer->z_buffer->width) ||
                        (ty < 0 || ty >= buffer->z_buffer->height))
                        continue;

                    // do depth test if the sample is not at the background
                    if (buffer->z_buffer->at(tx, ty)[i] <= 1.f - EPS) {
                        float t_depth = calc_depth(
                            buffer->pos_buffer->at(tx, ty)[i], camera);
                        if (t_depth - depth < 1.f) continue;
                    }

                    buffer->frame_buffer->at(x, y)[i] *= intensity;
                }
            }
        }
    }
}
}  // namespace rimlight

#endif