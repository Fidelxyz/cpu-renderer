#pragma once
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <Eigen/Core>
#include <Eigen/Dense>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

#include "effects/msaa.hpp"
#include "geometry/vertex.hpp"
#include "global.hpp"
#include "scene/camera.hpp"
#include "scene/material.hpp"
#include "shader/fragment_shader.hpp"
#include "texture/buffer.hpp"
#include "utils/omp_locker.hpp"

class Triangle {
   public:
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<vec3>> normals;
    std::vector<std::shared_ptr<vec2>> texcoords;
    std::shared_ptr<Material> material = nullptr;

    enum CullMethod { NO_CULL, CULL_BACK, CULL_FRONT };

   private:
    vec3 tbn_u = vec3(0, 0, 0);

   public:
    vec3 normal() const;

    // Return if the point is inside the triangle in the screen space.
    bool is_inside_ss(const vec3 &w_ss) const;

    template <typename FragmentShaderT>
    void rasterize(Buffer *buffer, FragmentShaderT *fragment_shader,
                   const Camera &camera, CullMethod = NO_CULL);

   private:
    // Calculate the cross product of two 2-dimension vectors.
    static float cross2d(const vec2 &v1, const vec2 &v2);

    // Truncate x-coordinate of pixels into screen spaces.
    static float truncate_x_ss(float x, const Camera &camera);

    // Truncate y-coordinate of pixels into screen spaces.
    static float truncate_y_ss(float y, const Camera &camera);

    bool is_culled_normal(const Camera &camera, CullMethod cull_method) const;
    bool is_culled_normal(const vec3 &normal, const vec3 &pos,
                          const Camera &camera, CullMethod cull_method) const;
    bool is_culled_view(const Camera &camera) const;

    // Calculate the screen-space barycentric coordinate at the pixel position.
    vec3 barycoord_ss(const vec2 &screen_pos) const;

    // Interpolate screen-space z-coordinate (depth).
    float interpolate_z_ss(const vec3 &barycoord_ss) const;

    // Convert screen space barycentic coordinate to perspective-corrected (view
    // space) barycentic coordinate.
    std::tuple<float, float, float> corrected_barycoord(
        const vec3 &barycoord_ss) const;

    // Interpolate any value with a barycentic coordinate.
    template <typename T>
    static T interpolate(const std::tuple<T, T, T> &vals,
                         const std::tuple<float, float, float> &weights);

    template <typename FragmentShaderT>
    std::tuple<vec3, vec3, vec3> shade(size_t pixel_x, size_t pixel_y,
                                       const std::tuple<float, float, float> &w,
                                       const vec2 &uv, const vec2 &duv,
                                       FragmentShaderT *fragment_shader) const;

    std::tuple<vec2, vec2> calc_uv(
        const std::tuple<float, float, float> &w_shading,
        const vec3 &barycoord_shading,
        const std::tuple<vec3, vec3> &barycoord_lod_sample_delta) const;
};

template <typename T>
T Triangle::interpolate(const std::tuple<T, T, T> &vals,
                        const std::tuple<float, float, float> &weights) {
    auto [w1, w2, w3] = weights;
    auto [v1, v2, v3] = vals;
    return w1 * v1 + w2 * v2 + w3 * v3;
}

template <typename FragmentShaderT>
void Triangle::rasterize(Buffer *buffer, FragmentShaderT *fragment_shader,
                         const Camera &camera, CullMethod cull_method) {
    static_assert(std::is_base_of_v<FragmentShader, FragmentShaderT>);

    if (is_culled_normal(camera, cull_method)) return;
    if (is_culled_view(camera)) return;

    auto v1 = vertices[0];
    auto v2 = vertices[1];
    auto v3 = vertices[2];

    // screen coordinate range
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

    // barycentric coordinate
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

    // mipmap lod related barycentric coordinate
    vec3 barycoord_lod_sample_x_delta =
        barycoord_ss(
            vec2(min_x + 0.5f + mipmap::LOD_SAMPLE_DELTA, min_y + 0.5f)) -
        barycoord_init;
    vec3 barycoord_lod_sample_y_delta =
        barycoord_ss(
            vec2(min_x + 0.5f, min_y + 0.5f + mipmap::LOD_SAMPLE_DELTA)) -
        barycoord_init;
    auto barycoord_lod_sample_delta = std::make_tuple(
        barycoord_lod_sample_x_delta, barycoord_lod_sample_y_delta);

    // tangent space conversion
    if (!texcoords.empty() && material->normal_texture != nullptr) {
        vec3 e1 = vertices[0]->pos - vertices[1]->pos;
        vec3 e2 = vertices[0]->pos - vertices[2]->pos;

        vec2 delta_uv1 = *texcoords[0] - *texcoords[1];
        vec2 delta_uv2 = *texcoords[0] - *texcoords[2];

        float f =
            (delta_uv1.x() * delta_uv2.y() - delta_uv2.x() * delta_uv1.y());

        tbn_u = ((delta_uv2.y() * e1 - delta_uv1.y() * e2) / f).normalized();
    }

    vec3 barycoord_y = barycoord_init;
    for (int pixel_y = min_y; pixel_y < max_y; pixel_y++) {
        vec3 barycoord_x = barycoord_y;
        for (int pixel_x = min_x; pixel_x < max_x; pixel_x++) {
            OmpLocker omp_locker(&buffer->mutex->at(pixel_x, pixel_y));
            vec3 barycoord_samples[msaa::MSAA_LEVEL];
            unsigned char covered_flag = 0;
            // for each MSAA sample
            for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                // screen-space barycentric coordinate
                barycoord_samples[i] = barycoord_x + barycoord_samples_delta[i];

                // inside test
                if (!is_inside_ss(barycoord_samples[i])) continue;

                auto w_sample = corrected_barycoord(barycoord_samples[i]);

                // cull test
                if (!normals.empty()) {  // if normals.empty() == true, culling
                                         // is finished at the beginning.
                    vec3 normal = interpolate(
                        std::make_tuple(*normals[0], *normals[1], *normals[2]),
                        w_sample);
                    vec3 pos = interpolate(
                        std::make_tuple(v1->pos, v2->pos, v3->pos), w_sample);
                    if (is_culled_normal(normal, pos, camera, cull_method))
                        continue;
                }

                // alpha test
                if (material->alpha_texture != nullptr) {
                    vec2 uv = interpolate(
                        std::make_tuple(*texcoords[0], *texcoords[1],
                                        *texcoords[2]),
                        w_sample);
                    if (material->alpha_texture->sample(uv) < EPS) continue;
                }

                // depth test
                float z = interpolate_z_ss(barycoord_samples[i]);
                if (0.0 < z && z < buffer->z_buffer->at(pixel_x, pixel_y)[i]) {
                    // write into z-buffer
                    buffer->z_buffer->at(pixel_x, pixel_y)[i] = z;
                    covered_flag |= 1u << i;
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
                                         barycoord_lod_sample_delta);
                auto [shading, pos, normal] = shade(pixel_x, pixel_y, w_shading,
                                                    uv, duv, fragment_shader);

                for (size_t i = 0; i < msaa::MSAA_LEVEL; i++) {
                    if (full_covered || (covered_flag & (1u << i))) {
                        buffer->frame_buffer->at(pixel_x, pixel_y)[i] = shading;
                        buffer->pos_buffer->at(pixel_x, pixel_y)[i] = pos;
                        buffer->normal_buffer->at(pixel_x, pixel_y)[i] = normal;
                    }
                }
                buffer->full_covered->at(pixel_x, pixel_y) = full_covered;
            }
            barycoord_x += barycoord_dx;
        }
        barycoord_y += barycoord_dy;
    }
}

template <typename FragmentShaderT>
std::tuple<vec3, vec3, vec3> Triangle::shade(
    size_t pixel_x, size_t pixel_y, const std::tuple<float, float, float> &w,
    const vec2 &uv, const vec2 &duv, FragmentShaderT *fragment_shader) const {
    vec3 pos = interpolate(
        std::make_tuple(vertices[0]->pos, vertices[1]->pos, vertices[2]->pos),
        w);

    vec3 normal =
        normals.empty()
            ? this->normal()
            : interpolate(
                  std::make_tuple(*normals[0], *normals[1], *normals[2]), w);

    // apply normal texture
    if (material->normal_texture != nullptr && !std::isnan(tbn_u.x())) {
        // * Why check `!std::isnan(tbn_u.x())`:
        // * delta_uv may be 0
        // * -> tbn_u = inf
        // * -> tbu_n.normalized() = nan
        // * -> normal = nan

        vec3 uv_normal =
            (material->normal_texture->sample(uv, duv) - vec3(0.5, 0.5, 0.5))
                .normalized();  // [0, 1] -> [-1, 1]

        // TBN transform
        mat3 tbn;
        vec3 t = (tbn_u - tbn_u.dot(normal) * normal).normalized();
        vec3 b = t.cross(normal);
        // clang-format off
        tbn << t.x(), t.y(), t.z(),
               b.x(), b.y(), b.z(),
               normal.x(), normal.y(), normal.z();
        // clang-format on

        normal = tbn.transpose() * uv_normal;
    }

    vec3 shading = fragment_shader->shade(pos, normal, uv, duv, material.get());

    return std::make_tuple(shading, pos, normal);
}

#endif