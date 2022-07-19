#pragma once
#ifndef SSAO_H
#define SSAO_H

#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <random>

#include "effects/msaa.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"
#include "shader/vertex_shader.hpp"
#include "texture/buffer.hpp"
#include "texture/texture.hpp"
#include "utils/transform.hpp"

namespace ssao {

const size_t SAMPLE_BASES_NUM = 4;
const size_t SAMPLES_DIR_NUM = SAMPLE_BASES_NUM * SAMPLE_BASES_NUM;
const size_t SAMPLES_DIST_NUM = SAMPLE_BASES_NUM;
const size_t SAMPLES_NUM = SAMPLES_DIR_NUM * SAMPLES_DIST_NUM;
const float SAMPLE_RADIUS = 0.05;

Texture<vec3> ssao_filter(Buffer *buffer, const Texture<vec3> &frame_buffer,
                          const Texture<float> &z_buffer,
                          VertexShader *vertex_shader) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> random_f(-1.f, 1.f);

    auto ao_texture =
        cv::Mat(frame_buffer.height, frame_buffer.width, CV_32FC1);

    float width = frame_buffer.width;
    float height = frame_buffer.height;

    // generate uniform dir
    float sample_dir_base_interval =
        180.f / static_cast<float>(SAMPLE_BASES_NUM);
    std::vector<float> sample_dir_bases;
    sample_dir_bases.reserve(SAMPLE_BASES_NUM);
    for (size_t i = 0; i < SAMPLE_BASES_NUM; i++) {
        sample_dir_bases.emplace_back(
            sample_dir_base_interval * (static_cast<float>(i) + 0.5f) - 90.f);
    }
    std::vector<vec3> samples_dir;
    samples_dir.reserve(SAMPLES_DIR_NUM);
    for (size_t i = 0; i < SAMPLE_BASES_NUM; i++) {
        for (size_t j = 0; j < SAMPLE_BASES_NUM; j++) {
            DirectionTransform transform;
            transform.rotation(
                vec3(sample_dir_bases[i], sample_dir_bases[j], 0));
            samples_dir.emplace_back(transform.transform(vec3(0, 0, 1)));
        }
    }

    // generate uniform dist
    float sample_dist_base_interval =
        SAMPLE_RADIUS / static_cast<float>(SAMPLE_BASES_NUM);
    std::vector<float> samples_dist;
    samples_dist.reserve(SAMPLES_DIST_NUM);
    for (size_t i = 0; i < SAMPLE_BASES_NUM; i++) {
        samples_dist.emplace_back(sample_dist_base_interval *
                                  (static_cast<float>(i) + 0.5f));
    }

    // for each pixel
#pragma omp parallel for
    for (size_t y = 0; y < frame_buffer.height; y++) {
        for (size_t x = 0; x < frame_buffer.width; x++) {
            float occlusion = 0.f;

            vec3 tangent =
                vec3(random_f(rng), random_f(rng), random_f(rng)).normalized();

            // for each MSAA sample
            for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                if (buffer->z_buffer->at(x, y)[i] >=
                    1.f - EPS)  // at the backgound
                    continue;

                mat3 tbn;
                vec3 n = buffer->normal_buffer->at(x, y)[i];
                vec3 t = (tangent - tangent.dot(n) * n).normalized();
                vec3 b = t.cross(n);

                // clang-format off
                tbn << t.x(), t.y(), t.z(),
                       b.x(), b.y(), b.z(),
                       n.x(), n.y(), n.z();
                // clang-format on

                vec3 pos = buffer->pos_buffer->at(x, y)[i];

                // each sample
                for (size_t sample_i = 0; sample_i < SAMPLES_DIR_NUM;
                     sample_i++) {
                    for (size_t sample_j = 0; sample_j < SAMPLES_DIST_NUM;
                         sample_j++) {
                        // generate a sampling direction in a hemisphere

                        // transform to tangent space
                        vec3 sample_dir =
                            tbn.transpose() * samples_dir[sample_i];

                        // generate the distance from the center to the sampling
                        // point

                        vec3 sample_pos =
                            pos + sample_dir * samples_dist[sample_j];

                        Vertex sample_vertex = Vertex(sample_pos);
                        vertex_shader->shade(&sample_vertex);
                        vec2 sample_uv =
                            vec2(sample_vertex.screen_pos.x() / width,
                                 1.f - sample_vertex.screen_pos.y() / height);

                        float sample_z = sample_vertex.screen_pos.z();
                        float orig_z = z_buffer.sample_no_repeat(sample_uv);

                        if (sample_z > orig_z) {  // if occluted
                            occlusion +=
                                std::max(0.f, sample_dir.dot(n)) *
                                (1.f - samples_dist[sample_j] / SAMPLE_RADIUS);
                        }
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

            occlusion = std::min(occlusion * 6.f, 1.f);
            occlusion = std::pow(occlusion, 1.5f);

            ao_texture.at<float>(y, x) = 1.f - occlusion;
        }
    }

    cv::Mat ao_texture_blurred;
    cv::bilateralFilter(ao_texture, ao_texture_blurred, 5, 0.2, 15);
    // std::swap(ao_texture, ao_texture_blurred);
    // cv::bilateralFilter(ao_texture, ao_texture_blurred, 9, 0.1, 200);
    // std::swap(ao_texture, ao_texture_blurred);
    // cv::bilateralFilter(ao_texture, ao_texture_blurred, 5, 0.3, 50);

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