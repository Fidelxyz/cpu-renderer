#pragma once
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <memory>
#include <tuple>
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
    std::shared_ptr<Material> material;

    vec3 normal() const;
    void normal_transform(const mat3 normal_transform);

    // Return if the point is inside the triangle in the screen space.
    bool is_inside_ss(const vec3 &w_ss) const;

    void rasterize(Texture<omp_lock_t> *mutex, frame_buffer_t *frame_buffer,
                   z_buffer_t *z_buffer, FragmentShader *fragment_shader,
                   const Camera &camera) const;

   private:
    // Calculate the cross product of two 2-dimension vectors.
    static float cross2d(const vec2 &v1, const vec2 &v2);

    // Truncate x-coordinate of pixels into screen spaces.
    static float truncate_x_ss(float x, const Camera &camera);

    // Truncate y-coordinate of pixels into screen spaces.
    static float truncate_y_ss(float y, const Camera &camera);

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

    vec3 shade(size_t pixel_x, size_t pixel_y,
               const std::tuple<float, float, float> &w, const vec2 &uv,
               const vec2 &duv, FragmentShader *fragment_shader) const;

    std::tuple<vec2, vec2> calc_uv(
        const std::tuple<float, float, float> &w_shading,
        const vec3 &barycoord_shading, const vec3 &barycoord_lod_sample_x_delta,
        const vec3 &barycoord_lod_sample_y_delta) const;
};

#endif