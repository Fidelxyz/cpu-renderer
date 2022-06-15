#pragma once
#ifndef TRIANGLE_H
#define TRIANGLE_H

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

class Triangle {
   public:
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<vec3>> normals;
    std::vector<std::shared_ptr<vec2>> texcoords;
    std::shared_ptr<Material> material = nullptr;

    enum CullMethod { NO_CULL, CULL_BACK, CULL_FRONT };

    vec3 normal() const;

    // Return if the point is inside the triangle in the screen space.
    bool is_inside_ss(const vec3 &w_ss) const;

    template <typename FragmentShaderT>
    void rasterize(Texture<omp_lock_t> *mutex, frame_buffer_t *frame_buffer,
                   z_buffer_t *z_buffer, FragmentShaderT *fragment_shader,
                   const Camera &camera, CullMethod = NO_CULL) const;

   private:
    // Calculate the cross product of two 2-dimension vectors.
    static float cross2d(const vec2 &v1, const vec2 &v2);

    // Truncate x-coordinate of pixels into screen spaces.
    static float truncate_x_ss(float x, const Camera &camera);

    // Truncate y-coordinate of pixels into screen spaces.
    static float truncate_y_ss(float y, const Camera &camera);

    bool is_culled(const Camera &camera, CullMethod cull_method) const;

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
    vec3 shade(size_t pixel_x, size_t pixel_y,
               const std::tuple<float, float, float> &w, const vec2 &uv,
               const vec2 &duv, FragmentShaderT *fragment_shader) const;

    std::tuple<vec2, vec2> calc_uv(
        const std::tuple<float, float, float> &w_shading,
        const vec3 &barycoord_shading, const vec3 &barycoord_lod_sample_x_delta,
        const vec3 &barycoord_lod_sample_y_delta) const;
};

template <typename T>
T Triangle::interpolate(const std::tuple<T, T, T> &vals,
                        const std::tuple<float, float, float> &weights) {
    auto [w1, w2, w3] = weights;
    auto [v1, v2, v3] = vals;
    return w1 * v1 + w2 * v2 + w3 * v3;
}

template <typename FragmentShaderT>
void Triangle::rasterize(Texture<omp_lock_t> *mutex,
                         frame_buffer_t *frame_buffer, z_buffer_t *z_buffer,
                         FragmentShaderT *fragment_shader, const Camera &camera,
                         CullMethod cull_method) const {
    static_assert(std::is_base_of_v<FragmentShader, FragmentShaderT>);

    if (is_culled(camera, cull_method)) return;

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

template <typename FragmentShaderT>
vec3 Triangle::shade(size_t pixel_x, size_t pixel_y,
                     const std::tuple<float, float, float> &w, const vec2 &uv,
                     const vec2 &duv, FragmentShaderT *fragment_shader) const {
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

#endif