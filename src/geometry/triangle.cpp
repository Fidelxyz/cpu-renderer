#include "geometry/triangle.hpp"

#include <omp.h>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <cmath>

#include "global.hpp"
#include "texture/mipmap.hpp"

float Triangle::cross2d(const vec2 &v1, const vec2 &v2) {
    return v1.x() * v2.y() - v2.x() * v1.y();
}

float Triangle::truncate_x_ss(float x, const Camera &camera) {
    x = std::min(x, static_cast<float>(camera.width));
    x = std::max(x, 0.f);
    return x;
}

float Triangle::truncate_y_ss(float y, const Camera &camera) {
    y = std::min(y, static_cast<float>(camera.height));
    y = std::max(y, 0.f);
    return y;
}

bool Triangle::is_inside_ss(const vec3 &w) const {
    return -EPS < w.x() && -EPS < w.y() && -EPS < w.z();
}

vec3 Triangle::normal() const {
    return (vertices[0]->pos - vertices[1]->pos)
        .cross(vertices[0]->pos - vertices[2]->pos)
        .normalized();
}

vec3 Triangle::barycoord_ss(const vec2 &screen_pos) const {
    auto v1 = vec2(vertices[0]->screen_pos.x(), vertices[0]->screen_pos.y());
    auto v2 = vec2(vertices[1]->screen_pos.x(), vertices[1]->screen_pos.y());
    auto v3 = vec2(vertices[2]->screen_pos.x(), vertices[2]->screen_pos.y());

    float dx1 = v2.x() - v3.x();
    float dy1 = v2.y() - v3.y();
    float det1 = v2.x() * v3.y() - v3.x() * v2.y();

    float w1 = (dy1 * screen_pos.x() - dx1 * screen_pos.y() + det1) /
               (dy1 * v1.x() - dx1 * v1.y() + det1);

    float dx2 = v3.x() - v1.x();
    float dy2 = v3.y() - v1.y();
    float det2 = v3.x() * v1.y() - v1.x() * v3.y();

    float w2 = (dy2 * screen_pos.x() - dx2 * screen_pos.y() + det2) /
               (dy2 * v2.x() - dx2 * v2.y() + det2);

    float w3 = 1.f - w1 - w2;

    return vec3(w1, w2, w3);
}

float Triangle::interpolate_z_ss(const vec3 &barycoord_ss) const {
    float w1 = barycoord_ss.x();
    float w2 = barycoord_ss.y();
    float w3 = barycoord_ss.z();
    float z1 = vertices[0]->screen_pos.z();
    float z2 = vertices[1]->screen_pos.z();
    float z3 = vertices[2]->screen_pos.z();
    return w1 * z1 + w2 * z2 + w3 * z3;
}

std::tuple<float, float, float> Triangle::corrected_barycoord(
    const vec3 &barycoord_ss) const {
    float w1 = vertices[0]->w;
    float w2 = vertices[1]->w;
    float w3 = vertices[2]->w;
    float c1 = barycoord_ss.x();
    float c2 = barycoord_ss.y();
    float c3 = barycoord_ss.z();
    float w = c1 / w1 + c2 / w2 + c3 / w3;
    return std::make_tuple(c1 / w1 / w, c2 / w2 / w, c3 / w3 / w);
}

template <typename T>
T Triangle::interpolate(const std::tuple<T, T, T> &vals,
                        const std::tuple<float, float, float> &weights) {
    auto [w1, w2, w3] = weights;
    auto [v1, v2, v3] = vals;
    return w1 * v1 + w2 * v2 + w3 * v3;
}

vec3 Triangle::shade(size_t pixel_x, size_t pixel_y,
                     const std::tuple<float, float, float> &w, const vec2 &uv,
                     const vec2 &duv, FragmentShader *fragment_shader) const {
    vec3 pos = interpolate(
        std::make_tuple(vertices[0]->pos, vertices[1]->pos, vertices[2]->pos),
        w);

    vec3 normal =
        normals.empty()
            ? this->normal()
            : interpolate(
                  std::make_tuple(*normals[0], *normals[1], *normals[2]), w);

    return fragment_shader->shade(pos, normal, uv, duv, material.get());
}

std::tuple<vec2, vec2> Triangle::calc_uv(
    const std::tuple<float, float, float> &w_shading,
    const vec3 &barycoord_shading, const vec3 &barycoord_lod_sample_x_delta,
    const vec3 &barycoord_lod_sample_y_delta) const {
    vec2 uv, duv;
    if (texcoords.empty()) {
        uv = vec2(0, 0);
        duv = vec2(1, 1);
    } else {
        auto texcoord_tuple =
            std::make_tuple(*texcoords[0], *texcoords[1], *texcoords[2]);

        // perspective-corrected interpolate
        auto w_x = corrected_barycoord(barycoord_shading +
                                       barycoord_lod_sample_x_delta);
        auto w_y = corrected_barycoord(barycoord_shading +
                                       barycoord_lod_sample_y_delta);

        uv = interpolate(texcoord_tuple, w_shading);
        vec2 ddx =
            (interpolate(texcoord_tuple, w_x) - uv) / mipmap::LOD_SAMPLE_DELTA;
        vec2 ddy =
            (interpolate(texcoord_tuple, w_y) - uv) / mipmap::LOD_SAMPLE_DELTA;
        float du = (std::fabs(ddx.x()) + std::fabs(ddy.x())) / 2.f;
        float dv = (std::fabs(ddx.y()) + std::fabs(ddy.y())) / 2.f;
        duv = vec2(du, dv);
    }
    return std::make_tuple(uv, duv);
}

void Triangle::rasterize(Texture<omp_lock_t> *mutex,
                         frame_buffer_t *frame_buffer, z_buffer_t *z_buffer,
                         FragmentShader *fragment_shader,
                         const Camera &camera) const {
    auto v1 = vertices[0];
    auto v2 = vertices[1];
    auto v3 = vertices[2];
    int min_x = truncate_x_ss(
        std::floor(std::min(
            {v1->screen_pos.x(), v2->screen_pos.x(), v3->screen_pos.x()})),
        camera);
    int min_y = truncate_y_ss(
        std::floor(std::min(
            {v1->screen_pos.y(), v2->screen_pos.y(), v3->screen_pos.y()})),
        camera);
    int max_x = truncate_x_ss(
        std::ceil(std::max(
            {v1->screen_pos.x(), v2->screen_pos.x(), v3->screen_pos.x()})),
        camera);
    int max_y = truncate_y_ss(
        std::ceil(std::max(
            {v1->screen_pos.y(), v2->screen_pos.y(), v3->screen_pos.y()})),
        camera);

    vec3 barycoord_init = barycoord_ss(vec2(min_x + 0.5f, min_y + 0.5f));
    vec3 barycoord_dx =
        barycoord_ss(vec2(min_x + 1.5f, min_y + 0.5f)) - barycoord_init;
    vec3 barycoord_dy =
        barycoord_ss(vec2(min_x + 0.5f, min_y + 1.5f)) - barycoord_init;

    vec3 barycoord_samples_delta[msaa::MSAA_LEVEL];
    for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
        barycoord_samples_delta[i] =
            barycoord_ss(vec2(min_x, min_y) + msaa::samples_coord_delta[i]) -
            barycoord_init;
    }

    vec3 barycoord_lod_sample_x_delta =
        barycoord_ss(
            vec2(min_x + 0.5f + mipmap::LOD_SAMPLE_DELTA, min_y + 0.5f)) -
        barycoord_init;
    vec3 barycoord_lod_sample_y_delta =
        barycoord_ss(
            vec2(min_x + 0.5f, min_y + 0.5f + mipmap::LOD_SAMPLE_DELTA)) -
        barycoord_init;

    vec3 barycoord_y = barycoord_init;
    for (int pixel_y = min_y; pixel_y < max_y; pixel_y++) {
        vec3 barycoord_x = barycoord_y;
        for (int pixel_x = min_x; pixel_x < max_x; pixel_x++) {
            omp_set_lock(&mutex->at(pixel_x, pixel_y));
            vec3 barycoord_samples[msaa::MSAA_LEVEL];
            unsigned char covered_flag = 0;
            for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                barycoord_samples[i] = barycoord_x + barycoord_samples_delta[i];
                if (is_inside_ss(barycoord_samples[i])) {
                    float z = interpolate_z_ss(barycoord_samples[i]);
                    if (0.f < z && z < z_buffer->at(pixel_x, pixel_y)[i]) {
                        z_buffer->at(pixel_x, pixel_y)[i] = z;
                        covered_flag |= 1u << i;
                    }
                }
            }

            if (covered_flag) {
                bool full_covered =
                    (covered_flag == (1u << msaa::MSAA_LEVEL) - 1);

                vec3 barycoord_shading;

                if (full_covered) {  // All samples covered
                    barycoord_shading = barycoord_x;
                } else {  // Partical samples covered
                    barycoord_shading = vec3(0, 0, 0);
                    int covered_count = 0;
                    for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                        if ((covered_flag >> i) & 1) {
                            barycoord_shading += barycoord_samples[i];
                            covered_count++;
                        }
                    }
                    barycoord_shading /= covered_count;
                }

                // perspective-corrected interpolate
                auto w_shading = corrected_barycoord(barycoord_shading);
                auto [uv, duv] = calc_uv(w_shading, barycoord_shading,
                                         barycoord_lod_sample_x_delta,
                                         barycoord_lod_sample_y_delta);
                vec3 shading = shade(pixel_x, pixel_y, w_shading, uv, duv,
                                     fragment_shader);

                if (full_covered) {  // All samples covered
                    for (auto &frame_buffer_val :
                         frame_buffer->at(pixel_x, pixel_y)) {
                        frame_buffer_val = shading;
                    }
                } else {  // Partical samples covered
                    for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                        if (covered_flag & (1u << i)) {
                            frame_buffer->at(pixel_x, pixel_y)[i] = shading;
                        }
                    }
                }
            }
            omp_unset_lock(&mutex->at(pixel_x, pixel_y));
            barycoord_x += barycoord_dx;
        }
        barycoord_y += barycoord_dy;
    }
}