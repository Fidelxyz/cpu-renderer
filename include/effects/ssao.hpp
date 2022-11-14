#pragma once
#ifndef SSAO_H
#define SSAO_H

#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "effects/msaa.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"
#include "shader/vertex_shader.hpp"
#include "texture/buffer.hpp"
#include "texture/texture.hpp"
#include "utils/functions.hpp"
#include "utils/transform.hpp"

namespace ssao {

const size_t SAMPLES_NUM = 32;
const float SAMPLE_RADIUS = 0.05;

// return noise between [0, 1]
float random(const float st1, const float st2) {
    return fract(sin(st1 * 114 + st2 * 514) * 19198.10);
}

float random(const float st1, const float st2, const float st3) {
    return fract(sin(st1 * 114.5 + st2 * 141.1 + st3 * 451.4) * 19198.10);
}

Texture<vec3> ssao_filter(Buffer *buffer, const Texture<vec3> &frame_buffer,
                          const Texture<float> &z_buffer,
                          VertexShader *vertex_shader) {
    auto ao_texture =
        cv::Mat(frame_buffer.height, frame_buffer.width, CV_32FC1);

    float width = frame_buffer.width;
    float height = frame_buffer.height;

    vec3 samples[SAMPLES_NUM];
    for (size_t i = 0; i < SAMPLES_NUM; i++) {
        float dist = i / SAMPLES_NUM * SAMPLE_RADIUS;
        dist = .1f + .9f * dist * dist;
        samples[i] = vec3(random(i, 0) * 2.f - 1.f,  // x: [-1, 1]
                          random(i, 1) * 2.f - 1.f,  // y: [-1, 1]
                          random(i, 2))              // z: [0, 1]
                         .normalized() *
                     dist;
    }

    // for each pixel
#pragma omp parallel for
    for (size_t y = 0; y < frame_buffer.height; y++) {
        for (size_t x = 0; x < frame_buffer.width; x++) {
            float occlusion = 0.f;

            // for each MSAA sample
            for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                // generate a random base tangent
                vec3 tangent =
                    vec3(random(x, y, i * 2) * 2.f - 1.f,      // x: [-1, 1]
                         random(x, y, i * 2 + 1) * 2.f - 1.f,  // y: [-1, 1]
                         0);                                   // z: 0

                mat3 tbn;
                vec3 n = buffer->normal_buffer->at(x, y)[i];
                vec3 t = (tangent - tangent.dot(n) * n).normalized();
                vec3 b = t.cross(n);

                // clang-format off
                tbn << t.x(), t.y(), t.z(),
                       b.x(), b.y(), b.z(),
                       n.x(), n.y(), n.z();
                // clang-format on
                tbn.transposeInPlace();

                if (buffer->z_buffer->at(x, y)[i] >=
                    1.f - EPS)  // at the backgound
                    continue;

                vec3 fragment_pos = buffer->pos_buffer->at(x, y)[i];
                Vertex fragment_vertex = Vertex(fragment_pos);
                vertex_shader->shade(&fragment_vertex);
                float fragment_z = fragment_vertex.screen_pos.z();

                // each sample
                for (size_t j = 0; j < SAMPLES_NUM; j++) {
                    // transform to tangent space
                    vec3 sample_pos = fragment_pos + tbn * samples[j];

                    Vertex sample_vertex = Vertex(sample_pos);
                    vertex_shader->shade(&sample_vertex);
                    vec2 sample_uv =
                        vec2(sample_vertex.screen_pos.x() / width,
                             1.f - sample_vertex.screen_pos.y() / height);

                    float sample_z = sample_vertex.screen_pos.z();
                    float buffer_z = z_buffer.sample_no_repeat(sample_uv);

                    if (sample_z > buffer_z + EPS) {  // if occluted
                        occlusion += smoothstep(
                            0.0, 1.0,
                            SAMPLE_RADIUS /
                                abs(fragment_z - buffer_z));  // range check
                    }
                }

                if (buffer->full_covered->at(x, y)) {
                    break;  // for full covered samples, only calculate AO
                            // once.
                }
            }

            if (!buffer->full_covered->at(x, y)) {
                occlusion /= msaa::MSAA_LEVEL;
            }

            occlusion /= SAMPLES_NUM;

            ao_texture.at<float>(y, x) = 1.f - occlusion;
        }
    }

    cv::UMat ao_texture_src = ao_texture.getUMat(cv::ACCESS_READ);
    cv::UMat ao_texture_dst;
    cv::bilateralFilter(ao_texture_src, ao_texture_dst, 10, 0.1, 10);
    cv::Mat ao_texture_blurred = ao_texture_dst.getMat(cv::ACCESS_READ);

    auto result = Texture<vec3>(frame_buffer.width, frame_buffer.height);
#pragma omp parallel for
    for (size_t y = 0; y < frame_buffer.height; y++) {
        for (size_t x = 0; x < frame_buffer.width; x++) {
            result.at(x, y) =
                frame_buffer.at(x, y) * ao_texture_blurred.at<float>(y, x);
            // result.at(x, y).x() = ao_texture_blurred.at<float>(y, x);
            // result.at(x, y).y() = ao_texture_blurred.at<float>(y, x);
            // result.at(x, y).z() = ao_texture_blurred.at<float>(y, x);
        }
    }

    return result;
}

}  // namespace ssao

#endif